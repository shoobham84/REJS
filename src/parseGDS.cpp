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
    std::string cellType;
    double x;
    double y;
    double rotation;
    bool xReflected;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(CellRecord, id,  cellType, x, y, rotation, xReflected);

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
void toJSON(Json& son, const polygonRecord& polygon) {
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
    if (!Name.starts_with("sky130_fd_sc_hd__")) return false;
    constexpr std::array<std::string_view, 4> filters{"tap", "decap", "diode", "fill"}; // diode was missin

    return std::ranges::any_of(filters, [Name](auto f) { return Name.find(f) != std::string_view::npos; });
}


// rectilin. polygon to rectangle: create using LLM
namespace {

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

    if (err != gdstk::ErrorCode::NoError &&
        err != gdstk::ErrorCode::MissingReference &&
        err != gdstk::ErrorCode::UnsupportedRecord &&
        err != gdstk::ErrorCode::UnofficialSpecification) {
        std::println("failed to read gds file");
        return 1;
    }
    
    std::println("Loaded library: {} count: {} unit: {} precision: {}", gdsFile.name, gdsFile.cell_array.count, gdsFile.unit, gdsFile.precision);

    // find topcell
    gdstk::Cell *topCell { gdsFile.get_cell("puzzle") }; // test
    if (!topCell) { 
        gdsFile.get_cell("adder_demo");  // test2
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



    //extract pin definitions

    std::println(stderr, "std cell library pin defs: ");

    // extract functional pin placements from top cell
    // extract top level io pad labels
    //
    // serializw json

}

