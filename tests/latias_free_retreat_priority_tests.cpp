#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool resolve_final_promotion_and_attachment(Engine& engine) {
    return engine.resolve_final_promotion_and_attachment();
  }
};

}  // namespace sim

namespace {

const sim::DeckRecipe& test_recipe() {
  // Engine stores the recipe by reference, so this owner must outlive every test Engine:
  // https://eel.is/c++draft/class.temporary#6.10
  // https://github.com/FlareZ123/pokemon-sims/issues/907
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return recipe;
}

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
  sim::Engine engine(scenario, test_recipe(), rng);
  set_ready_tapu_state(engine, true);

  // Latias ex makes the Basic Active Tapu Lele-GX's Retreat Cost zero. The free
  // promotion must preserve Fire Energy and the once-per-turn manual attachment:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/905
  assert(sim::EngineTestAccess::resolve_final_promotion_and_attachment(engine));

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
  sim::Engine engine(scenario, test_recipe(), rng);
  set_ready_tapu_state(engine, false);

  // Without Skyliner in play, Tapu Lele-GX still needs its printed one-Colorless
  // Retreat Cost, preserving the confirmed paid fallback from issue #802:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/802
  // https://github.com/FlareZ123/pokemon-sims/issues/905
  assert(sim::EngineTestAccess::resolve_final_promotion_and_attachment(engine));

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
  sim::Engine engine(scenario, test_recipe(), rng);
  set_ready_tapu_state(engine, true);

  // The modeled Rule Box Ability lock suppresses Latias ex's Skyliner. The free
  // route is unavailable, so the same paid Tapu fallback remains legal:
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/905
  assert(sim::EngineTestAccess::resolve_final_promotion_and_attachment(engine));

  const sim::State& state = sim::EngineTestAccess::state(engine);
  assert(state.active.has_value());
  assert(state.active->card == sim::Card::RegidragoVstar);
  assert(state.retreat_used);
  assert(state.manual_energy_used);
  assert(!contains(state.hand, sim::Card::Fire));
  assert(contains(state.discard, sim::Card::Fire));
}

void test_incomplete_vstar_attaches_before_free_retreat() {
  const sim::Scenario scenario{"issue-905-attach-before-free-retreat",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng(90504);
  sim::Engine engine(scenario, test_recipe(), rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None},
      sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None}};
  state.hand = {sim::Card::Fire};
  state.discard = {sim::Card::DialgaGX};
  state.discarded_this_turn = {sim::Card::DialgaGX};

  // Skyliner supplies the later free retreat, while the incomplete VSTAR first
  // needs the held Fire Energy attached to complete Apex Dragon's GGF cost:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/905
  assert(sim::EngineTestAccess::resolve_final_promotion_and_attachment(engine));

  assert(state.active.has_value());
  assert(state.active->card == sim::Card::RegidragoVstar);
  assert(state.active->grass == 2);
  assert(state.active->fire == 1);
  assert(state.manual_energy_used);
  assert(state.retreat_used);
  assert(!contains(state.hand, sim::Card::Fire));
  assert(!contains(state.discard, sim::Card::Fire));
}

}  // namespace

int main() {
  test_free_latias_retreat_precedes_paid_tapu_route();
  test_paid_tapu_route_remains_when_latias_is_absent();
  test_paid_tapu_route_remains_when_skyliner_is_locked();
  test_incomplete_vstar_attaches_before_free_retreat();
  return 0;
}
