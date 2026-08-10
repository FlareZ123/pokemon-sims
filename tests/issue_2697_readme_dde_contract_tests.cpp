#include <cassert>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {

std::string read_repo_file(const std::filesystem::path& path) {
  for (const std::filesystem::path& root :
       {std::filesystem::current_path(), std::filesystem::current_path().parent_path()}) {
    std::ifstream input(root / path);
    if (!input) continue;
    std::ostringstream content;
    content << input.rdbuf();
    return content.str();
  }
  return {};
}

}  // namespace

int main() {
  const std::string readme = read_repo_file("README.md");
  const std::string energy_source = read_repo_file("src/trace_engine_v2/part_004.inc");
  assert(!readme.empty());
  assert(!energy_source.empty());

  // README must describe semantic Apex payment, including DDE's legal two-unit contribution:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  // https://github.com/FlareZ123/pokemon-sims/issues/2697
  assert(readme.find("Pays Apex Dragon's `[G][G][R]` attack cost with legally attached Energy; Double Dragon Energy may supply two of those Energy units while attached to Regidrago VSTAR.") != std::string::npos);
  assert(readme.find("At least two Grass Energy and one Fire Energy attached.") == std::string::npos);

  // Keep the documentation contract anchored to the production semantic predicate:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc
  // https://github.com/FlareZ123/pokemon-sims/issues/2697
  assert(energy_source.find("bool pays_apex_energy_cost(const Pokemon& pokemon) const") != std::string::npos);
  assert(energy_source.find("return is_dragon(pokemon.card) && total_energy_units(pokemon) >= 3;") != std::string::npos);
}
