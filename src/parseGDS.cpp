#include <print>
#include <gdstk/gdstk.hpp>
#include <string>
#include <string_view>
#include <ranges>
#include <algorithm>
#include "json.hpp"

using Json = nlohmann::json;

struct PinInfo
{
    std::string_view Name;
    uint32_t Layer;
    double x, y;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PinInfo, Name, Layer, x, y)

struct Instance 
{
    int id;
    std::string_view cellType;
    double x;
    double y;
    double Rotation;
    bool xReflected;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Instance, cellType, x, y, Rotation, xReflected);

struct PadInfo {
    std::string Name;
    double x;
    double y;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PadInfo, Name, x , y);

namespace {

constexpr bool isFiller(std::string_view Name) noexcept {
    if (!Name.starts_with("sky130_fd_sc_hd__")) return true;
    constexpr std::array<std::string_view, 4> filters{"tap", "decap", "diode", "fill"}; // diode was missin

    return std::ranges::any_of(filters, [&](auto f) { return Name.find(f) != std::string_view::npos; });
}

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
    const char *gdsPath { "../puzzle.gds" };
    const char *outJSON { "..outputs/parsedCells.json"};

    gdstk::ErrorCode err { gdstk::ErrorCode::NoError };
    gdstk::Library gdsFile { gdstk::read_gds(gdsPath, 0, 0, nullptr, &err)};

    if (err != gdstk::ErrorCode::NoError) {
        std::print("error reading gds");
        return EXIT_FAILURE;
    }

    // find topcell
    gdstk::Cell *top { nullptr };
    for (auto i{0}; i < gdsFile.cell_array.count; ++i) {
    }

    //extract pin definitions
    
    // extract functional pin placements from top cell
    // extract top level io pad labels
    //
    // serializw json

}

