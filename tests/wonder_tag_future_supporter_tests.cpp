#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string_view>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool needs_tapu_connector(Engine& engine) {
    return engine.needs_tapu_connector();
  }
  static bool bench_tapu_if_useful(Engine& engine) {
    return engine.bench_tapu_if_useful();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

struct Fixture {
  explicit Fixture(const sim::LockMode locks = sim::LockMode::None)
      : scenario{"wonder-tag-future-supporter", sim::DciProfile::StrictJit,
                 locks, true, 4},
        recipe{sim::baseline_recipe()},
        rng{748},
        engine{scenario, recipe, rng} {}

  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;
};

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State base_turn_one_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::TapuLeleGX};
  return state;
}

void expect_banked(sim::State state, const sim::Card expected,
                   const std::string_view label) {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), true, true);

  if (!sim::EngineTestAccess::needs_tapu_connector(fixture.engine) ||
      !sim::EngineTestAccess::bench_tapu_if_useful(fixture.engine)) {
    throw std::runtime_error(std::string(label) +
                             ": Wonder Tag future route must be selected.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, expected) || after.supporter_used) {
    throw std::runtime_error(std::string(label) +
                             ": Supporter must be banked without being played.");
  }
}

void expect_rejected(sim::State state, const std::string_view label,
                     const sim::LockMode locks = sim::LockMode::None,
                     const bool prizes_revealed = true) {
  Fixture fixture{locks};
  const std::size_t bench_before = state.bench.size();
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), true,
                                   prizes_revealed);

  if (sim::EngineTestAccess::needs_tapu_connector(fixture.engine) ||
      sim::EngineTestAccess::bench_tapu_if_useful(fixture.engine)) {
    throw std::runtime_error(std::string(label) +
                             ": Wonder Tag future route must remain rejected.");
  }
  if (sim::EngineTestAccess::state(fixture.engine).bench.size() != bench_before) {
    throw std::runtime_error(std::string(label) +
                             ": rejection must preserve Bench space.");
  }
}

void test_banks_gladion_for_known_prized_vstar() {
  sim::State state = base_turn_one_state();
  state.deck = {sim::Card::Gladion};
  state.prizes = {sim::Card::RegidragoVstar};

  // Going first prevents playing a Supporter on turn one. Wonder Tag still searches
  // Gladion, and Gladion may remain in hand until turn two to exchange for the known
  // prized Regidrago VSTAR:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/748
  expect_banked(std::move(state), sim::Card::Gladion,
                "known prized VSTAR Gladion route");
}

void test_banks_arven_for_concrete_vstar_chain() {
  sim::State state = base_turn_one_state();
  state.deck = {sim::Card::Arven, sim::Card::RegidragoVstar,
                sim::Card::ForestSealStone};

  // Wonder Tag may bank Arven. On turn two Arven can search Forest Seal Stone,
  // the Tool can attach to the prior-turn Regidrago V, and Star Alchemy can search
  // Regidrago VSTAR:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/748
  expect_banked(std::move(state), sim::Card::Arven,
                "concrete Arven VSTAR route");
}

void test_absent_future_target_is_rejected() {
  sim::State state = base_turn_one_state();
  state.deck = {sim::Card::Serena};

  // Wonder Tag is optional and a Bench slot must not be spent when K1 proves that
  // neither the Gladion nor Arven future connector remains in deck:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/748
  expect_rejected(std::move(state), "absent future target");
}

void test_direct_vstar_connector_is_preferred() {
  sim::State state = base_turn_one_state();
  state.hand.push_back(sim::Card::EvolutionIncense);
  state.deck = {sim::Card::Arven, sim::Card::RegidragoVstar,
                sim::Card::ForestSealStone};

  // Evolution Incense already searches the Evolution Pokémon directly, so Tapu and
  // a future Supporter should not consume an additional Bench slot and connector:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/748
  expect_rejected(std::move(state), "direct Evolution Incense route");
}

void test_full_bench_is_rejected() {
  sim::State state = base_turn_one_state();
  state.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 0, 0, 0,
                                     sim::Tool::None});
  state.bench.push_back(sim::Pokemon{sim::Card::MawileGX, 0, 0, 0,
                                     sim::Tool::None});
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                                     sim::Tool::None});
  state.bench.push_back(sim::Pokemon{sim::Card::LatiasEx, 0, 0, 0,
                                     sim::Tool::None});
  state.deck = {sim::Card::Gladion};
  state.prizes = {sim::Card::RegidragoVstar};

  // A player cannot Bench a sixth Pokémon. Future value never overrides the Bench
  // limit:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://github.com/FlareZ123/pokemon-sims/issues/748
  expect_rejected(std::move(state), "full Bench");
}

void test_rule_box_ability_lock_is_rejected() {
  sim::State state = base_turn_one_state();
  state.deck = {sim::Card::Gladion};
  state.prizes = {sim::Card::RegidragoVstar};

  // Wonder Tag is an Ability on a Rule Box Pokémon and remains unavailable under
  // the modeled full Rule Box Ability lock:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-scenarios
  // https://github.com/FlareZ123/pokemon-sims/issues/748
  expect_rejected(std::move(state), "Rule Box Ability lock",
                  sim::LockMode::FullRuleBoxAbility);
}

}  // namespace

int main() {
  test_banks_gladion_for_known_prized_vstar();
  test_banks_arven_for_concrete_vstar_chain();
  test_absent_future_target_is_rejected();
  test_direct_vstar_connector_is_preferred();
  test_full_bench_is_rejected();
  test_rule_box_ability_lock_is_rejected();
  return 0;
}
