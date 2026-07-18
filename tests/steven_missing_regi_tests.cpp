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
  static bool bench_tapu_if_useful(Engine& engine) { return engine.bench_tapu_if_useful(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Scenario scenario() {
  return sim::Scenario{"steven-missing-regi", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
}

sim::State missing_regi_state(const std::vector<sim::Card>& hand) {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = hand;
  state.deck = {sim::Card::RegidragoV, sim::Card::Crispin, sim::Card::BrilliantBlender,
                sim::Card::MegaDragonite, sim::Card::Grass, sim::Card::Fire};
  return state;
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void expect_steven_setup_targets(const sim::State& state) {
  expect(contains(state.hand, sim::Card::RegidragoV),
         "Steven's Resolve should establish the missing Regidrago V axis.");
  expect(contains(state.hand, sim::Card::Crispin),
         "Steven's Resolve should retain the turn-two Energy connector.");
  expect(contains(state.hand, sim::Card::BrilliantBlender),
         "Steven's Resolve should retain the payload outlet.");
  expect(state.turn_ended, "Steven's Resolve must end the turn after its legal three-card search.");
}

void test_direct_steven_fetches_missing_regidrago() {
  // Steven's Resolve searches for up to 3 cards and ends the turn: https://api.pokemontcg.io/v2/cards/sm7-145
  // Regidrago V is a Basic Pokémon that may be selected as one of those cards: https://api.pokemontcg.io/v2/cards/swsh12-135
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{103};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, missing_regi_state({sim::Card::StevensResolve}));

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven's Resolve should be playable when it can establish the missing Regidrago V axis.");
  expect_steven_setup_targets(sim::EngineTestAccess::state(engine));
}

void test_wonder_tag_fetches_steven_for_missing_regidrago() {
  // Wonder Tag can search the deck for a Supporter when Tapu Lele-GX is Benched from hand: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{104};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::State state = missing_regi_state({sim::Card::TapuLeleGX});
  state.deck.push_back(sim::Card::StevensResolve);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::bench_tapu_if_useful(engine),
         "Wonder Tag should be a live connector when Steven's Resolve establishes Regidrago V.");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::StevensResolve),
         "Wonder Tag should fetch Steven's Resolve for the missing Regidrago V setup line.");
  expect(sim::EngineTestAccess::play_steven(engine),
         "The fetched Steven's Resolve should establish the three-card setup line.");
  expect_steven_setup_targets(sim::EngineTestAccess::state(engine));
}

void test_steven_projects_searchable_vstar_after_searching_basic() {
  // Steven's Resolve searches all three cards simultaneously. Once Regidrago V is
  // selected, a remaining deck copy of Regidrago VSTAR is the immediate evolution
  // axis and outranks a payload fallback:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/867
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{8671};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::State state = missing_regi_state({sim::Card::StevensResolve});
  state.deck.push_back(sim::Card::RegidragoVstar);
  state.prizes = {sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should project the evolution axis after selecting Regidrago V.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::RegidragoV),
         "Steven should search the missing Basic prerequisite.");
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "Steven should search a remaining VSTAR copy before a generic payload outlet.");
  expect(contains(after.hand, sim::Card::Crispin),
         "Steven should retain the Energy connector after projecting the evolution axis.");
  expect(!contains(after.hand, sim::Card::BrilliantBlender),
         "The projected evolution axis must outrank a payload fallback.");
}

void test_steven_uses_gladion_when_all_vstars_are_prized() {
  // If legal inspection proves no VSTAR remains in deck, Gladion is the bridge to
  // the revealed Prize card: https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/867
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{8675};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::State state = missing_regi_state({sim::Card::StevensResolve});
  state.deck.push_back(sim::Card::Gladion);
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::RegidragoVstar,
                  sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should preserve a Prize bridge when every VSTAR is absent from deck.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::RegidragoV),
         "Steven should search the missing Basic prerequisite.");
  expect(contains(after.hand, sim::Card::Gladion),
         "Steven should search Gladion when all VSTAR copies are known prized.");
  expect(contains(after.hand, sim::Card::Crispin),
         "Steven should retain the Energy connector beside the Prize bridge.");
}

