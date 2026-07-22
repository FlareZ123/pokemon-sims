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
    if (std::filesystem::exists(candidate / "docs/MODEL_ASSUMPTIONS.md") &&
        std::filesystem::exists(candidate / "README.md") &&
        std::filesystem::exists(candidate / "src/trace_engine_v2/part_001.inc")) {
      return candidate;
    }
    if (!candidate.has_parent_path()) break;
    candidate = candidate.parent_path();
  }
  throw std::runtime_error("Could not locate the repository root for the model contract.");
}

std::string read_text(const std::filesystem::path& path) {
  std::ifstream input(path, std::ios::binary);
  if (!input) throw std::runtime_error("Could not read " + path.string());
  std::ostringstream buffer;
  buffer << input.rdbuf();
  return buffer.str();
}

std::string section_between(const std::string& text, const std::string& heading,
                            const std::string& next_heading) {
  const std::size_t heading_text = text.find(heading);
  if (heading_text == std::string::npos) {
    throw std::runtime_error("Could not locate documentation heading: " + heading);
  }
  const std::size_t start = text.rfind("###", heading_text);
  const std::size_t end = text.find(next_heading, heading_text);
  if (start == std::string::npos || end == std::string::npos) {
    throw std::runtime_error("Could not isolate documentation section: " + heading);
  }
  return text.substr(start, end - start);
}

void test_model_assumptions_names_every_executable_payload() {
  const std::filesystem::path root = find_repo_root();
  const std::string source = read_text(root / "src/trace_engine_v2/part_001.inc");
  const std::string model = read_text(root / "docs/MODEL_ASSUMPTIONS.md");
  const std::string readme = read_text(root / "README.md");

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
    throw std::runtime_error("The model-assumptions mapping drifted from is_payload.");
  }

  const std::string model_section =
      section_between(model, "A or S payload", "\n## DCI implementation");

  // The probability definition must name every member accepted by the executable
  // readiness predicate, including recipe-gated Appletun:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_001.inc#L128-L137
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#L82-L91
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1320
  for (const auto& [member, display_name] : documented_names) {
    static_cast<void>(member);
    if (model_section.find(display_name) == std::string::npos) {
      throw std::runtime_error("Model assumptions omitted payload " + display_name + ".");
    }
  }
  if (model_section.find("https://api.pokemontcg.io/v2/cards/sv8-140") == std::string::npos ||
      model_section.find("https://api.pokemontcg.io/v2/cards/swsh12-136") == std::string::npos) {
    throw std::runtime_error("Model assumptions omitted direct Appletun or Apex Dragon sources.");
  }

  const std::size_t readme_start = readme.find("## Ready-state and T5 policy");
  if (readme_start == std::string::npos) {
    throw std::runtime_error("Could not locate the README ready-state contract.");
  }
  const std::string readme_section = readme.substr(readme_start);

  // README and model assumptions must preserve the same recipe-gated Appletun rule:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#L72-L81
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/1320
  const std::string recipe_gate = "Appletun is eligible only in a recipe that contains it";
  if (readme_section.find(recipe_gate) == std::string::npos ||
      model_section.find("when the selected recipe contains it") == std::string::npos) {
    throw std::runtime_error("README and model assumptions disagree on Appletun recipe gating.");
  }
}

}  // namespace

int main() {
  test_model_assumptions_names_every_executable_payload();
  std::cout << "Issue 1320 model-assumptions payload documentation test passed.\n";
  return 0;
}
