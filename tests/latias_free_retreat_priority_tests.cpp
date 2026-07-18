#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool promote_ready_benched_vstar(Engine& engine) {
    return engine.promote_ready_benched_vstar();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void set_ready_tapu_state(sim::Engine& engine, const bool include_latias) {
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None}};
  if (include_latias) {
    state.bench.push_back(
        sim::Pokemon{sim::Card::LatiasEx, 3, 0, 0, sim::Tool::None});
  }
  state.hand = {sim::Card::Fire};
  state.discard = {sim::Card::MegaDragonite};
}

void test_free_latias_retreat_precedes_paid_tapu_route() {
  const sim::Scenario scenario{"issue-905-latias-free-retreat",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng(90501);
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  set_ready_tapu_state(engine, true);

  // Latias ex makes the Basic Active Tapu Lele-GX's Retreat Cost zero. The free
  // promotion must preserve Fire Energy and the once-per-turn manual attachment:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/905
  assert(sim::EngineTestAccess::promote_ready_benched_vstar(engine));

  const sim::State& state = sim::EngineTestAccess::state(engine);
  assert(state.active.has_value());
  assert(state.active->card == sim::Card::RegidragoVstar);
  assert(state.retreat_used);
  assert(!state.manual_energy_used);
  assert(contains(state.hand, sim::Card::Fire));
  assert(!contains(state.discard, sim::Card::Fire));
}

void test_paid_tapu_route_remains_when_latias_is_absent() {
  const sim::Scenario scenario{"issue-905-paid-retreat-no-latias",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 4};
  std::mt19937_64 rng(90502);
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  set_ready_tapu_state(engine, false);

  // Without Skyliner in play, Tapu Lele-GX still needs its printed one-Colorless
  // Retreat Cost, preserving the confirmed paid fallback from issue #802:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/802
  // https://github.com/FlareZ123/pokemon-sims/issues/905
  assert(sim::EngineTestAccess::promote_ready_benched_vstar(engine));

  const sim::State& state = sim::EngineTestAccess::state(engine);
  assert(state.active.has_value());
  assert(state.active->card == sim::Card::RegidragoVstar);
  assert(state.retreat_used);
  assert(state.manual_energy_used);
  assert(!contains(state.hand, sim::Card::Fire));
  assert(contains(state.discard, sim::Card::Fire));
}

void test_paid_tapu_route_remains_when_skyliner_is_locked() {
  const sim::Scenario scenario{"issue-905-paid-retreat-ability-lock",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::FullRuleBoxAbility, true, 4};
  std::mt19937_64 rng(90503);
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  set_ready_tapu_state(engine, true);

  // The modeled Rule Box Ability lock suppresses Latias ex's Skyliner. The free
  // route is unavailable, so the same paid Tapu fallback remains legal:
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/905
  assert(sim::EngineTestAccess::promote_ready_benched_vstar(engine));

  const sim::State& state = sim::EngineTestAccess::state(engine);
  assert(state.active.has_value());
  assert(state.active->card == sim::Card::RegidragoVstar);
  assert(state.retreat_used);
  assert(state.manual_energy_used);
  assert(!contains(state.hand, sim::Card::Fire));
  assert(contains(state.discard, sim::Card::Fire));
}

}  // namespace

int main() {
  test_free_latias_retreat_precedes_paid_tapu_route();
  test_paid_tapu_route_remains_when_latias_is_absent();
  test_paid_tapu_route_remains_when_skyliner_is_locked();
  return 0;
}
