#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

// Unified-test generator contract: every issue test contributes exactly one
// EngineTestAccess block, even when the regression needs no private Engine state:
// https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py#L119-L128
struct EngineTestAccess {};

}  // namespace sim

namespace {

void require(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_live_lookup_rejects_xy_double_dragon_energy_model() {
  // Pokémon Support says XY cards are unavailable for play in Pokémon TCG Live,
  // while Double Dragon Energy is XY—Roaring Skies 97/108:
  // https://support.pokemon.com/hc/en-us/articles/6489934466708-Pok%C3%A9mon-TCG-Live-Migration-FAQ-from-the-Pok%C3%A9mon-TCG-Online
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://github.com/FlareZ123/pokemon-sims/issues/2332
  require(sim::tcg_live_deck_by_id("regidrago-dde-model") == nullptr,
          "Production TCG Live lookup exposed the paper-only DDE model.");
}

void test_paper_mechanics_model_remains_available_to_focused_tests() {
  // The paper Expanded mechanics model remains available internally for the
  // issue-2238 DDE rules tests; only the production Live lookup excludes it:
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // https://github.com/FlareZ123/pokemon-sims/issues/2238
  // https://github.com/FlareZ123/pokemon-sims/issues/2332
  require(sim::deck_by_id("regidrago-dde-model") != nullptr,
          "Focused paper mechanics lookup lost the DDE model.");
}

void test_registered_live_decks_remain_reachable() {
  require(sim::tcg_live_deck_by_id("regidrago-shell") != nullptr,
          "Live lookup lost regidrago-shell.");
  require(sim::tcg_live_deck_by_id("regidrago-pineco") != nullptr,
          "Live lookup lost regidrago-pineco.");
  require(sim::tcg_live_deck_by_id("unknown") == nullptr,
          "Live lookup accepted an unknown deck.");
}

}  // namespace

int main() {
  try {
    test_live_lookup_rejects_xy_double_dragon_energy_model();
    test_paper_mechanics_model_remains_available_to_focused_tests();
    test_registered_live_decks_remain_reachable();
    std::cout << "Issue 2332 TCG Live card-pool tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
