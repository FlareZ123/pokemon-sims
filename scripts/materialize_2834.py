from __future__ import annotations

import fcntl
import os
import re
import tempfile
from pathlib import Path


def atomic_locked_write(path: Path, content: str) -> None:
    with path.open("r", encoding="utf-8") as source_file:
        fcntl.flock(source_file.fileno(), fcntl.LOCK_EX)
        fd, temporary_name = tempfile.mkstemp(
            dir=path.parent, prefix=path.name + ".", suffix=".tmp"
        )
        try:
            with os.fdopen(fd, "w", encoding="utf-8", newline="") as temporary_file:
                temporary_file.write(content)
                temporary_file.flush()
                os.fsync(temporary_file.fileno())
            os.replace(temporary_name, path)
        finally:
            if os.path.exists(temporary_name):
                os.unlink(temporary_name)


source_path = Path("src/trace_engine_v2/part_010.inc")
source = source_path.read_text(encoding="utf-8")
helper_pattern = re.compile(
    r"\n  bool hold_payload_outlet_for_post_powerglass_turn\(\) const \{.*?\n  \}\n\n  bool play_earthen_vessel",
    re.DOTALL,
)
source, helper_count = helper_pattern.subn("\n  bool play_earthen_vessel", source, count=1)
if helper_count != 1:
    raise RuntimeError("expected exactly one historical #944 Powerglass hold helper")

old_gate = """    // Do not discard the held Dragon before Powerglass completes GGF after the attack
    // step. The recovered Vessel remains live on the first post-Powerglass turn:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/944
    if (permit_payload && hold_payload_outlet_for_post_powerglass_turn()) return false;
"""
new_gate = """    // A current-turn Earthen Vessel payload discard composes with Powerglass's
    // end-of-turn attachment. Repository readiness is the resulting legal board
    // state, so deferring this payload to the next turn misses an earlier ready state:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Ready-state objective: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scope
    // Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed systemic correction: https://github.com/FlareZ123/pokemon-sims/issues/2834
"""
if source.count(old_gate) != 1:
    raise RuntimeError("expected exactly one historical #944 Vessel hold gate")
source = source.replace(old_gate, new_gate, 1)
atomic_locked_write(source_path, source)

test_path = Path("tests/legacy_star_post_powerglass_vessel_tests.cpp")
test_source = r'''#define REGIDRAGO_SIM_NO_MAIN
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
  state.deck = {sim::Card::Grass, sim::Card::Dragapult,
                sim::Card::RoseannesBackup, sim::Card::ErikasInvitation,
                sim::Card::Arven, sim::Card::Grass, sim::Card::Grass,
                sim::Card::ForestSealStone, sim::Card::EarthenVessel};
  sim::EngineTestAccess::set_deck_seen(engine, true);
}

void test_recovered_vessel_completes_current_turn_with_powerglass() {
  sim::Engine engine = make_engine();
  set_post_crispin_powerglass_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);

  // Legacy Star may recover Earthen Vessel during T2. Vessel may then discard the
  // held Dragon in that same turn, and Powerglass may attach the final Basic Grass
  // from discard at end of turn. The resulting T2 state satisfies the repository's
  // readiness contract and strict-JIT payload timing:
  // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Ready-state objective: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scope
  // Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2834
  if (!sim::EngineTestAccess::use_legacy_star(engine) ||
      !contains(state.hand, sim::Card::EarthenVessel)) {
    throw std::runtime_error("Legacy Star did not recover the live Earthen Vessel route.");
  }
  if (!sim::EngineTestAccess::play_earthen_vessel(engine) ||
      !contains(state.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Recovered Vessel did not create the same-turn Dragon payload.");
  }
  if (!sim::EngineTestAccess::attach_powerglass(engine) ||
      !sim::EngineTestAccess::resolve_powerglass(engine)) {
    throw std::runtime_error("Powerglass did not resolve after the current-turn Vessel route.");
  }
  if (state.turn != 2 || !sim::EngineTestAccess::pays_apex(engine) ||
      !sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("Vessel plus Powerglass did not complete the T2 ready state.");
  }
}

void test_item_lock_still_blocks_recovered_vessel() {
  sim::Engine engine = make_engine(sim::LockMode::FullItem);
  set_post_crispin_powerglass_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);
  if (!sim::EngineTestAccess::use_legacy_star(engine) ||
      !contains(state.hand, sim::Card::EarthenVessel)) {
    throw std::runtime_error("Legacy Star fixture did not recover Earthen Vessel under Item lock.");
  }
  if (sim::EngineTestAccess::play_earthen_vessel(engine)) {
    throw std::runtime_error("Item lock must still prevent the recovered Earthen Vessel play.");
  }
}

void test_powerglass_does_not_fake_completion_with_two_energy_missing() {
  sim::Engine engine = make_engine();
  set_post_crispin_powerglass_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.active->grass = 0;
  if (!sim::EngineTestAccess::use_legacy_star(engine) ||
      !sim::EngineTestAccess::play_earthen_vessel(engine) ||
      !sim::EngineTestAccess::attach_powerglass(engine) ||
      !sim::EngineTestAccess::resolve_powerglass(engine)) {
    throw std::runtime_error("Two-Energy control could not execute its legal actions.");
  }
  if (sim::EngineTestAccess::pays_apex(engine)) {
    throw std::runtime_error("One Powerglass attachment incorrectly completed two missing Energy.");
  }
}

}  // namespace

int main() {
  test_recovered_vessel_completes_current_turn_with_powerglass();
  test_item_lock_still_blocks_recovered_vessel();
  test_powerglass_does_not_fake_completion_with_two_energy_missing();
  std::cout << "Legacy Star current-turn Powerglass Vessel tests passed\n";
}
'''
atomic_locked_write(test_path, test_source)
