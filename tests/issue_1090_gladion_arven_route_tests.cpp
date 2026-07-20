#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

// Confirmed issue and implementation PR:
// https://github.com/FlareZ123/pokemon-sims/issues/1090
// https://github.com/FlareZ123/pokemon-sims/pull/1095

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void set_known_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

const sim::DeckRecipe& recipe() {
  static const sim::DeckRecipe value = sim::baseline_recipe();
  return value;
}

sim::State missing_basic_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.hand = {sim::Card::Arven, sim::Card::Serena, sim::Card::Grass,
                sim::Card::RegidragoVstar, sim::Card::MysteriousTreasure,
                sim::Card::EarthenVessel, sim::Card::Gladion};
  state.deck = {sim::Card::QuickBall, sim::Card::ForestSealStone,
                sim::Card::BrilliantBlender, sim::Card::Fire};
  state.prizes = {sim::Card::Dragapult, sim::Card::Crispin,
                  sim::Card::MysteriousTreasure, sim::Card::RegidragoV,
                  sim::Card::RegidragoV, sim::Card::Channeler};
  return state;
}

void unpayable_arven_does_not_suppress_blind_gladion() {
  const sim::Scenario scenario{"issue-1090-unpayable-arven", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{109001};
  sim::Engine engine(scenario, recipe(), rng);
  sim::EngineTestAccess::set_state(engine, missing_basic_state());

  // Arven may search an Item, but Quick Ball and Mysterious Treasure still require
  // a distinct discard cost. The strict DCI hand has no admissible cost, so Gladion
  // must remain the live Prize-inspection route for the missing Regidrago V:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/1090
  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Unpayable Arven incorrectly suppressed blind Gladion.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::RegidragoV) ||
      !contains(after.hand, sim::Card::Arven) ||
      !contains(after.prizes, sim::Card::Gladion)) {
    throw std::runtime_error("Gladion failed to recover Regidrago V while preserving Arven.");
  }
}

void payable_one_discard_arven_route_preserves_gladion() {
  const sim::Scenario scenario{"issue-1090-payable-arven-discard", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{109002};
  sim::Engine engine(scenario, recipe(), rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::Gladion, sim::Card::Gladion, sim::Card::Arven};
  state.deck = {sim::Card::QuickBall, sim::Card::MysteriousTreasure,
                sim::Card::RegidragoV};
  state.prizes = {sim::Card::Grass, sim::Card::Fire, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // After Arven leaves hand, the duplicate Gladion is a strict-DCI legal cost for
  // the searched one-discard Item, so the ordinary Basic route remains live:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/1090
  if (sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion displaced a payable Arven-to-Basic route.");
  }
}

void payable_communication_arven_route_preserves_gladion() {
  const sim::Scenario scenario{"issue-1090-payable-arven-communication", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{109003};
  sim::Engine engine(scenario, recipe(), rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::Gladion, sim::Card::Arven, sim::Card::Dipplin};
  state.deck = {sim::Card::PokemonCommunication, sim::Card::RegidragoV};
  state.prizes = {sim::Card::Grass, sim::Card::Fire, sim::Card::Channeler};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Arven can search Pokémon Communication, return the dead Dipplin, and obtain the
  // missing Basic after the Supporter slot is spent. Gladion must therefore be held:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://github.com/FlareZ123/pokemon-sims/issues/1090
  if (sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion displaced the payable Arven-to-Communication route.");
  }
}

void known_prize_heavy_ball_arven_route_preserves_gladion() {
  const sim::Scenario scenario{"issue-1090-payable-arven-heavy-ball", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{109004};
  sim::Engine engine(scenario, recipe(), rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::Gladion, sim::Card::Arven};
  state.deck = {sim::Card::HisuianHeavyBall};
  state.prizes = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_known_state(engine, std::move(state));

  // K1 proves Regidrago V is prized. Arven may search Hisuian Heavy Ball, whose
  // legal Prize exchange resolves the same missing Basic axis while retaining Gladion:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://github.com/FlareZ123/pokemon-sims/issues/1090
  if (sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion displaced the known-Prize Arven-to-Heavy-Ball route.");
  }
}

void item_lock_keeps_arven_from_suppressing_gladion() {
  const sim::Scenario scenario{"issue-1090-item-lock-control", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 5};
  std::mt19937_64 rng{109005};
  sim::Engine engine(scenario, recipe(), rng);
  sim::State state = missing_basic_state();
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Arven remains playable as a Supporter, but Item lock prevents the searched Item
  // from resolving, so it cannot suppress the blind Gladion recovery action:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://github.com/FlareZ123/pokemon-sims/issues/1090
  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Item-locked Arven incorrectly suppressed Gladion.");
  }
}

}  // namespace

int main() {
  unpayable_arven_does_not_suppress_blind_gladion();
  payable_one_discard_arven_route_preserves_gladion();
  payable_communication_arven_route_preserves_gladion();
  known_prize_heavy_ball_arven_route_preserves_gladion();
  item_lock_keeps_arven_from_suppressing_gladion();
  return 0;
}
