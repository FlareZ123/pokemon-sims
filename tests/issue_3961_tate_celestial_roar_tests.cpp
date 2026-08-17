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
  static bool play_issue_3961_route(Engine& engine) {
    return engine.issue_3961_play_tate_celestial_roar_preservation_route();
  }
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

  // Tate & Liza can switch the Active with any Benched Pokemon. Celestial Roar
  // costs [C], and the same-turn Basic cannot use the ordinary evolution action.
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

void test_tate_route_does_not_preempt_prior_turn_evolution() {
  using namespace sim;
  const Scenario scenario{"issue-3961-prior-turn-guard", DciProfile::StrictJit,
                          LockMode::None, false, 5};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(3962);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::TapuLeleGX, 2, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoV, 1, 1, 0, Tool::None}};
  state.hand = {Card::TateLiza, Card::RegidragoVstar};

  // A prior-turn Regidrago V is evolution-legal, so the #3961 fallback must leave
  // the Supporter and board untouched for the stronger deterministic VSTAR route.
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Failed broad predecessor: https://github.com/FlareZ123/pokemon-sims/pull/3973
  assert(!EngineTestAccess::play_issue_3961_route(engine));
  assert(!state.supporter_used);
  assert(state.active.has_value());
  assert(state.active->card == Card::TapuLeleGX);
  assert(state.bench.size() == 1);
  assert(state.bench.front().card == Card::RegidragoV);
  assert(contains(state.hand, Card::RegidragoVstar));
  assert(contains(state.hand, Card::TateLiza));
}

void test_tate_route_yields_to_later_turn_latias_connector() {
  using namespace sim;
  const Scenario scenario{"issue-3961-later-turn-guard", DciProfile::StrictJit,
                          LockMode::None, true, 5};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(3964);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 3;
  state.active = Pokemon{Card::Oricorio, 1, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::TapuLeleGX, 2, 0, 0, Tool::None},
                 Pokemon{Card::RegidragoV, 3, 2, 0, Tool::ForestSealStone}};
  state.hand = {Card::TateLiza, Card::QuickBall, Card::RegidragoVstar,
                Card::ProfessorBurnet, Card::Fire};
  state.deck = {Card::Grass, Card::Fire, Card::LatiasEx,
                Card::MegaDragonite, Card::Dragapult, Card::RegidragoV};
  state.discard = {Card::StevensResolve, Card::Crispin};

  // The registered turn-3 seed-23 route uses Tate & Liza as Quick Ball's replaced
  // switch resource, searches Latias ex, then preserves Burnet for the T4 payload.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Route witness: https://github.com/FlareZ123/pokemon-sims/issues/1403
  // Connector-domination policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  assert(!EngineTestAccess::play_issue_3961_route(engine));
  assert(!state.supporter_used);
  assert(state.active.has_value());
  assert(state.active->card == Card::Oricorio);
  assert(contains(state.hand, Card::TateLiza));
  assert(contains(state.hand, Card::RegidragoVstar));
}

void test_tate_route_requires_celestial_roar_energy() {
  using namespace sim;
  const Scenario scenario{"issue-3961-energy-guard", DciProfile::StrictJit,
                          LockMode::None, false, 5};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(3963);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 1;
  state.active = Pokemon{Card::TapuLeleGX, 1, 0, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoV, 1, 0, 0, Tool::None}};
  state.hand = {Card::TateLiza, Card::RegidragoVstar};

  // Celestial Roar costs one Colorless Energy, so switching to an unpowered Basic
  // would spend the Supporter without preserving the immediate attack route.
  // Regidrago V / Celestial Roar [C]: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Attack and Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  assert(!EngineTestAccess::play_issue_3961_route(engine));
  assert(!state.supporter_used);
  assert(state.active.has_value());
  assert(state.active->card == Card::TapuLeleGX);
  assert(contains(state.hand, Card::RegidragoVstar));
  assert(contains(state.hand, Card::TateLiza));
}

}  // namespace

int main() {
  test_tate_switch_preserves_vstar_and_enables_celestial_roar();
  test_tate_route_does_not_preempt_prior_turn_evolution();
  test_tate_route_yields_to_later_turn_latias_connector();
  test_tate_route_requires_celestial_roar_energy();
  return 0;
}
