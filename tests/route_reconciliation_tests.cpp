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
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) { return engine.play_mysterious_treasure(permit_payload); }
  static bool play_quick_ball(Engine& engine, const bool permit_payload) { return engine.play_quick_ball(permit_payload); }
  static bool play_ultra_ball(Engine& engine, const bool permit_payload) { return engine.play_ultra_ball(permit_payload); }
  static bool play_evolution_incense(Engine& engine, const bool permit_payload) { return engine.play_evolution_incense(permit_payload); }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool play_turo_active_promotion_route(Engine& engine) {
    return engine.play_turo_active_promotion_route();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_ultra_ball_requires_two_distinct_costs() {
  // Ultra Ball requires discarding 2 other cards from hand: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  const sim::Scenario scenario{"ultra-ball-distinct-cost", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{54};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.hand = {sim::Card::UltraBall, sim::Card::Dipplin};
  state.deck = {sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_ultra_ball(engine, false),
         "Ultra Ball must not use the same singleton card twice for its two discard costs.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::UltraBall) && contains(after.hand, sim::Card::Dipplin) &&
             after.discard.empty() && contains(after.deck, sim::Card::RegidragoV),
         "An unpayable Ultra Ball must leave the exact state unchanged.");
}

void test_evolution_incense_hands_payload_to_payable_ultra_ball() {
  // Evolution Incense may find an Evolution Pokémon, and Ultra Ball may then discard
  // that payload plus a distinct card: https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  const sim::Scenario scenario{"evolution-incense-ultra-ball", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{112};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::UltraBall, sim::Card::Dipplin};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_evolution_incense(engine, true),
         "Evolution Incense should fetch the current-turn payload for a payable Ultra Ball.");
  expect(sim::EngineTestAccess::play_ultra_ball(engine, true),
         "Ultra Ball should discard the fetched payload with Dipplin as its other cost.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.discard, sim::Card::MegaDragonite) &&
             contains(after.discarded_this_turn, sim::Card::MegaDragonite),
         "The Evolution Incense route must leave the Dragon payload in this turn's discard.");
}

void test_two_ultra_ball_payload_line_requires_second_cost() {
  // Each Ultra Ball independently needs 2 other cards from hand: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  const sim::Scenario scenario{"two-ultra-ball-capacity", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{115};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.bench = {{sim::Card::RegidragoV, 1}, {sim::Card::RegidragoV, 1},
                 {sim::Card::MawileGX, 1}, {sim::Card::LatiasEx, 1}, {sim::Card::Oricorio, 1}};
  state.hand = {sim::Card::UltraBall, sim::Card::UltraBall, sim::Card::Dipplin, sim::Card::RegidragoV};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
  state.discard = {sim::Card::ProfessorBurnet};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_ultra_ball(engine, true),
         "The first Ultra Ball must be held when a second Ultra Ball would lack a distinct second cost.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(std::count(after.hand.begin(), after.hand.end(), sim::Card::UltraBall) == 2 &&
             contains(after.hand, sim::Card::Dipplin) && contains(after.hand, sim::Card::RegidragoV) &&
             contains(after.deck, sim::Card::MegaDragonite),
         "Rejecting the unpayable two-Ultra-Ball continuation must preserve state.");
}

sim::State prized_vstar_state(std::vector<sim::Card> hand) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = std::move(hand);
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  return state;
}

void test_gladion_uses_prized_vstar_only_when_item_cost_is_unpayable() {
  // Mysterious Treasure needs one discard and Ultra Ball needs two other card costs;
  // Gladion exchanges itself for a Prize card: https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146 https://api.pokemontcg.io/v2/cards/sm4-95
  const sim::Scenario scenario{"gladion-vstar-cost", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();

  std::mt19937_64 unpayable_rng{116};
  sim::Engine unpayable(scenario, recipe, unpayable_rng);
  sim::EngineTestAccess::set_state(unpayable, prized_vstar_state({sim::Card::Gladion, sim::Card::MysteriousTreasure}));
  expect(sim::EngineTestAccess::play_gladion(unpayable) &&
             contains(sim::EngineTestAccess::state(unpayable).hand, sim::Card::RegidragoVstar),
         "Gladion must recover the prized VSTAR when Mysterious Treasure cannot pay its cost.");

  std::mt19937_64 payable_rng{117};
  sim::Engine payable(scenario, recipe, payable_rng);
  sim::EngineTestAccess::set_state(payable, prized_vstar_state({sim::Card::Gladion, sim::Card::MysteriousTreasure, sim::Card::Dipplin}));
  expect(!sim::EngineTestAccess::play_gladion(payable) &&
             contains(sim::EngineTestAccess::state(payable).hand, sim::Card::Gladion),
         "Gladion must remain available when Mysterious Treasure has a legal discard cost.");
}

void test_gladion_is_held_for_arven_forest_seal_stone_under_item_lock() {
  // Arven is a Supporter and can fetch Forest Seal Stone through Item lock; the Tool
  // then provides a VSTAR Power route rather than consuming Gladion: https://api.pokemontcg.io/v2/cards/sv1-166
  // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  const sim::Scenario scenario{"arven-fss-holds-gladion", sim::DciProfile::StrictJit, sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{101};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::Arven};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.deck = {sim::Card::ForestSealStone, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::play_gladion(engine),
         "Gladion should be held while Arven can retrieve a live Forest Seal Stone VSTAR route.");
  expect(sim::EngineTestAccess::play_arven(engine) &&
             contains(sim::EngineTestAccess::state(engine).hand, sim::Card::ForestSealStone),
         "Arven should retrieve Forest Seal Stone through Item lock.");
}

