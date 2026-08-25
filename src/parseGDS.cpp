#include <print>
#include <gdstk/gdstk.hpp>
#include <string>
#include <string_view>
#include <ranges>
#include <algorithm>
#include "json.hpp"
#include <map>
#include <span>
#include <iostream>
#include <vector>
#include <fstream>

using Json = nlohmann::json;

struct LayerDef
{
    uint32_t Layer;
    uint32_t dataType;
    std::string_view Name;
};

constexpr std::array<LayerDef, 11> ROUTING_LAYERS{
    LayerDef{67, 20, "li1"},  {67, 44, "mcon"}, {68, 20, "met1"},
    {68, 44, "via"},  {69, 20, "met2"}, {69, 44, "via2"},
    {70, 20, "met3"}, {70, 44, "via3"}, {71, 20, "met4"},
    {71, 44, "via4"}, {72, 20, "met5"},
};

struct CellRecord {
    int id;
    std::string cell_type;
    double x;
    double y;
    double rotation;
    bool x_reflection;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CellRecord, id,  cell_type, x, y, rotation, x_reflection);

struct polygonRecord {
    int id;
    uint32_t Layer;
    uint32_t dataType;
    
    std::array<double, 4> bBox; 

    auto tiedbBox() const noexcept {
        return std::tie(Layer, dataType, bBox[0], bBox[1],bBox[2], bBox[3]); // tie into tuple
    }
};

constexpr std::string_view layerNameFor(uint32_t layer, uint32_t dataType) noexcept {
    for (auto const&[ lr, dt, name ] : ROUTING_LAYERS) {
        if (lr == layer && dt == dataType) return name;
    }
    return "idk bro";
}

// json helpa
//
void to_json(Json& son, const polygonRecord& polygon) {
    son = Json{
        {"id", polygon.id},
        {"layer", polygon.Layer},
        {"datatype", polygon.dataType},
        {"layer_name", layerNameFor(polygon.Layer, polygon.dataType)},
        {"bbox", polygon.bBox}
    };
}

constexpr bool isRoutingTag(gdstk::Tag tag) noexcept {
    const uint32_t layer { gdstk::get_layer(tag) };
    const uint32_t type { gdstk::get_type(tag)};

    return std::ranges::any_of(ROUTING_LAYERS, [layer, type](const auto& routingType) {
        return routingType.Layer == layer && routingType.dataType == type;
    });
}


constexpr bool isFiller(std::string_view Name) noexcept {
    if (!Name.starts_with("sky130_fd_sc_hd__")) return true;
    constexpr std::array<std::string_view, 4> filters{"tap", "decap", "diode", "fill"}; // diode was missin

    return std::ranges::any_of(filters, [Name](auto f) { return Name.find(f) != std::string_view::npos; });
}


