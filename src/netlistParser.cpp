#include "netlistParser.hpp"
#include <regex>
#include <fstream>
#include <print>
#include <sstream>
#include <charconv>


namespace PN  // parse netlist
{
	

[[nodiscard]] static constexpr std::string_view trim(std::string_view str) noexcept {
	const auto first = str.find_first_not_of(" \t\r\n");
	if (first == std::string_view::npos) 
		return {};

	const auto last = str.find_last_not_of(" \t\r\n");
	return str.substr(first, (last-first+1));
}

static void split_and_append_trimmed(std::string_view input, std::vector<std::string>& dest) {
	auto start{0uz};
	while (start < input.size()) {
		const auto end { input.find(',', start) };
		const auto token { end == std::string_view::npos ?
				trim(input.substr(start))
				: trim(input.substr(start, end - start))
		};

		if (!token.empty()) {
			dest.emplace_back(token);
		}

		if (end == std::string_view::npos) break;
		start = end+1;
	}
}

void printNetlistSummary(const NetlistAST& ast) {
    auto format_list = [](const std::vector<std::string>& items) -> std::string {
        if (items.empty()) return "()";
        std::ostringstream oss;
        oss << "(";
        for (std::size_t i = 0; i < items.size(); ++i) {
            oss << items[i] << (i + 1 < items.size() ? ", " : "");
        }
        oss << ")";
        return oss.str();
    };

    std::println("Netlist ast debug: ");
    std::println("Module Name: {}", ast.moduleName);
    std::println("Total instances: {}", ast.instances.size());
    std::println("Logic Gates: {}", ast.logicGateIndices.size());
    std::println(" Flip Flops:  {}", ast.flipFlopIndices.size());
    std::println("  All Cells:  {}", ast.allNets.size());
    std::println("Top level inputs: {}  {}", ast.inputs.size(), format_list(ast.inputs));
    std::println("Top level outputs: {}  {}", ast.outputs.size(), format_list(ast.outputs));
    std::println("Direct assing: {}", ast.assignments.size());
}

static const std::regex module_regex{R"(module\s+(\w+)\s*\((.*?)\);)", std::regex::optimize};
static const std::regex input_regex{R"(input\s+wire\s+(.*?);)", std::regex::optimize};
static const std::regex output_regex{R"(output\s+wire\s+(.*?);)", std::regex::optimize};
static const std::regex wire_regex{R"(wire\s+(net_\d+);)", std::regex::optimize};
static const std::regex assign_regex{R"(assign\s+(.*?)\s*=\s*(.*?);)", std::regex::optimize};
static const std::regex inst_regex{R"((sky130_\w+)\s+(\w+)\s*\((.*?)\);)", std::regex::optimize};
static const std::regex port_regex{R"(\.(\w+)\((.*?)\))", std::regex::optimize};


// parses our created netlist into an abstract syntax tree 
[[nodiscard]] NetlistAST parseVerilogNetlist(const std::filesystem::path& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error(std::format("Cannot open Verilog netlist file: {}", filepath.string()));
    }

    NetlistAST ast;
    std::string line;
    std::smatch m;

    while (std::getline(file, line)) {
        const std::string_view line_view = trim(line);
        if (line_view.empty() || line_view.starts_with("//") || line_view.starts_with('`')) {
            continue;
        }

        if (std::regex_search(line, m, module_regex)) {
            ast.moduleName = m[1].str();
        } else if (std::regex_search(line, m, input_regex)) {
            split_and_append_trimmed(m[1].str(), ast.inputs);
        } else if (std::regex_search(line, m, output_regex)) {
            split_and_append_trimmed(m[1].str(), ast.outputs);
        } else if (std::regex_search(line, m, wire_regex)) {
            ast.allNets.emplace(m[1].str());
        } else if (std::regex_search(line, m, assign_regex)) {
            std::string lhs{trim(m[1].str())};
            std::string rhs{trim(m[2].str())};
            ast.assignments.emplace(std::move(lhs), std::move(rhs));
        } else if (std::regex_search(line, m, inst_regex)) {
            CellInstance inst;
            inst.cellType = m[1].str();
            inst.instanceName = m[2].str();

            // Safe, non-throwing integer extraction via std::from_chars
            if (std::string_view(inst.instanceName).starts_with("inst_")) {
                const auto id_str = std::string_view(inst.instanceName).substr(5);
                std::uint32_t parsed_id = 0;
                const auto [ptr, ec] = std::from_chars(id_str.data(), id_str.data() + id_str.size(), parsed_id);
                inst.id = (ec == std::errc{}) ? parsed_id : static_cast<std::uint32_t>(ast.instances.size());
            } else {
                inst.id = static_cast<std::uint32_t>(ast.instances.size());
            }

            const std::string ports_str = m[3].str();
            for (std::sregex_iterator it(ports_str.begin(), ports_str.end(), port_regex), end; it != end; ++it) {
                const std::smatch& pm = *it;
                auto& port = inst.ports.emplace_back(PinBinding{
                    .pinName = pm[1].str(),
                    .netName = std::string{trim(pm[2].str())}
                });

                if (std::string_view(port.netName).starts_with("net_")) {
                    ast.allNets.insert(port.netName);
                }
            }

            ast.instances.emplace_back(std::move(inst));
        }
    }

    ast.categorizeCells();
    return ast;
}

}

#ifndef PARSER_LIB
int main(int argc, char** argv) {
    const std::filesystem::path netlistFile = (argc > 1)
        ? std::filesystem::path(argv[1])
        : std::filesystem::path("outputs/extracted_netlist.v");

    try {
        const auto ast { PN::parseVerilogNetlist(netlistFile) };
        PN::printNetlistSummary(ast);
    }
    catch (const std::exception& error) {
        std::println(stderr, "Error parsing netlist: {}", error.what());
        return EXIT_FAILURE;
    }
}
#endif