void test_mysterious_treasure_holds_false_single_ultra_ball_payload_line() {
  // Mysterious Treasure pays its discard before it searches, and Ultra Ball must
  // later discard the fetched payload plus another card from hand:
  // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  const sim::Scenario scenario{"mysterious-treasure-single-ultra-false-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{164};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::UltraBall, sim::Card::Dipplin, sim::Card::Gladion};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  expect(!sim::EngineTestAccess::play_mysterious_treasure(engine, true),
         "Mysterious Treasure must be held when its only prospective Ultra Ball continuation lacks a second cost.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::MysteriousTreasure) && contains(after.hand, sim::Card::UltraBall) &&
             contains(after.hand, sim::Card::Dipplin) && contains(after.hand, sim::Card::Gladion) &&
             after.discard.empty() && contains(after.deck, sim::Card::MegaDragonite),
         "Rejecting the false Mysterious Treasure continuation must preserve the exact state.");
}

void test_quick_ball_holds_false_single_ultra_ball_payload_line() {
  // Quick Ball pays its discard before it searches, and Ultra Ball must later
  // discard the fetched payload plus another card from hand:
  // https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  const sim::Scenario scenario{"quick-ball-single-ultra-false-route", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{165};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::UltraBall, sim::Card::Dipplin, sim::Card::Gladion};
  state.deck = {sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  expect(!sim::EngineTestAccess::play_quick_ball(engine, true),
         "Quick Ball must be held when its only prospective Ultra Ball continuation lacks a second cost.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::QuickBall) && contains(after.hand, sim::Card::UltraBall) &&
             contains(after.hand, sim::Card::Dipplin) && contains(after.hand, sim::Card::Gladion) &&
             after.discard.empty() && contains(after.deck, sim::Card::DialgaGX),
         "Rejecting the false Quick Ball continuation must preserve the exact state.");
}

sim::State energy_target_state(const bool active_can_evolve, const bool with_latias,
                               const bool with_tate, const bool payload_is_ready) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, active_can_evolve ? 1 : 2,
                              1, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 0, sim::Tool::None}};
  if (with_latias) state.bench.push_back(sim::Pokemon{sim::Card::LatiasEx, 1});
  state.hand = {sim::Card::Grass};
  if (active_can_evolve) state.hand.push_back(sim::Card::RegidragoVstar);
  if (with_tate) state.hand.push_back(sim::Card::TateLiza);
  if (payload_is_ready) {
    state.discard = {sim::Card::MegaDragonite};
    state.discarded_this_turn = {sim::Card::MegaDragonite};
  }
  return state;
}

