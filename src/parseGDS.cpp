#include <print>
#include <gdstk/gdstk.hpp>
#include <string>
#include <string_view>
#include <ranges>
#include <algorithm>

struct Layer {
    uint32_t layer;
    uint32_t dataType;
    std::string_view name;
};

// straigt aaway pastin ts
static constexpr Layer ROUTING_LAYERS[] = {
    {67, 20, "li1"},
    {67, 44, "mcon"},
    {68, 20, "met1"},
    {68, 44, "via"},
    {69, 20, "met2"},
    {69, 44, "via2"},
    {70, 20, "met3"},
    {70, 44, "via3"},
    {71, 20, "met4"},
    {71, 44, "via4"},
    {72, 20, "met5"},
};

constexpr int ROUTING_LAYER_COUNT { 11 };

constexpr bool isRoutingTag(gdstk::Tag tag) noexcept {
    const uint32_t layer { gdstk::get_layer(tag) };
    const uint32_t type { gdstk::get_type(tag) };
    
    for (int i{0}; i < ROUTING_LAYER_COUNT; ++i) {
        if( ROUTING_LAYERS[i].layer == layer && ROUTING_LAYERS[i].dataType == type) {return true;}
    }
    return false;
}

constexpr std::string_view layerNameFor(uint32_t layer, uint32_t datatype) {
    for (const auto& [lr, dt, name] : ROUTING_LAYERS) {
        if (lr == layer && dt == datatype) return name; 
    }
    return "not found -:ERR";

}

constexpr bool isFiller(std::string_view Name) noexcept {
    if (!Name.starts_with("sky130_fd_sc_hd__")) return true;
    constexpr std::array<std::string_view, 4> filters{"tap", "decap", "diode", "fill"}; // diode was missin

    return std::ranges::any_of(filters, [&](auto f) { return Name.find(f) != std::string_view::npos; });
}

struct CellRec {
};

struct PolygonRecord {
};


void decompRectinilinear() {} // rectilinear polygon -> rectangle, get with ai idk

int main() {
    std::println("parseGDS compiled successfully with GDSTK!");
    return 0;
}

