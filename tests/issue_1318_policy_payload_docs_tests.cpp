#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {

std::filesystem::path find_repo_root() {
  std::filesystem::path candidate = std::filesystem::current_path();
  for (int depth = 0; depth < 8; ++depth) {
    if (std::filesystem::exists(candidate / "docs/POLICY_DECISIONS.md") &&
        std::filesystem::exists(candidate / "src/trace_engine_v2/part_001.inc")) {
      return candidate;
    }
    if (!candidate.has_parent_path()) break;
    candidate = candidate.parent_path();
  }
  throw std::runtime_error("Could not locate the repository root for the policy contract.");
}

std::string read_text(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) throw std::runtime_error("Could not read " + path.string());
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

void test_policy_names_every_executable_payload() {
  const std::filesystem::path root = find_repo_root();
  const std::string source = read_text(root / "src/trace_engine_v2/part_001.inc");
  const std::string policy = read_text(root / "docs/POLICY_DECISIONS.md");

  const std::size_t function_start = source.find("constexpr bool is_payload(const Card card)");
  const std::size_t function_end = source.find("\n}\n", function_start);
  if (function_start == std::string::npos || function_end == std::string::npos) {
    throw std::runtime_error("Could not locate the executable is_payload predicate.");
  }

  const std::string function = source.substr(function_start, function_end - function_start);
  const std::regex member_pattern(R"(Card::([A-Za-z0-9_]+))");
  std::set<std::string> executable_members;
  for (std::sregex_iterator it(function.begin(), function.end(), member_pattern), end;
       it != end; ++it) {
    executable_members.insert((*it)[1].str());
  }

  const std::map<std::string, std::string> documented_names{
      {"Dragapult", "Dragapult ex"},
      {"MegaDragonite", "Mega Dragonite ex"},
      {"DialgaGX", "Dialga-GX"},
      {"GoodraVstar", "Hisuian Goodra VSTAR"},
      {"Appletun", "Appletun"},
  };
  std::set<std::string> mapped_members;
  for (const auto& [member, unused] : documented_names) {
    static_cast<void>(unused);
    mapped_members.insert(member);
  }
  if (executable_members != mapped_members) {
    throw std::runtime_error("The policy contract mapping drifted from is_payload.");
  }

  const std::size_t section_start = policy.find("### A/S payload classification");
  const std::size_t section_end = policy.find("\n## ", section_start);
  if (section_start == std::string::npos || section_end == std::string::npos) {
    throw std::runtime_error("Could not locate the policy payload classification.");
  }
  const std::string section = policy.substr(section_start, section_end - section_start);

  // The canonical policy must name every member of the executable readiness set.
  // Appletun card data and Apex Dragon establish its modeled payload role:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_001.inc#L128-L137
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#L15-L19
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1318
  for (const auto& [member, display_name] : documented_names) {
    static_cast<void>(member);
    if (section.find(display_name) == std::string::npos) {
      throw std::runtime_error("Policy payload classification omitted " + display_name + ".");
    }
  }
  if (section.find("https://api.pokemontcg.io/v2/cards/sv8-140") == std::string::npos ||
      section.find("https://api.pokemontcg.io/v2/cards/swsh12-136") == std::string::npos) {
    throw std::runtime_error("Policy payload classification omitted direct Appletun or Apex Dragon sources.");
  }
}

}  // namespace

int main() {
  test_policy_names_every_executable_payload();
  std::cout << "Issue 1318 policy payload documentation test passed.\n";
  return 0;
}
