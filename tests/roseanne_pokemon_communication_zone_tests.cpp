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
  static bool play_recovery(Engine& engine) {
    return engine.play_roseanne_vstar_energy_recovery();
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};
}  // namespace sim

namespace {

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::RoseannesBackup, sim::Card::PokemonCommunication,
                sim::Card::Dipplin, sim::Card::EarthenVessel,
                sim::Card::Arven, sim::Card::Arven};
  state.deck = {sim::Card::Grass};
  state.discard = {sim::Card::RegidragoVstar, sim::Card::RegidragoVstar,
                   sim::Card::RegidragoVstar, sim::Card::Fire,
                   sim::Card::Fire, sim::Card::Fire,
                   sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.vstar_power_used = true;
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const char* label, const sim::LockMode locks, const std::uint64_t seed)
      : scenario{label, sim::DciProfile::StrictJit, locks, false, 4},
        rng(seed),
        engine(scenario, recipe, rng) {}

  Fixture(const Fixture&) = delete;
  Fixture& operator=(const Fixture&) = delete;
};

void assert_exact_recovery(const sim::State& state) {
  assert(state.supporter_used);
  assert(contains(state.discard, sim::Card::RoseannesBackup));
  assert(count(state.discard, sim::Card::RegidragoVstar) == 2);
  assert(count(state.discard, sim::Card::Fire) == 2);
  assert(count(state.deck, sim::Card::RegidragoVstar) == 1);
  assert(count(state.deck, sim::Card::Fire) == 1);
}

void k0_moves_restored_cards_between_zones() {
  Fixture fixture{"issue-837-k0", sim::LockMode::None, 83701};
  sim::EngineTestAccess::state(fixture.engine) = route_state();

  // All fixed-list VSTAR and Fire copies are public at K0. Roseanne's Backup moves
  // one of each into the deck, Pokémon Communication exchanges Dipplin for the
  // restored VSTAR, and Earthen Vessel retains a separate Arven cost:
  // https://api.pokemontcg.io/v2/cards/swsh9-148
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L86-L104
  // https://github.com/FlareZ123/pokemon-sims/issues/837
  assert(sim::EngineTestAccess::play_recovery(fixture.engine));
  assert_exact_recovery(sim::EngineTestAccess::state(fixture.engine));
}

void k1_preserves_the_exact_route() {
  Fixture fixture{"issue-837-k1", sim::LockMode::None, 83702};
  sim::EngineTestAccess::state(fixture.engine) = route_state();
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  assert(sim::EngineTestAccess::play_recovery(fixture.engine));
  assert_exact_recovery(sim::EngineTestAccess::state(fixture.engine));
}

void full_turn_evolves_and_attaches_restored_fire() {
  Fixture fixture{"issue-837-full-turn", sim::LockMode::None, 83703};
  sim::EngineTestAccess::state(fixture.engine) = route_state();
  sim::EngineTestAccess::run_turn(fixture.engine);

  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  assert(state.active.has_value());
  assert(state.active->card == sim::Card::RegidragoVstar);
  assert(state.active->grass == 2);
  assert(state.active->fire == 1);
  assert(state.supporter_used);
  assert(state.manual_energy_used);
  assert(contains(state.discard, sim::Card::RoseannesBackup));
  assert(contains(state.discard, sim::Card::PokemonCommunication));
  assert(contains(state.discard, sim::Card::EarthenVessel));
  assert(contains(state.deck, sim::Card::Dipplin));
}

void missing_vstar_is_rejected() {
  Fixture fixture{"issue-837-no-vstar", sim::LockMode::None, 83704};
  sim::State state = route_state();
  state.discard.erase(std::remove(state.discard.begin(), state.discard.end(),
                                  sim::Card::RegidragoVstar),
                      state.discard.end());
  sim::EngineTestAccess::state(fixture.engine) = state;
  assert(!sim::EngineTestAccess::play_recovery(fixture.engine));
}

void missing_energy_is_rejected() {
  Fixture fixture{"issue-837-no-energy", sim::LockMode::None, 83705};
  sim::State state = route_state();
  state.discard.erase(std::remove(state.discard.begin(), state.discard.end(),
                                  sim::Card::Fire),
                      state.discard.end());
  sim::EngineTestAccess::state(fixture.engine) = state;
  assert(!sim::EngineTestAccess::play_recovery(fixture.engine));
}

void unsafe_communication_exchange_is_rejected() {
  Fixture fixture{"issue-837-no-exchange", sim::LockMode::None, 83706};
  sim::State state = route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Dipplin));
  sim::EngineTestAccess::state(fixture.engine) = state;
  assert(!sim::EngineTestAccess::play_recovery(fixture.engine));
}

void unpayable_vessel_is_rejected() {
  Fixture fixture{"issue-837-vessel-cost", sim::LockMode::None, 83707};
  sim::State state = route_state();
  state.hand = {sim::Card::RoseannesBackup, sim::Card::PokemonCommunication,
                sim::Card::Dipplin, sim::Card::EarthenVessel};
  sim::EngineTestAccess::state(fixture.engine) = state;
  assert(!sim::EngineTestAccess::play_recovery(fixture.engine));
}

void item_lock_is_rejected() {
  Fixture fixture{"issue-837-item-lock", sim::LockMode::FullItem, 83708};
  sim::EngineTestAccess::state(fixture.engine) = route_state();
  assert(!sim::EngineTestAccess::play_recovery(fixture.engine));
}

void used_manual_attachment_is_rejected() {
  Fixture fixture{"issue-837-manual-used", sim::LockMode::None, 83709};
  sim::State state = route_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::state(fixture.engine) = state;
  assert(!sim::EngineTestAccess::play_recovery(fixture.engine));
}

void current_turn_regidrago_is_rejected() {
  Fixture fixture{"issue-837-cannot-evolve", sim::LockMode::None, 83710};
  sim::State state = route_state();
  state.active->entered_turn = 2;
  sim::EngineTestAccess::state(fixture.engine) = state;
  assert(!sim::EngineTestAccess::play_recovery(fixture.engine));
}

}  // namespace

int main() {
  k0_moves_restored_cards_between_zones();
  k1_preserves_the_exact_route();
  full_turn_evolves_and_attaches_restored_fire();
  missing_vstar_is_rejected();
  missing_energy_is_rejected();
  unsafe_communication_exchange_is_rejected();
  unpayable_vessel_is_rejected();
  item_lock_is_rejected();
  used_manual_attachment_is_rejected();
  current_turn_regidrago_is_rejected();
  return 0;
}
