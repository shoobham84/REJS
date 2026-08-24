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

void decomposeRectilinear() {

}

}


int main(int argc, char** argv) {
    std::println("Standard cell and pin definition: ---------------------------------");

    const char *gdsPath { "../puzzle.gds" };
    const char *outJSON { "outputs/parsedCells.json"};

    gdstk::ErrorCode err { gdstk::ErrorCode::NoError };
    gdstk::Library gdsFile { gdstk::read_gds(gdsPath, 0, 0, nullptr, &err)};

    struct LibGuard {
        gdstk::Library& l;
        ~LibGuard() { l.free_all(); }
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
    
    // find topcell
    gdstk::Cell *top { gdsFile.get_cell("puzzle") };
    if (!top) { std::println(stderr, "top cell of gds thing not found"); 
        return EXIT_FAILURE; }


    std::println("Loaded library: {} count: {} unit: {} precision: {}", gdsFile.name, gdsFile.cell_array.count, gdsFile.unit, gdsFile.precision);


    //extract pin definitions

    std::println(stderr, "std cell library pin defs: ");

    // extract functional pin placements from top cell
    // extract top level io pad labels
    //
    // serializw json

}

