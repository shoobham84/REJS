#include <string_view>
#include <vector>
#include <string>
#include <ranges>
#include <optional>

namespace SAT
{

struct PinBinding {
	std::string pinName;   // A, B, CLK, Y
	std::string netName;   // net_412, 1'b0, rst_n
};

struct CellInstance {
	int id{};
	std::string cellType;  // sky130_fd_sc_hd__nand2_2  etc
	std::string instanceName; // inst_42
	std::vector<PinBinding> ports;

	[[nodiscard]] std::optional<std::string_view> getNet(std::string_view pin) const noexcept {
		const auto it = std::ranges::find(ports, pin, &PinBinding::pinName);
		if (it != ports.end())
			return it->netName;

		return std::nullopt;
	}

	[[nodiscard]] bool isSequential() const noexcept {
		return cellType.find("__df") != std::string::npos;
	}
};

}