// rectilin. polygon to rectangle: create using LLM
namespace {

// non rectangular shapes (Lshaped, Tshaped, dumbell shaped routing wires) had cauased false spatial overlap problems in previous iterations of this project. 
// The only *fix* could've been to treat the polygon as a simple discrete non-overlapping rectangle (decomposing them)
// function written using LLM
void decompRectilinear(const gdstk::Polygon* poly, uint32_t layer, uint32_t datatype,
                           std::vector<polygonRecord>& out, int& poly_id) {
    const uint64_t nv = poly->point_array.count;

    if (nv <= 4) {
        gdstk::Vec2 bmin{}, bmax{};
        poly->bounding_box(bmin, bmax);
        out.push_back({poly_id++, layer, datatype, {bmin.x, bmin.y, bmax.x, bmax.y}});
        return;
    }

    std::span<const gdstk::Vec2> pts(poly->point_array.items, nv);

    std::vector<double> ys;
    ys.reserve(nv);
    for (const auto& pt : pts) ys.push_back(pt.y);
    std::ranges::sort(ys);
    ys.erase(std::ranges::unique(ys).begin(), ys.end());

    std::vector<double> x_crossings;
    for (size_t i = 0; i + 1 < ys.size(); ++i) {
        const double y_lo = ys[i];
        const double y_hi = ys[i + 1];
        const double y_mid = (y_lo + y_hi) * 0.5;

        x_crossings.clear();
        for (uint64_t e = 0; e < nv; ++e) {
            const uint64_t f = (e + 1) % nv;
            const double ey = pts[e].y, fy = pts[f].y;
            if (ey == fy) continue;

            if ((ey <= y_mid && fy > y_mid) || (fy <= y_mid && ey > y_mid)) {
                const double t = (y_mid - ey) / (fy - ey);
                x_crossings.push_back(pts[e].x + t * (pts[f].x - pts[e].x));
            }
        }
        std::ranges::sort(x_crossings);

        for (size_t j = 0; j + 1 < x_crossings.size(); j += 2) {
            const double x_lo = x_crossings[j];
            const double x_hi = x_crossings[j + 1];
            if (x_hi - x_lo < 1e-6) continue;

            out.push_back({poly_id++, layer, datatype, {x_lo, y_lo, x_hi, y_hi}});
        }
    }
}

} // namespace

namespace {

class ScopedPolygonArray {
private:
    constexpr void reset() noexcept {
        for (uint64_t i{0}; i < SP_Array.count; ++i) {
            if (SP_Array[i]) {
                SP_Array[i]->clear();
                gdstk::free_allocation(SP_Array[i]);
            }
        }
        SP_Array.clear();
    }

public:
    gdstk::Array<gdstk::Polygon *> SP_Array{};

    ScopedPolygonArray() = default;
    ~ScopedPolygonArray() {
        reset();
    }

    ScopedPolygonArray(const ScopedPolygonArray&) = delete;
    ScopedPolygonArray operator=(const ScopedPolygonArray&) = delete;

    ScopedPolygonArray(ScopedPolygonArray&& other) noexcept
    : SP_Array(other.SP_Array) {
        other.SP_Array = {};
    }

    [[nodiscard]] std::span<gdstk::Polygon *> span() const noexcept {
        return { SP_Array.items, SP_Array.count };
    }
};

}

