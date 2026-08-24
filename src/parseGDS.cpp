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

constexpr bool isRoutingTag(gdstk::Tag tag) noexcept {
    const uint32_t layer { gdstk::get_layer(tag) };
    const uint32_t type { gdstk::get_type(tag)};

    return std::ranges::any_of(ROUTING_LAYERS, [layer, type](const auto& routingType) {
        return routingType.Layer == layer && routingType.dataType == type;
    });
}


namespace {

constexpr bool isFiller(std::string_view Name) noexcept {
    if (!Name.starts_with("sky130_fd_sc_hd__")) return false;
    constexpr std::array<std::string_view, 4> filters{"tap", "decap", "diode", "fill"}; // diode was missin

    return std::ranges::any_of(filters, [Name](auto f) { return Name.find(f) != std::string_view::npos; });
}

// paste ts yo
constexpr bool isValidPinName(std::string_view Name) noexcept {
    constexpr std::array<std::string_view, 29> ValidPins {
        "A", "A0", "A1", "A1_N", "A2", "A2_N", "A3", "A4", "A_N",
        "B", "B1", "B1_N", "B2", "B_N",
        "C", "C1", "C_N", "CLK",
        "D", "D1", "D_N",
        "HI", "LO", "Q", "RESET_B", "S", "SET_B", "X", "Y"
    };

    return std::ranges::binary_search(ValidPins, Name);
}

constexpr bool is_flip_flop(std::string_view type) noexcept {
    return type.find("dfrtp") != std::string_view::npos ||
           type.find("dfstp") != std::string_view::npos ||
           type.find("dfxtp") != std::string_view::npos;
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
    std::map<std::string, std::vector<PinInfo> ,std::less<>> cellPinLibrary;
    std::span<gdstk::Cell *> cells(gdsFile.cell_array.items, gdsFile.cell_array.count);
    
    for (const auto *c : cells) {
        if (!isFiller(c->name)) continue;

        std::span<gdstk::Label *> labels(c->label_array.items, c->label_array.count);

        for (const auto* lbl : labels) {
            if (!lbl->text) continue;

            const std::string_view pname { lbl->text };
            if (!isFiller(pname)) continue;

            const uint32_t layer { gdstk::get_layer(lbl->tag)};
            if (layer == 67 || layer == 68) {
                cellPinLibrary[c->name].push_back({std::string(pname), layer, lbl->origin.x, lbl->origin.y});
            }
        }
    }

    std::println(stderr, "std cell library pin defs: ");
    std::println(stderr, "Loaded pin defs for {} cells", cellPinLibrary.size());

    for (const auto& [cellname, pins] : cellPinLibrary) {
        std::cerr << " " << std::left << std::setw(32) << cellname << " -> [ ";
        for (auto p{0}; p < pins.size(); ++p) {
            std::cerr << (p > 0 ? ", " : "") << pins[p].Name << "(layer" << pins[p].Layer << ")";
        }
        std::cerr << "]\n";
    }

    // extract functional pin placements from top cell
    // extract top level io pad labels
    //
    // serializw json

}

