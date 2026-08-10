#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine, const bool seen) { engine.deck_seen_ = seen; }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
  static bool play_earthen_vessel(Engine& engine) { return engine.play_earthen_vessel(true); }
  static bool attach_powerglass(Engine& engine) { return engine.attach_powerglass(); }
  static bool resolve_powerglass(Engine& engine) { return engine.resolve_powerglass_end_turn(); }
  static bool powerglass_vessel_finish_visible(const Engine& engine) {
    return engine.powerglass_jit_vessel_finish_visible();
  }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
  static bool pays_apex(const Engine& engine) {
    return engine.state_.active && engine.pays_apex_energy_cost(*engine.state_.active);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

const sim::Scenario& scenario_for(const sim::LockMode locks) {
  static const sim::Scenario no_lock{"issue-2834", sim::DciProfile::StrictJit,
                                     sim::LockMode::None, true, 4};
  static const sim::Scenario item_lock{"issue-2834-item-lock", sim::DciProfile::StrictJit,
                                       sim::LockMode::FullItem, true, 4};
  return locks == sim::LockMode::FullItem ? item_lock : no_lock;
}

sim::Engine make_engine(const sim::LockMode locks = sim::LockMode::None) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng{2834};
  return sim::Engine(scenario_for(locks), recipe, rng);
}

void set_post_crispin_powerglass_state(sim::Engine& engine) {
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None};
  state.manual_energy_used = true;
  state.supporter_used = true;
  state.hand = {sim::Card::Powerglass, sim::Card::MegaDragonite};
  state.discard = {sim::Card::MysteriousTreasure, sim::Card::RegidragoV};
  // Legacy Star removes from the back. Grass Energy and Dragapult ex remain after
  // the seven-card discard as the legal Vessel target and public payload possibility.
  state.deck = {sim::Card::Grass, sim::Card::Dragapult,
                sim::Card::RoseannesBackup, sim::Card::ErikasInvitation,
                sim::Card::Arven, sim::Card::Grass, sim::Card::Grass,
                sim::Card::ForestSealStone, sim::Card::EarthenVessel};
  sim::EngineTestAccess::set_deck_seen(engine, true);
}

void test_legacy_star_vessel_and_powerglass_finish_on_turn_two() {
  sim::Engine engine = make_engine();
  set_post_crispin_powerglass_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);

  // Legacy Star recovers Earthen Vessel during T2. Vessel can immediately discard
  // Mega Dragonite ex as the current-turn strict-JIT payload, then Powerglass can
  // attach the final Basic Grass from discard at end of the same turn. The resulting
  // T2 board satisfies the repository's ready-state contract:
  // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Ready-state objective: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scope
  // Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2834
  if (!sim::EngineTestAccess::use_legacy_star(engine) ||
      !contains(state.hand, sim::Card::EarthenVessel)) {
    throw std::runtime_error("Legacy Star did not recover the live T2 Earthen Vessel route.");
  }
  if (!sim::EngineTestAccess::powerglass_vessel_finish_visible(engine)) {
    throw std::runtime_error("The observable T2 Vessel plus Powerglass finish was not recognized.");
  }
  if (!sim::EngineTestAccess::play_earthen_vessel(engine) ||
      !contains(state.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Recovered Vessel did not create the same-turn Dragon payload.");
  }
  if (!sim::EngineTestAccess::attach_powerglass(engine) ||
      !sim::EngineTestAccess::resolve_powerglass(engine)) {
    throw std::runtime_error("Powerglass did not resolve after the T2 Vessel route.");
  }
  if (state.turn != 2 || !sim::EngineTestAccess::pays_apex(engine) ||
      !sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("Vessel plus Powerglass did not complete the T2 ready state.");
  }
}

void test_powerglass_vessel_finish_requires_a_complete_public_route() {
  {
    sim::Engine engine = make_engine(sim::LockMode::FullItem);
    set_post_crispin_powerglass_state(engine);
    if (sim::EngineTestAccess::powerglass_vessel_finish_visible(engine) ||
        sim::EngineTestAccess::play_earthen_vessel(engine)) {
      throw std::runtime_error("Item lock must reject the recovered Vessel route.");
    }
  }
  {
    sim::Engine engine = make_engine();
    set_post_crispin_powerglass_state(engine);
    sim::State& state = sim::EngineTestAccess::state(engine);
    state.active->grass = 0;
    if (sim::EngineTestAccess::powerglass_vessel_finish_visible(engine)) {
      throw std::runtime_error("One Powerglass attachment cannot complete two missing Energy.");
    }
  }
  {
    sim::Engine engine = make_engine();
    set_post_crispin_powerglass_state(engine);
    sim::State& state = sim::EngineTestAccess::state(engine);
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite));
    if (sim::EngineTestAccess::powerglass_vessel_finish_visible(engine)) {
      throw std::runtime_error("The route requires a held Dragon payload for Vessel's cost.");
    }
  }
  {
    sim::Engine engine = make_engine();
    set_post_crispin_powerglass_state(engine);
    sim::State& state = sim::EngineTestAccess::state(engine);
    state.active->grass = 2;
    state.active->fire = 0;
    state.discard.push_back(sim::Card::Fire);
    if (!sim::EngineTestAccess::powerglass_vessel_finish_visible(engine)) {
      throw std::runtime_error("The systemic route must support a missing Fire Powerglass finish.");
    }
  }
  {
    sim::Engine engine = make_engine();
    set_post_crispin_powerglass_state(engine);
    sim::State& state = sim::EngineTestAccess::state(engine);
    state.active->tool = sim::Tool::Powerglass;
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Powerglass));
    if (!sim::EngineTestAccess::powerglass_vessel_finish_visible(engine)) {
      throw std::runtime_error("An already attached Powerglass must keep the same legal route live.");
    }
  }
}

}  // namespace

int main() {
  test_legacy_star_vessel_and_powerglass_finish_on_turn_two();
  test_powerglass_vessel_finish_requires_a_complete_public_route();
  std::cout << "Legacy Star current-turn Powerglass Vessel tests passed\n";
}
