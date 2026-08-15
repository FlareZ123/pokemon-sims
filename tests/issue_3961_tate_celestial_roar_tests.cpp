#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
  static bool play_tate_switch(Engine& engine) { return engine.play_tate_switch(); }
  static bool use_celestial_roar(Engine& engine) {
    return engine.use_celestial_roar();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_tate_switch_preserves_vstar_and_enables_celestial_roar() {
  using namespace sim;
  const Scenario scenario{"issue-3961-tate-switch", DciProfile::StrictJit,
                          LockMode::None, false, 5};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(3961);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 1;
  state.active = Pokemon{Card::TapuLeleGX, 1, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoV, 1, 1, 0, Tool::None}};
  state.hand = {Card::TateLiza, Card::RegidragoVstar};
  state.deck = {Card::Grass, Card::Fire, Card::Grass};

  // Tate & Liza may switch the Active with any Benched Pokemon. Celestial Roar
  // costs [C], and a Basic played this turn cannot evolve until the next turn.
  // Preserve the held VSTAR while advancing the same-turn Basic through its legal attack.
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Regidrago V / Celestial Roar [C]: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Evolution, Supporter, switching, attack, and turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3961
  EngineTestAccess::choose_supporter(engine);

  assert(state.supporter_used);
  assert(state.active.has_value());
  assert(state.active->card == Card::RegidragoV);
  assert(state.active->grass == 1);
  assert(contains(state.hand, Card::RegidragoVstar));
  assert(!contains(state.hand, Card::TateLiza));
  assert(contains(state.discard, Card::TateLiza));
  assert(state.bench.size() == 1);
  assert(state.bench.front().card == Card::TapuLeleGX);

  assert(EngineTestAccess::use_celestial_roar(engine));
  assert(contains(state.hand, Card::RegidragoVstar));
}

void test_tate_switch_does_not_preempt_prior_turn_evolution_route() {
  using namespace sim;
  const Scenario scenario{"issue-3961-evolution-guard", DciProfile::StrictJit,
                          LockMode::None, false, 5};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(3962);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::TapuLeleGX, 2, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoV, 1, 1, 0, Tool::None}};
  state.hand = {Card::TateLiza, Card::RegidragoVstar};
  state.deck = {Card::Grass, Card::Fire, Card::Grass};

  // The prior-turn Basic is evolution-legal now. Spending Tate merely to promote
  // Regidrago V would preempt the stronger held-VSTAR evolution route, so the
  // Celestial Roar target selector is intentionally limited to same-turn Basics.
  // Regidrago VSTAR evolution: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Route-priority specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression history: https://github.com/FlareZ123/pokemon-sims/pull/3973
  assert(!EngineTestAccess::play_tate_switch(engine));
  assert(!state.supporter_used);
  assert(state.active.has_value());
  assert(state.active->card == Card::TapuLeleGX);
  assert(state.bench.size() == 1);
  assert(state.bench.front().card == Card::RegidragoV);
  assert(contains(state.hand, Card::RegidragoVstar));
  assert(contains(state.hand, Card::TateLiza));
}

}  // namespace

int main() {
  test_tate_switch_preserves_vstar_and_enables_celestial_roar();
  test_tate_switch_does_not_preempt_prior_turn_evolution_route();
  return 0;
}
