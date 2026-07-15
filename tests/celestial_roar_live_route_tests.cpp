#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_professor_burnet(Engine& engine) {
    return engine.play_professor_burnet();
  }
  static bool use_celestial_roar(Engine& engine) { return engine.use_celestial_roar(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_strict_jit_holds_when_known_deck_has_no_needed_energy() {
  const sim::Scenario scenario{"celestial-roar-dead-strict", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3101};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.deck = {sim::Card::Grass, sim::Card::MegaDragonite, sim::Card::Arven};
  sim::EngineTestAccess::set_deck_seen(engine);

  // Celestial Roar attaches only Energy among the discarded top 3. K1 proves that
  // no Fire Energy remains, and strict JIT cannot carry this turn's payload discard
  // into a later ready turn:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  assert(!sim::EngineTestAccess::use_celestial_roar(engine));
  assert(!state.turn_ended);
  assert(state.deck.size() == 3U);
  assert(!contains(state.discard, sim::Card::MegaDragonite));
  assert(state.active->fire == 0);
}

void test_no_control_keeps_live_payload_banking_attack() {
  const sim::Scenario scenario{"celestial-roar-payload-bank", sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3102};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.deck = {sim::Card::Grass, sim::Card::MegaDragonite, sim::Card::Arven};
  sim::EngineTestAccess::set_deck_seen(engine);

  // No-discard-control may bank a Dragon payload before the eventual Apex Dragon
  // turn, so the known payload in Celestial Roar's possible top-three set remains a
  // live modeled route even though no needed Fire Energy remains:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(sim::EngineTestAccess::use_celestial_roar(engine));
  assert(state.turn_ended);
  assert(contains(state.discard, sim::Card::MegaDragonite));
}

void test_no_control_banks_payload_with_full_ggf_and_known_prized_vstar() {
  const sim::Scenario scenario{"celestial-roar-full-ggf-payload-bank",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{6081};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.supporter_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Gladion};
  state.prizes = {sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Arven, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_deck_seen(engine);

  // Celestial Roar remains a legal attack after GGF is complete and may bank a
  // Dragon payload for a later Apex Dragon turn. The known prized VSTAR plus held
  // Gladion preserves the next-turn evolution route without exposing the VSTAR to
  // the top-three discard:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  assert(sim::EngineTestAccess::use_celestial_roar(engine));
  assert(state.turn_ended);
  assert(contains(state.discard, sim::Card::MegaDragonite));
  assert(contains(state.prizes, sim::Card::RegidragoVstar));
  assert(contains(state.hand, sim::Card::Gladion));
}

void test_strict_jit_holds_celestial_roar_with_full_ggf() {
  const sim::Scenario scenario{"celestial-roar-full-ggf-strict-hold",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{6082};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.deck = {sim::Card::Arven, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::Fire};
  const std::vector<sim::Card> original_deck = state.deck;
  sim::EngineTestAccess::set_deck_seen(engine);

  // Strict JIT requires the payload to enter discard during the eventual ready
  // turn, so a full-GGF unevolved Regidrago V must preserve the deck here:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  assert(!sim::EngineTestAccess::use_celestial_roar(engine));
  assert(!state.turn_ended);
  assert(state.deck == original_deck);
  assert(state.discard.empty());
}

void test_no_control_holds_full_ggf_when_known_deck_has_no_payload() {
  const sim::Scenario scenario{"celestial-roar-full-ggf-no-payload-hold",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{6083};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.deck = {sim::Card::Arven, sim::Card::Grass, sim::Card::Fire};
  const std::vector<sim::Card> original_deck = state.deck;
  sim::EngineTestAccess::set_deck_seen(engine);

  // K1 proves that neither a needed Energy nor a modeled Dragon payload can be
  // advanced, so the optional top-three discard remains correctly held:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  assert(!sim::EngineTestAccess::use_celestial_roar(engine));
  assert(!state.turn_ended);
  assert(state.deck == original_deck);
  assert(state.discard.empty());
}

void test_burnet_discards_two_payloads_before_dipplin() {
  const sim::Scenario scenario{"burnet-two-payloads",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{353};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Dipplin, sim::Card::Arven};

  // Burnet may discard two selected deck cards. Both modeled A/S Dragons add an
  // Apex Dragon attack, so they take priority over Dipplin:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv6-130
  assert(sim::EngineTestAccess::play_professor_burnet(engine));
  assert(contains(state.discard, sim::Card::MegaDragonite));
  assert(contains(state.discard, sim::Card::Dragapult));
  assert(!contains(state.discard, sim::Card::Dipplin));
  assert(contains(state.deck, sim::Card::Dipplin));
}

void test_burnet_uses_second_safe_selection_for_celestial_roar() {
  const sim::Scenario scenario{"burnet-celestial-roar-thinning",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{340};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::MegaDragonite, sim::Card::PathToPeak,
                sim::Card::Fire, sim::Card::Arven, sim::Card::LatiasEx};

  // Burnet may discard two selected deck cards. Removing the no-lock setup-dead
  // Stadium with the payload leaves exactly three cards, so Celestial Roar processes
  // the remaining Fire Energy with certainty:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  assert(sim::EngineTestAccess::play_professor_burnet(engine));
  assert(state.deck.size() == 3U);
  assert(contains(state.discard, sim::Card::MegaDragonite));
  assert(contains(state.discard, sim::Card::PathToPeak));

  assert(sim::EngineTestAccess::use_celestial_roar(engine));
  assert(state.turn_ended);
  assert(state.deck.empty());
  assert(state.active->fire == 1);
}

void test_strict_jit_attacks_when_needed_fire_may_remain() {
  const sim::Scenario scenario{"celestial-roar-live-fire", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{3103};
  sim::Engine engine(scenario, recipe, rng);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.deck = {sim::Card::Arven, sim::Card::MegaDragonite, sim::Card::Fire};
  sim::EngineTestAccess::set_deck_seen(engine);

  // A known remaining Fire Energy can still be attached by Celestial Roar:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  assert(sim::EngineTestAccess::use_celestial_roar(engine));
  assert(state.turn_ended);
  assert(state.active->fire == 1);
}

}  // namespace

int main() {
  test_strict_jit_holds_when_known_deck_has_no_needed_energy();
  test_no_control_keeps_live_payload_banking_attack();
  test_no_control_banks_payload_with_full_ggf_and_known_prized_vstar();
  test_strict_jit_holds_celestial_roar_with_full_ggf();
  test_no_control_holds_full_ggf_when_known_deck_has_no_payload();
  test_burnet_discards_two_payloads_before_dipplin();
  test_burnet_uses_second_safe_selection_for_celestial_roar();
  test_strict_jit_attacks_when_needed_fire_may_remain();
  return 0;
}
