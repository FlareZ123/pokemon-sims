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
  static bool hold_post_powerglass_outlet(const Engine& engine) {
    return engine.hold_payload_outlet_for_post_powerglass_turn();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

const sim::Scenario& scenario_for(const sim::LockMode locks, const int max_turn) {
  static const sim::Scenario no_lock_turn4{"issue-944", sim::DciProfile::StrictJit,
                                           sim::LockMode::None, false, 4};
  static const sim::Scenario item_lock_turn4{"issue-944-item-lock", sim::DciProfile::StrictJit,
                                             sim::LockMode::FullItem, false, 4};
  static const sim::Scenario no_lock_turn2{"issue-944-horizon", sim::DciProfile::StrictJit,
                                           sim::LockMode::None, false, 2};
  if (locks == sim::LockMode::FullItem) return item_lock_turn4;
  if (max_turn == 2) return no_lock_turn2;
  return no_lock_turn4;
}

sim::Engine make_engine(const sim::LockMode locks = sim::LockMode::None,
                        const int max_turn = 4) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng{944};
  return sim::Engine(scenario_for(locks, max_turn), recipe, rng);
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

void test_legacy_star_recovers_and_holds_post_powerglass_vessel() {
  sim::Engine engine = make_engine();
  set_post_crispin_powerglass_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);

  // Legacy Star may recover Earthen Vessel. Powerglass supplies the final Grass only
  // after the attack step, so strict-JIT must hold Vessel and its Dragon cost until
  // turn 3 instead of spending both recovery slots on future Supporters:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/944
  if (!sim::EngineTestAccess::use_legacy_star(engine) ||
      !contains(state.hand, sim::Card::EarthenVessel)) {
    throw std::runtime_error("Legacy Star did not preserve the post-Powerglass Vessel route.");
  }
  if (!sim::EngineTestAccess::hold_post_powerglass_outlet(engine) ||
      sim::EngineTestAccess::play_earthen_vessel(engine)) {
    throw std::runtime_error("Earthen Vessel was spent before Powerglass completed GGF.");
  }
  if (!contains(state.hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The strict-JIT payload was discarded one turn too early.");
  }

  if (!sim::EngineTestAccess::attach_powerglass(engine) ||
      !sim::EngineTestAccess::resolve_powerglass(engine) ||
      !state.active || state.active->grass != 2 || state.active->fire != 1) {
    throw std::runtime_error("Powerglass did not complete the exact GGF route.");
  }

  state.turn = 3;
  state.manual_energy_used = false;
  state.supporter_used = false;
  state.discarded_this_turn.clear();
  if (sim::EngineTestAccess::hold_post_powerglass_outlet(engine) ||
      !sim::EngineTestAccess::play_earthen_vessel(engine) ||
      !contains(state.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The held Vessel did not create the turn-3 payload axis.");
  }
}

void test_post_powerglass_hold_requires_exact_live_route() {
  {
    sim::Engine engine = make_engine(sim::LockMode::FullItem);
    set_post_crispin_powerglass_state(engine);
    if (sim::EngineTestAccess::hold_post_powerglass_outlet(engine)) {
      throw std::runtime_error("Item lock must reject the delayed Vessel route.");
    }
  }
  {
    sim::Engine engine = make_engine();
    set_post_crispin_powerglass_state(engine);
    sim::EngineTestAccess::state(engine).hand.erase(
        std::find(sim::EngineTestAccess::state(engine).hand.begin(),
                  sim::EngineTestAccess::state(engine).hand.end(),
                  sim::Card::MegaDragonite));
    if (sim::EngineTestAccess::hold_post_powerglass_outlet(engine)) {
      throw std::runtime_error("The delayed route requires a held Dragon payload.");
    }
  }
  {
    sim::Engine engine = make_engine();
    set_post_crispin_powerglass_state(engine);
    sim::EngineTestAccess::state(engine).active->grass = 0;
    if (sim::EngineTestAccess::hold_post_powerglass_outlet(engine)) {
      throw std::runtime_error("Powerglass cannot complete two missing Energy.");
    }
  }
  {
    sim::Engine engine = make_engine();
    set_post_crispin_powerglass_state(engine);
    sim::EngineTestAccess::state(engine).active->card = sim::Card::RegidragoV;
    if (sim::EngineTestAccess::hold_post_powerglass_outlet(engine)) {
      throw std::runtime_error("The delayed route requires the Active VSTAR holder.");
    }
  }
  {
    sim::Engine engine = make_engine(sim::LockMode::None, 2);
    set_post_crispin_powerglass_state(engine);
    if (sim::EngineTestAccess::hold_post_powerglass_outlet(engine)) {
      throw std::runtime_error("A closed simulation horizon must reject the delayed route.");
    }
  }
}

}  // namespace

int main() {
  test_legacy_star_recovers_and_holds_post_powerglass_vessel();
  test_post_powerglass_hold_requires_exact_live_route();
  std::cout << "Legacy Star post-Powerglass Vessel tests passed\n";
}