void test_steven_separates_prized_basic_and_vstar_bridges() {
  // Gladion exchanges for exactly one Prize card. When both the Basic and every
  // VSTAR copy are known prized, Steven must reserve Hisuian Heavy Ball for the
  // Basic and Gladion for the evolution instead of treating one Supporter as two
  // independent recoveries:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/867
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario = scenario();
  std::mt19937_64 rng{8676};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::State state = missing_regi_state({sim::Card::StevensResolve});
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::RegidragoV),
                   state.deck.end());
  state.deck.push_back(sim::Card::HisuianHeavyBall);
  state.deck.push_back(sim::Card::Gladion);
  state.prizes = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                  sim::Card::RegidragoVstar, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should preserve two independent bridges for two prized axes.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::HisuianHeavyBall),
         "Steven should reserve Heavy Ball for the prized Basic Regidrago V.");
  expect(contains(after.hand, sim::Card::Gladion),
         "Steven should reserve Gladion separately for the prized Regidrago VSTAR.");
  expect(contains(after.hand, sim::Card::Crispin),
         "Steven should use the third target on the Energy connector.");
  expect(!contains(after.hand, sim::Card::BrilliantBlender),
         "A generic payload outlet must not displace either required Prize bridge.");
}

sim::State projected_latias_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0},
                 sim::Pokemon{sim::Card::TapuLeleGX, 1}};
  state.hand = {sim::Card::StevensResolve, sim::Card::BrilliantBlender,
                sim::Card::Fire, sim::Card::Dipplin};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin, sim::Card::RegidragoV,
                sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite};
  return state;
}

void test_steven_projects_latias_after_vstar_and_crispin() {
  // Crispin plus the next manual attachment completes GGF on the one-Grass Regidrago.
  // Latias ex then gives the Basic Active Tapu Lele-GX no Retreat Cost, so Latias
  // outranks a redundant Regidrago V as Steven's third target:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/867
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario{"steven-projected-latias", sim::DciProfile::StrictJit,
                                    sim::LockMode::TurnTwoItem, false, 4};
  std::mt19937_64 rng{8672};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, projected_latias_state());

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should remain the live turn-one compression route under scheduled Item lock.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "Steven should search the missing evolution.");
  expect(contains(after.hand, sim::Card::Crispin),
         "Steven should search the Energy connector.");
  expect(contains(after.hand, sim::Card::LatiasEx),
         "Steven should project the future Active-position axis and search Latias ex.");
  expect(!contains(after.hand, sim::Card::RegidragoV),
         "A redundant Regidrago V must not outrank the deterministic Latias promotion route.");
}

void test_steven_does_not_target_locked_latias() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario{"steven-locked-latias", sim::DciProfile::StrictJit,
                                    sim::LockMode::FullCombined, false, 4};
  std::mt19937_64 rng{8673};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, projected_latias_state());

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should remain playable under the combined lock.");
  expect(!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::LatiasEx),
         "Steven must not spend a target on Latias while its Rule Box Ability is suppressed.");
}

void test_steven_preserves_payload_outlet_before_latias() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  const sim::Scenario test_scenario{"steven-payload-before-latias", sim::DciProfile::StrictJit,
                                    sim::LockMode::TurnTwoItem, false, 4};
  std::mt19937_64 rng{8674};
  sim::Engine engine(test_scenario, recipe, rng);
  sim::State state = projected_latias_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), sim::Card::BrilliantBlender),
                   state.hand.end());
  state.vstar_power_used = true;
  state.deck.push_back(sim::Card::ProfessorBurnet);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven should remain playable when it must reserve a payload outlet.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::ProfessorBurnet),
         "Scheduled Item lock requires Steven to reserve Professor Burnet for payload access.");
  expect(!contains(after.hand, sim::Card::LatiasEx),
         "Latias must not displace the only live payload outlet.");
}

}  // namespace

int main() {
  try {
    test_direct_steven_fetches_missing_regidrago();
    test_wonder_tag_fetches_steven_for_missing_regidrago();
    test_steven_projects_searchable_vstar_after_searching_basic();
    test_steven_uses_gladion_when_all_vstars_are_prized();
    test_steven_separates_prized_basic_and_vstar_bridges();
    test_steven_projects_latias_after_vstar_and_crispin();
    test_steven_does_not_target_locked_latias();
    test_steven_preserves_payload_outlet_before_latias();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
