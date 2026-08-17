#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sim {

struct EngineTestAccess {};

}  // namespace sim

namespace {

std::filesystem::path repo_root() {
  const std::filesystem::path current = std::filesystem::current_path();
  if (std::filesystem::exists(current / "SIM-PLAN.md")) return current;
  if (std::filesystem::exists(current.parent_path() / "SIM-PLAN.md")) {
    return current.parent_path();
  }
  throw std::runtime_error("Unable to locate the repository root.");
}

std::string read_text(const std::filesystem::path& path) {
  std::ifstream input(path);
  if (!input) throw std::runtime_error("Unable to read " + path.string());
  std::ostringstream content;
  content << input.rdbuf();
  return content.str();
}

void expect_contains(const std::string& text, const std::string& expected,
                     const char* message) {
  if (text.find(expected) == std::string::npos) {
    throw std::runtime_error(message);
  }
}

}  // namespace

int main() {
  const std::filesystem::path root = repo_root();
  const std::string plan = read_text(root / "SIM-PLAN.md");
  const std::string payload_source =
      read_text(root / "src" / "trace_engine_v2" / "core" / "card_catalog.inc");

  // Appletun is a Dragon Pokémon whose attacks are available to Apex Dragon, and
  // the executable includes it in the recipe-gated readiness set:
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
  // https://github.com/FlareZ123/pokemon-sims/issues/1489
  expect_contains(plan, "recipe-gated Appletun `sv8-140`",
                  "SIM-PLAN.md omits recipe-gated Appletun readiness.");
  expect_contains(payload_source, "card == Card::Appletun",
                  "The executable Appletun payload predicate drifted.");
  return 0;
}