void test_latias_route_powers_existing_benched_vstar() {
  const sim::Scenario scenario{"latias-benched-vstar-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{215};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, energy_target_state(false, true, false, true));

  // Latias ex gives the Basic Active no Retreat Cost, so the Benched Regidrago
  // VSTAR is the reachable attacker and should receive the manual Energy:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://www.pokemon.com/us/pokemon-tcg/rules
  expect(sim::EngineTestAccess::attach_manual(engine),
         "The Latias route should permit a manual attachment to the Benched VSTAR.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active->grass == 1 && after.bench.front().grass == 1,
         "The Energy should move to the reachable Benched VSTAR instead of the stranded Active Basic.");
}

void test_tate_route_powers_existing_benched_vstar_when_payload_ready() {
  const sim::Scenario scenario{"tate-benched-vstar-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{216};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, energy_target_state(false, false, true, true));

  // Tate & Liza can switch the powered Benched VSTAR Active. The payload is already
  // ready, so using the Supporter does not displace Professor Burnet:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  expect(sim::EngineTestAccess::attach_manual(engine),
         "The Tate switch route should permit a manual attachment to the Benched VSTAR.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active->grass == 1 && after.bench.front().grass == 1,
         "The Energy should target the Benched VSTAR reachable through Tate & Liza.");
}

void test_stranded_benched_vstar_does_not_steal_energy() {
  const sim::Scenario scenario{"stranded-benched-vstar-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{217};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, energy_target_state(false, false, false, true));

  expect(sim::EngineTestAccess::attach_manual(engine),
         "The Active Regidrago line should remain the best reachable target.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active->grass == 2 && after.bench.front().grass == 0,
         "A stranded Benched VSTAR must not receive the route bonus.");
}

void test_active_evolution_route_keeps_energy_on_active_regidrago() {
  const sim::Scenario scenario{"active-evolution-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{218};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, energy_target_state(true, true, false, true));

  // A prior-turn Active Regidrago V with Regidrago VSTAR in hand can legally evolve
  // this turn, so its direct evolution route remains preferred:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::attach_manual(engine),
         "The legal Active evolution route should receive the manual Energy.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active->grass == 2 && after.bench.front().grass == 0,
         "Latias must not redirect Energy away from an immediately evolvable Active Regidrago V.");
}

sim::State turo_regidrago_active_state(const int active_entered_turn) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, active_entered_turn, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  // The shared state includes Regidrago VSTAR so the same-turn case still cannot
  // evolve, while the prior-turn case has the direct evolution route required by
  // the control's own rule claim:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  state.hand = {sim::Card::ProfessorTuro, sim::Card::RegidragoVstar};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  return state;
}

void test_turo_promotes_complete_vstar_behind_same_turn_active_regidrago() {
  const sim::Scenario scenario{"turo-same-turn-active-regidrago", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{392};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, turo_regidrago_active_state(2));

  // A Pokémon cannot evolve during the turn it entered play. Professor Turo's
  // Scenario may return that Active Regidrago V, discard its attachments, and force
  // promotion of the complete Benched Regidrago VSTAR:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sv4-171
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::play_turo_active_promotion_route(engine),
         "Professor Turo should remove a same-turn Active Regidrago V and promote the ready Benched VSTAR.");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
             after.active->grass == 2 && after.active->fire == 1,
         "Professor Turo must promote the complete Benched Regidrago VSTAR.");
  expect(after.bench.empty() && contains(after.hand, sim::Card::RegidragoV) &&
             contains(after.discard, sim::Card::ProfessorTuro),
         "The same-turn Active Regidrago V must return to hand and Professor Turo must enter discard.");
  expect(after.supporter_used && !after.retreat_used,
         "The route must consume the Supporter play without consuming the retreat.");
}

void test_turo_preserves_prior_turn_active_regidrago_evolution_route() {
  const sim::Scenario scenario{"turo-prior-turn-active-regidrago", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{393};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, turo_regidrago_active_state(1));

  // A prior-turn Active Regidrago V is legally evolvable this turn, so the existing
  // direct evolution preference remains intact:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(!sim::EngineTestAccess::play_turo_active_promotion_route(engine),
         "Professor Turo should remain held when the Active Regidrago V can legally evolve this turn.");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::RegidragoV &&
             after.bench.size() == 1 && contains(after.hand, sim::Card::ProfessorTuro) &&
             contains(after.hand, sim::Card::RegidragoVstar) && !after.supporter_used,
         "Rejecting the Turo route must preserve the prior-turn Active evolution state.");
}

}  // namespace

int main() {
  try {
    test_ultra_ball_requires_two_distinct_costs();
    test_evolution_incense_hands_payload_to_payable_ultra_ball();
    test_two_ultra_ball_payload_line_requires_second_cost();
    test_gladion_uses_prized_vstar_only_when_item_cost_is_unpayable();
    test_gladion_is_held_for_arven_forest_seal_stone_under_item_lock();
    test_mysterious_treasure_holds_false_single_ultra_ball_payload_line();
    test_quick_ball_holds_false_single_ultra_ball_payload_line();
    test_latias_route_powers_existing_benched_vstar();
    test_tate_route_powers_existing_benched_vstar_when_payload_ready();
    test_stranded_benched_vstar_does_not_steal_energy();
    test_active_evolution_route_keeps_energy_on_active_regidrago();
    test_turo_promotes_complete_vstar_behind_same_turn_active_regidrago();
    test_turo_preserves_prior_turn_active_regidrago_evolution_route();
    std::cout << "route reconciliation tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
