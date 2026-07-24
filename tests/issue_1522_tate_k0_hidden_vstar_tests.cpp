#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

// Paired public-state regression: https://github.com/FlareZ123/pokemon-sims/issues/1522
#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = known;
  }

  static bool tate_route_completes(const Engine& engine) {
    return engine.tate_draw_has_held_non_supporter_completion();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::DeckRecipe issue_recipe() {
  return {
      {sim::Card::RegidragoV, 1},         {sim::Card::RegidragoVstar, 1},
      {sim::Card::MegaDragonite, 1},      {sim::Card::TateLiza, 1},
      {sim::Card::MysteriousTreasure, 1}, {sim::Card::Crispin, 1},
      {sim::Card::Serena, 1},             {sim::Card::Gladion, 1},
      {sim::Card::Arven, 1},              {sim::Card::QuickBall, 1},
      {sim::Card::EarthenVessel, 1},      {sim::Card::LatiasEx, 1},
      {sim::Card::TapuLeleGX, 1},         {sim::Card::Grass, 2},
      {sim::Card::Fire, 1},
  };
}

sim::State paired_state(const bool vstar_in_deck) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::TateLiza, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};
  state.deck = {vstar_in_deck ? sim::Card::RegidragoVstar
                              : sim::Card::Crispin,
                sim::Card::Serena, sim::Card::Gladion};
  state.prizes = {vstar_in_deck ? sim::Card::Crispin
                                : sim::Card::RegidragoVstar,
                  sim::Card::Arven, sim::Card::QuickBall,
                  sim::Card::EarthenVessel, sim::Card::LatiasEx,
                  sim::Card::TapuLeleGX};
  return state;
}

bool tate_route_completes(sim::State state, const bool known) {
  const sim::Scenario scenario{"issue-1522", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = issue_recipe();
  std::mt19937_64 rng{1522};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return sim::EngineTestAccess::tate_route_completes(engine);
}

void test_k0_tate_decision_ignores_hidden_vstar_location() {
  // The public hand can legally pay Mysterious Treasure's one-card discard and a
  // Regidrago VSTAR remains possible from fixed copy counts. Before the search,
  // moving that hidden VSTAR between deck and Prizes cannot change the Tate choice:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core search, Supporter, discard, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K0/K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1522
  const bool deck_result = tate_route_completes(paired_state(true), false);
  const bool prize_result = tate_route_completes(paired_state(false), false);
  expect(deck_result == prize_result,
         "K0 Tate decision still depends on hidden VSTAR placement");
  expect(deck_result,
         "K0 public Mysterious Treasure completion was not recognized");
}

void test_k1_tate_decision_uses_legally_inspected_vstar_location() {
  // After a legal inspection establishes K1, the exact Mysterious Treasure target
  // availability may distinguish the two states:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1 begins only during legal effect resolution: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug and required positive control: https://github.com/FlareZ123/pokemon-sims/issues/1522
  expect(tate_route_completes(paired_state(true), true),
         "K1 failed to preserve the observable deck VSTAR route");
  expect(!tate_route_completes(paired_state(false), true),
         "K1 treated a known prized VSTAR as a Treasure target");
}

}  // namespace

int main() {
  test_k0_tate_decision_ignores_hidden_vstar_location();
  test_k1_tate_decision_uses_legally_inspected_vstar_location();
  return 0;
}
