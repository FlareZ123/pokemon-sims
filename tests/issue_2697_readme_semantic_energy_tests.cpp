#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {
std::string read_readme() {
  const std::filesystem::path root_readme{"README.md"};
  const std::filesystem::path build_readme{"../README.md"};
  const std::filesystem::path path =
      std::filesystem::exists(root_readme) ? root_readme : build_readme;
  std::ifstream input(path);
  if (!input) throw std::runtime_error("unable to open README.md");
  std::ostringstream text;
  text << input.rdbuf();
  return text.str();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
}  // namespace

int main() {
  try {
    const std::string readme = read_readme();

    // The user-facing ready-state contract must describe semantic Apex payment.
    // Double Dragon Energy provides every Energy type and two Energy at a time:
    // https://api.pokemontcg.io/v2/cards/xy6-97
    // Apex Dragon costs [Grass][Grass][Fire]:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // Engine semantic helper:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc
    // Confirmed documentation bug:
    // https://github.com/FlareZ123/pokemon-sims/issues/2697
    expect(readme.find("Semantic payment of Apex Dragon's `[Grass][Grass][Fire]` Energy cost") !=
               std::string::npos,
           "README lost the semantic Apex Energy readiness wording.");
    expect(readme.find("At least two Grass Energy and one Fire Energy attached") ==
               std::string::npos,
           "README regressed to raw Basic Grass/Fire attachment wording.");

    std::cout << "Issue 2697 README semantic Energy contract passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