int main(int argc, char** argv) {
    std::println("Standard cell and pin definition: ---------------------------------");

    const std::string_view gdsPath { (argc > 1) ? argv[1] : "puzzle.gds"};
    const char *outJSON { (argc > 2) ? argv[2] : nullptr };

    gdstk::ErrorCode err { gdstk::ErrorCode::NoError };
    gdstk::Library gdsFile { gdstk::read_gds(gdsPath.data(), 0, 0, nullptr, &err)};

    struct LibGuard {
        gdstk::Library& lib;
        ~LibGuard() { lib.free_all(); }
    } guard{ gdsFile };

    if (err != gdstk::ErrorCode::NoError) {
        std::print("error reading gds");
        return EXIT_FAILURE;
    }

    std::println("Loaded library: {} count: {} unit: {} precision: {}", gdsFile.name ? gdsFile.name : "(unnamed)", gdsFile.cell_array.count, gdsFile.unit, gdsFile.precision);

    // find topcell
    gdstk::Cell *topCell { gdsFile.get_cell("puzzle") }; // test
    if (!topCell) { 
        topCell = gdsFile.get_cell("adder_demo");  // test2
    }

    if (!topCell) {
        gdstk::Array<gdstk::Cell *> topCells{};
        gdstk::Array<gdstk::RawCell *> topRawCells{};

        gdsFile.top_level(topCells, topRawCells);

        if (topCells.count > 0) {
            topCell = topCells[0];
            if (topCells.count > 1) 
                std::println(stderr, "{} top level cells found using {}", topCells.count, topCell->name);
        }
        topCells.clear();
        topRawCells.clear();
    }

    if (!topCell) {
        std::println(stderr, "Couldn't find top level cell bruh");
        return EXIT_FAILURE;
    }
    std::println(stderr, "Top level cell: {} ", topCell->name);

    // sref : structure references. extract cellplacement
    std::vector<CellRecord> cells;
    int cellID{0} ;
    constexpr double RAD_TO_DEGREE { 180.0 / M_PI };

    std::span<gdstk::Reference *> references(topCell->reference_array.items, topCell->reference_array.count);

    for (const auto* ref : references) {
        const char* rawName { (ref->type == gdstk::ReferenceType::Cell && ref->cell)
                            ? ref->cell->name
                            : (ref->type == gdstk::ReferenceType::Name ? ref->name : nullptr)
        };

        if (!rawName) continue;

        const std::string_view refName { rawName };
        if (isFiller(refName)) continue;

        cells.push_back({
            cellID++,
            std::string(refName),
            ref->origin.x,
            ref->origin.y,
            ref->rotation * RAD_TO_DEGREE,
            ref->x_reflection
        });
    }

    std::println(stderr, "Extracted {} cell placements from {}", cells.size(), topCell->reference_array.count);


    // extract metal routing and vias-- filter using Sky130 BEOL (google it brah) routing stack i.e. li1, mcon, met1-met5's vias (via1- via4)
    std::vector<polygonRecord> polygons;
    int polygonID{0};
        // topcell polygon
    for(const auto& rlayer : ROUTING_LAYERS) {
        const gdstk::Tag tag { gdstk::make_tag(rlayer.Layer ,rlayer.dataType) };
        
        ScopedPolygonArray tmp;
        topCell->get_polygons(true, true, 0, true, tag, tmp.SP_Array);

        for (const auto *p : tmp.span()) {
            if (isRoutingTag(p->tag)) {
                decompRectilinear(p, gdstk::get_layer(p->tag), gdstk::get_type(p->tag), polygons, polygonID);
            }
        }
    }
    
    // polygons from VIA* instances
    for (const auto *ref : references) {
        const char* raw_name = (ref->type == gdstk::ReferenceType::Cell && ref->cell) 
                            ? ref->cell->name 
                            : (ref->type == gdstk::ReferenceType::Name ? ref->name : nullptr);
        if (!raw_name) continue;
        
        const std::string_view name{raw_name};

        if (!name.starts_with("VIA_via") && name.starts_with("VIA_")) {

            ScopedPolygonArray tmp;
            ref->get_polygons(true, true, -1, false, 0, tmp.SP_Array);

            for (const auto *p : tmp.span()) {
                if (isRoutingTag(p->tag)) {
                    decompRectilinear(p ,gdstk::get_layer(p->tag), gdstk::get_type(p->tag), polygons, polygonID);
                }
            }
        }
    }


    // remove duplicates
    std::ranges::sort(polygons, [](const auto& a, const auto& b) {
        return a.tiedbBox() < b.tiedbBox();
    });

    const auto[dropStart, dropEnd] = std::ranges::unique(polygons, [](const auto& a, const auto& b) {
        return a.tiedbBox() == b.tiedbBox();
    });
    polygons.erase(dropStart, dropEnd);

    for (auto i{0uz}; i < polygons.size(); ++i) {
        polygons[i].id = static_cast<int>(i);
    }
    std::println("Extracted {} routing/contact polygons", polygons.size());

    // serializw json
    Json out_json_obj = Json::object();
    out_json_obj["cells"] = cells;
    out_json_obj["polygons"] = polygons;

    if (outJSON) {
        std::ofstream fileOut(outJSON);
        if (!fileOut) {
            std::println(stderr, "ERROR: Cannot open {}", outJSON);
            return EXIT_FAILURE;
        }

        fileOut << out_json_obj.dump(2) << '\n';
        std::println(stderr, "Output written to {}", outJSON);
    }
    else {
        println("{}", out_json_obj.dump(2));
    }

}

