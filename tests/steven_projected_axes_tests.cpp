#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_steven(Engine& engine) { return engine.play_steven(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{"issue-867-steven-projected-axes",
                       sim::DciProfile::StrictJit, locks, false, 4};
}

sim::State missing_regi_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::StevensResolve};
  state.deck = {sim::Card::RegidragoV, sim::Card::Crispin,
                sim::Card::BrilliantBlender, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

void test_projects_searchable_vstar_after_basic() {
  // Steven searches up to three unrestricted cards at once. Regidrago V exposes
  // Regidrago VSTAR as the immediate evolution axis:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/867
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{86701};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::State state = missing_regi_state();
  state.deck.push_back(sim::Card::RegidragoVstar);
  state.prizes = {sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should project the VSTAR axis after selecting Regidrago V.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::RegidragoV),
         "Steven should search the missing Basic.");
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "A searchable VSTAR should outrank the generic payload outlet.");
  expect(contains(after.hand, sim::Card::Crispin),
         "Steven should retain the Energy connector.");
  expect(!contains(after.hand, sim::Card::BrilliantBlender),
         "The projected evolution axis should use the payload fallback slot.");
}

void test_uses_gladion_only_when_vstars_are_absent() {
  // Gladion exchanges for one Prize card. It is the evolution bridge only after
  // inspection proves that every Regidrago VSTAR copy is absent from the deck:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/867
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{86702};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::State state = missing_regi_state();
  state.deck.push_back(sim::Card::Gladion);
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::RegidragoVstar,
                  sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should preserve the known Prize bridge.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::RegidragoV),
         "Steven should search the Basic prerequisite.");
  expect(contains(after.hand, sim::Card::Gladion),
         "Steven should search Gladion when every VSTAR is known prized.");
  expect(contains(after.hand, sim::Card::Crispin),
         "Steven should retain the Energy connector beside Gladion.");
}

sim::State both_axes_prized_state() {
  sim::State state = missing_regi_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::RegidragoV),
                   state.deck.end());
  state.deck.push_back(sim::Card::HisuianHeavyBall);
  state.deck.push_back(sim::Card::Gladion);
  state.prizes = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                  sim::Card::RegidragoVstar, sim::Card::RegidragoVstar};
  return state;
}

void test_separates_two_prize_bridges_without_lock() {
  // Hisuian Heavy Ball can recover the prized Basic while Gladion exchanges for
  // exactly one separate Prize card:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/867
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{86703};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, both_axes_prized_state());

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should preserve two legal Prize bridges.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::HisuianHeavyBall),
         "Heavy Ball should bridge the prized Basic when Items remain legal.");
  expect(contains(after.hand, sim::Card::Gladion),
         "Gladion should bridge the prized evolution separately.");
  expect(contains(after.hand, sim::Card::Crispin),
         "The third target should preserve the Energy axis.");
}

void test_rejects_future_heavy_ball_under_turn_two_lock() {
  // Steven ends turn one after searching. Hisuian Heavy Ball is an Item, and the
  // scheduled lock makes it unusable from turn two onward:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L143-L148
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/867
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario(sim::LockMode::TurnTwoItem);
  std::mt19937_64 rng{86704};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, both_axes_prized_state());

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should still resolve its available legal targets.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(!contains(after.hand, sim::Card::HisuianHeavyBall),
         "Steven must not reserve a Heavy Ball that will be locked next turn.");
  expect(contains(after.hand, sim::Card::Gladion),
         "One Gladion may bridge one prized axis.");
  expect(count(after.hand, sim::Card::Gladion) == 1,
         "One Gladion must not be counted as two independent Prize bridges.");
  expect(!contains(after.hand, sim::Card::RegidragoVstar),
         "The unavailable second Prize bridge must leave the VSTAR axis unresolved.");
}

sim::State projected_latias_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0},
                 sim::Pokemon{sim::Card::TapuLeleGX, 1}};
  state.hand = {sim::Card::StevensResolve, sim::Card::BrilliantBlender,
                sim::Card::Fire, sim::Card::Dipplin};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::RegidragoV, sim::Card::LatiasEx,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite};
  return state;
}

void test_projects_latias_after_vstar_and_crispin() {
  // VSTAR plus Crispin and the next manual attachment complete GGF. Latias ex then
  // gives the Basic Active no Retreat Cost and completes the promotion axis:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/867
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario(sim::LockMode::TurnTwoItem);
  std::mt19937_64 rng{86705};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, projected_latias_state());

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should use the turn-one compression route.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "Steven should search the evolution.");
  expect(contains(after.hand, sim::Card::Crispin),
         "Steven should search the Energy connector.");
  expect(contains(after.hand, sim::Card::LatiasEx),
         "Steven should project the future promotion axis.");
  expect(!contains(after.hand, sim::Card::RegidragoV),
         "A redundant Basic must not displace Latias.");
}

void test_rejects_latias_when_skyliner_is_locked() {
  // The combined lock suppresses Latias ex's Rule Box Ability:
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/867
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario(sim::LockMode::FullCombined);
  std::mt19937_64 rng{86706};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, projected_latias_state());

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should remain playable under the combined lock.");
  expect(!contains(sim::EngineTestAccess::state(engine).hand,
                   sim::Card::LatiasEx),
         "Steven must not target a suppressed Skyliner route.");
}

void test_preserves_payload_outlet_before_latias() {
  // With Legacy Star spent and turn-two Item lock scheduled, Professor Burnet is the
  // surviving deck-to-discard outlet and must outrank Latias:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/issues/867
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario(sim::LockMode::TurnTwoItem);
  std::mt19937_64 rng{86707};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::State state = projected_latias_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::BrilliantBlender),
                   state.hand.end());
  state.vstar_power_used = true;
  state.deck.push_back(sim::Card::ProfessorBurnet);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should reserve the surviving payload outlet.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::ProfessorBurnet),
         "Steven should reserve Burnet through scheduled Item lock.");
  expect(!contains(after.hand, sim::Card::LatiasEx),
         "Latias must not displace the only live payload outlet.");
}

}  // namespace

int main() {
  try {
    test_projects_searchable_vstar_after_basic();
    test_uses_gladion_only_when_vstars_are_absent();
    test_separates_two_prize_bridges_without_lock();
    test_rejects_future_heavy_ball_under_turn_two_lock();
    test_projects_latias_after_vstar_and_crispin();
    test_rejects_latias_when_skyliner_is_locked();
    test_preserves_payload_outlet_before_latias();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
