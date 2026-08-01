#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static bool direct_crispin_route_available(const Engine& engine) {
    return engine.fss_should_take_crispin_over_redundant_blender();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario exact_scenario() {
  return sim::Scenario{"issue-2013-fss-crispin-prize-k1",
                       sim::DciProfile::StrictJit,
                       sim::LockMode::None, true, 3};
}

sim::State exact_route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 2, 1, 0,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::Dragapult, sim::Card::Grass,
                sim::Card::MysteriousTreasure,
                sim::Card::RegidragoVstar,
                sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Crispin, sim::Card::BrilliantBlender,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
  return state;
}

bool route_available(const bool deck_seen, const bool prizes_revealed) {
  const sim::Scenario scenario = exact_scenario();
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{2013};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, exact_route_state(), deck_seen, prizes_revealed);
  return sim::EngineTestAccess::direct_crispin_route_available(engine);
}

void test_both_k1_provenances_and_k0_boundary() {
  // A legal deck search and a complete Hisuian Heavy Ball Prize inspection both
  // establish exact fixed-list K1, while true K0 must remain rejected:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Official Prize, Tool, VSTAR Power, Supporter, Item, discard, search, attachment, evolution, Ability, retreat, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Existing physical-route regression: https://github.com/FlareZ123/pokemon-sims/blob/main/tests/issue_976_fss_direct_crispin_tests.cpp
  // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2013
  expect(route_available(true, false),
         "The deck-search K1 direct-Crispin route was rejected.");
  expect(route_available(false, true),
         "The Prize-inspection K1 direct-Crispin route was rejected.");
  expect(!route_available(false, false),
         "The direct-Crispin selector used exact hidden composition before K1.");
}

}  // namespace

int main() {
  test_both_k1_provenances_and_k0_boundary();
  return 0;
}
