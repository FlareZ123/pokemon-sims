#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <filesystem>
#include <fstream>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

std::filesystem::path artifact_path() {
  std::filesystem::path directory = std::filesystem::current_path();
  for (int depth = 0; depth < 3; ++depth) {
    const auto candidate = directory / "results" / "crobat_variant_model.csv";
    if (std::filesystem::exists(candidate)) return candidate;
    if (!directory.has_parent_path()) break;
    directory = directory.parent_path();
  }
  throw std::runtime_error("Could not locate results/crobat_variant_model.csv");
}

std::pair<std::string, std::string> variant_scenario(const std::string& line) {
  const std::size_t first = line.find(',');
  const std::size_t second = line.find(',', first + 1);
  const std::size_t third = line.find(',', second + 1);
  const std::size_t fourth = line.find(',', third + 1);
  if (first == std::string::npos || second == std::string::npos ||
      third == std::string::npos || fourth == std::string::npos) {
    throw std::runtime_error("Malformed Crobat modeling CSV row");
  }
  return {line.substr(0, first), line.substr(third + 1, fourth - third - 1)};
}

void test_checked_crobat_artifact_matches_live_registry() {
  // The checked artifact must follow the live --model-crobat registry, where the
  // registered shell's recovery cut is Klara rather than the retired Roseanne
  // result. Keeping this contract source-derived prevents another complete but
  // stale matrix from passing after a registry rename:
  // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
  // Live Crobat registry: https://github.com/FlareZ123/pokemon-sims/blob/fix/2252-crobat-compression-on-2247/src/trace_engine_v2/part_016.inc
  // Registered-shell Klara migration: https://github.com/FlareZ123/pokemon-sims/issues/1773
  // Confirmed artifact defect: https://github.com/FlareZ123/pokemon-sims/issues/2253
  std::set<std::string> expected_variants;
  for (const sim::CrobatModelingDeck& deck : sim::crobat_modeling_decks()) {
    expected_variants.insert(deck.id);
  }
  expect(expected_variants.size() == sim::crobat_modeling_decks().size(),
         "Live Crobat modeling registry contains duplicate variant IDs");
  expect(expected_variants.contains("crobat1-klara"),
         "Live Crobat modeling registry lost crobat1-klara");
  expect(!expected_variants.contains("crobat1-roseanne"),
         "Live Crobat modeling registry unexpectedly restored crobat1-roseanne");

  // Current-paper aggregate reporting has exactly the live all_scenarios() set.
  // Issue #2247 retires both full-turn-one Item-lock labels while preserving the
  // turn-two Item-lock scenarios, so the Crobat artifact must bind to that same set:
  // Official first-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/mew_rulebook_en.pdf
  // Expanded Forest of Giant Plants ban: https://www.pokemon.com/es/sol-luna-sombras-ardientes-anuncio-trimestral-sobre-lista-de-cartas-prohibidas-y-cambios-en-las-reglas/
  // Scenario correction: https://github.com/FlareZ123/pokemon-sims/issues/2247
  // Confirmed artifact defect: https://github.com/FlareZ123/pokemon-sims/issues/2253
  std::set<std::string> expected_scenarios;
  for (const sim::Scenario& scenario : sim::all_scenarios()) {
    expected_scenarios.insert(scenario.label);
  }
  expect(expected_scenarios.size() == sim::all_scenarios().size(),
         "Live aggregate scenario registry contains duplicate labels");
  expect(!expected_scenarios.contains("strict-jit-full-item-lock/go-first") &&
             !expected_scenarios.contains("strict-jit-full-item-lock/go-second"),
         "Live aggregate scenario registry restored retired full Item-lock labels");

  std::ifstream input(artifact_path());
  expect(input.good(), "Could not open checked Crobat modeling artifact");
  std::string line;
  expect(static_cast<bool>(std::getline(input, line)),
         "Checked Crobat modeling artifact is empty");

  std::set<std::string> actual_variants;
  std::set<std::string> actual_scenarios;
  std::set<std::pair<std::string, std::string>> pairs;
  std::size_t row_count = 0;
  while (std::getline(input, line)) {
    if (line.empty()) continue;
    const auto pair = variant_scenario(line);
    ++row_count;
    expect(pairs.insert(pair).second,
           "Checked Crobat modeling artifact contains a duplicate variant/scenario pair");
    actual_variants.insert(pair.first);
    actual_scenarios.insert(pair.second);
  }

  expect(actual_variants == expected_variants,
         "Checked Crobat modeling variants do not match the live source registry");
  expect(actual_scenarios == expected_scenarios,
         "Checked Crobat modeling scenarios do not match all_scenarios()");
  expect(row_count == expected_variants.size() * expected_scenarios.size(),
         "Checked Crobat modeling artifact is missing source-bound variant/scenario rows");
}
}  // namespace

int main() {
  test_checked_crobat_artifact_matches_live_registry();
}
