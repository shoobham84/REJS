#include "netlistParser.hpp"
#include <regex>


namespace SAT
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

static const std::regex module_regex{R"(module\s+(\w+)\s*\((.*?)\);)", std::regex::optimize};
static const std::regex input_regex{R"(input\s+wire\s+(.*?);)", std::regex::optimize};
static const std::regex output_regex{R"(output\s+wire\s+(.*?);)", std::regex::optimize};
static const std::regex wire_regex{R"(wire\s+(net_\d+);)", std::regex::optimize};
static const std::regex assign_regex{R"(assign\s+(.*?)\s*=\s*(.*?);)", std::regex::optimize};
static const std::regex inst_regex{R"((sky130_\w+)\s+(\w+)\s*\((.*?)\);)", std::regex::optimize};
static const std::regex port_regex{R"(\.(\w+)\((.*?)\))", std::regex::optimize};


}

