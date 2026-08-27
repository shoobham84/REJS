#include <string_view>
#include <vector>
#include <string>
#include <ranges>
#include <optional>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>

namespace PN
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

// netlist abstract syntax tree
struct NetlistAST
{
	std::string moduleName;
	std::vector<std::string> inputs;
	std::vector<std::string> outputs;

	std::unordered_set<std::string> allNets;

	std::vector<CellInstance> instances;

	std::unordered_map<std::string, std::string> assignments;


	std::vector<size_t> flipFlopIndices;
	std::vector<size_t> logicGateIndices;

	void categorizeCells() {
		flipFlopIndices.clear();
		logicGateIndices.clear();

		flipFlopIndices.reserve(instances.size() / 4);
		logicGateIndices.reserve(instances.size());

		for (auto i{0uz}; i < instances.size(); ++i) {
			if (instances[i].isSequential())
				flipFlopIndices.push_back(i);
			else
				logicGateIndices.push_back(i);
		}
	}
};

void printNetlistSummary(const NetlistAST& ast);


[[nodiscard]] NetlistAST parseVerilogNetlist(const std::filesystem::path& filepath);

}
