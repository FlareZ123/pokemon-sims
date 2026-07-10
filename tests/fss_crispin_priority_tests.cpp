#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void record_ready(Engine& engine) { engine.record_ready(); }
};

}  // namespace sim

namespace {

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool benched(const sim::State& state, const sim::Card card) {
  return std::any_of(state.bench.begin(), state.bench.end(), [card](const sim::Pokemon& pokemon) {
    return pokemon.card == card;
  });
}

void test_star_alchemy_uses_crispin_for_same_turn_ggf() {
  using namespace sim;
  const Scenario scenario{"fss-crispin-completion", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(156);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 1, 0, Tool::ForestSealStone};
  state.hand = {Card::RegidragoVstar};
  state.deck = {Card::Crispin, Card::EarthenVessel, Card::Grass, Card::Fire};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};

  // Crispin attaches one selected Energy and puts the other into hand, while a player
  // may manually attach one Energy during the turn: https://api.pokemontcg.io/v2/cards/sv7-133
  // https://www.pokemon.com/us/pokemon-tcg/rules
  EngineTestAccess::run_turn(engine);
  EngineTestAccess::record_ready(engine);

  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(state.active->grass == 2 && state.active->fire == 1);
  assert(count(state.discard, Card::Crispin) == 1);
  assert(count(state.discard, Card::EarthenVessel) == 0);
  assert(count(state.deck, Card::EarthenVessel) == 1);
  assert(EngineTestAccess::outcome(engine).first_ready_turn == 2);
}

void test_star_alchemy_keeps_vessel_priority_without_same_turn_crispin_completion() {
  using namespace sim;
  const Scenario scenario{"fss-vessel-future-energy", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(157);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 0, 0, Tool::ForestSealStone};
  state.hand = {Card::RegidragoVstar, Card::Dipplin};
  state.deck = {Card::Crispin, Card::EarthenVessel, Card::Grass, Card::Fire};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};

  // Earthen Vessel puts Basic Energy into hand rather than attaching it, so the
  // three-missing-Energy state cannot be completed by either route this turn: https://api.pokemontcg.io/v2/cards/sv4-163
  EngineTestAccess::run_turn(engine);
  EngineTestAccess::record_ready(engine);

  assert(count(state.discard, Card::EarthenVessel) == 1);
  assert(count(state.deck, Card::Crispin) == 1);
  assert(EngineTestAccess::outcome(engine).first_ready_turn == 0);
}

void test_star_alchemy_fetches_oricorio_when_crispin_blocks_burnet() {
  using namespace sim;
  const Scenario scenario{"fss-oricorio-preserves-burnet", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(187);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoV, 1, 0, 0, Tool::ForestSealStone}};
  state.hand = {Card::ProfessorBurnet};
  state.deck = {Card::Crispin, Card::Oricorio, Card::Fire, Card::MegaDragonite};

  // Star Alchemy may search any card and should take Oricorio when Crispin would
  // consume the Supporter play needed by Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Oricorio's Vital Dance can search up to 2 Basic Energy, so one final Fire is
  // enough for the manual attachment: https://api.pokemontcg.io/v2/cards/sm2-55
  // Burnet then searches and discards the Dragon payload in the same turn:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Only one Supporter and one manual attachment are available each turn:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  EngineTestAccess::run_turn(engine);
  EngineTestAccess::record_ready(engine);

  assert(state.vstar_power_used);
  assert(benched(state, Card::Oricorio));
  assert(count(state.discard, Card::Crispin) == 0);
  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(state.active->grass == 2 && state.active->fire == 1);
  assert(count(state.discard, Card::ProfessorBurnet) == 1);
  assert(count(state.discard, Card::MegaDragonite) == 1);
  assert(EngineTestAccess::outcome(engine).first_ready_turn == 2);
}

void test_arven_fss_fetches_raw_energy_after_supporter_use() {
  using namespace sim;
  const Scenario scenario{"arven-fss-raw-energy", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(206);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.hand = {Card::Arven};
  state.deck = {Card::ForestSealStone, Card::Crispin, Card::Fire};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};

  // Arven uses the turn's Supporter play before Forest Seal Stone resolves:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Star Alchemy may search the missing Fire Energy directly, and the unused manual
  // attachment completes GGF while Crispin remains unplayable this turn:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv7-133
  EngineTestAccess::run_turn(engine);
  EngineTestAccess::record_ready(engine);

  assert(state.supporter_used);
  assert(state.vstar_power_used);
  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(state.active->grass == 2 && state.active->fire == 1);
  assert(count(state.discard, Card::Arven) == 1);
  assert(count(state.hand, Card::Crispin) == 0);
  assert(count(state.deck, Card::Crispin) == 1);
  assert(EngineTestAccess::outcome(engine).first_ready_turn == 2);
}

}  // namespace

int main() {
  test_star_alchemy_uses_crispin_for_same_turn_ggf();
  test_star_alchemy_keeps_vessel_priority_without_same_turn_crispin_completion();
  test_star_alchemy_fetches_oricorio_when_crispin_blocks_burnet();
  test_arven_fss_fetches_raw_energy_after_supporter_use();
  return 0;
}
