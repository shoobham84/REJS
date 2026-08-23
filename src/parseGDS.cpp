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

struct Instance 
{
    int id;
    std::string_view cellType;
    double x;
    double y;
    double Rotation;
    bool xReflected;
};


constexpr bool isFiller(std::string_view Name) noexcept {
    if (!Name.starts_with("sky130_fd_sc_hd__")) return true;
    constexpr std::array<std::string_view, 4> filters{"tap", "decap", "diode", "fill"}; // diode was missin

    return std::ranges::any_of(filters, [&](auto f) { return Name.find(f) != std::string_view::npos; });
}


int main() {
    std::println("parseGDS compiled successfully with GDSTK!");
    return 0;
}

