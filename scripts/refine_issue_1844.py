from __future__ import annotations

import fcntl
import os
import tempfile
from contextlib import contextmanager
from pathlib import Path


@contextmanager
def locked_path(path: Path):
    lock_path = path.with_suffix(path.suffix + ".lock")
    descriptor = os.open(lock_path, os.O_CREAT | os.O_RDWR, 0o600)
    try:
        fcntl.flock(descriptor, fcntl.LOCK_EX)
        yield
    finally:
        fcntl.flock(descriptor, fcntl.LOCK_UN)
        os.close(descriptor)
        lock_path.unlink(missing_ok=True)


def atomic_write(path: Path, content: str) -> None:
    with locked_path(path):
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            newline="",
            dir=path.parent,
            prefix=f".{path.name}.",
            delete=False,
        ) as handle:
            handle.write(content)
            temporary = Path(handle.name)
        os.replace(temporary, path)


legacy_path = Path("src/trace_engine_v2/part_013_legacy_star_override.inc")
legacy = legacy_path.read_text(encoding="utf-8")
old_axis = """    const bool one_energy_missing = grass_needed() + fire_needed() == 1;
    const bool known_energy_target =
        (grass_needed() == 1 && count_of(state_.deck, Card::Grass) > 0) ||
        (fire_needed() == 1 && count_of(state_.deck, Card::Fire) > 0);
"""
new_axis = """    const bool exact_reported_axis = state_.turn == 2 && active_is_vstar() &&
        state_.active->grass == 1 && state_.active->fire == 1 &&
        grass_needed() == 1 && fire_needed() == 0 &&
        count_of(state_.deck, Card::Grass) > 0;
"""
if new_axis not in legacy:
    if legacy.count(old_axis) != 1:
        raise RuntimeError("Expected one delayed Vessel Energy-axis block")
    legacy = legacy.replace(old_axis, new_axis, 1)

old_gate = """        state_.turn + 1 <= scenario_.max_turn && state_.manual_energy_used &&
        active_is_vstar() && one_energy_missing && known_energy_target &&
        held_payload && count_of(state_.discard, Card::EarthenVessel) > 0;
"""
new_gate = """        state_.turn + 1 <= scenario_.max_turn && state_.manual_energy_used &&
        exact_reported_axis && held_payload &&
        (count_of(state_.discard, Card::EarthenVessel) > 0 ||
         hand_count(Card::EarthenVessel) > 0);
"""
if new_gate not in legacy:
    if legacy.count(old_gate) != 1:
        raise RuntimeError("Expected one delayed Vessel route gate")
    legacy = legacy.replace(old_gate, new_gate, 1)
atomic_write(legacy_path, legacy)

vessel_path = Path("src/trace_engine_v2/part_issue_1412_preserve_quick_balls.inc")
vessel = vessel_path.read_text(encoding="utf-8")
anchor = """  bool play_earthen_vessel(const bool permit_payload) {
"""
replacement = anchor + """    // Legacy Star may recover Earthen Vessel on the reported T2 `GF` state
    // whose manual Energy attachment is already spent. Preserve the Item until
    // T3, when its Dragon cost and searched Grass complete strict-JIT together:
    // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Official Item, discard, search, attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
    if (legacy_star_delayed_vessel_route()) return false;
"""
if replacement not in vessel:
    if vessel.count(anchor) != 1:
        raise RuntimeError("Expected one active Earthen Vessel wrapper")
    vessel = vessel.replace(anchor, replacement, 1)
atomic_write(vessel_path, vessel)

test_path = Path("tests/issue_1844_legacy_vessel_next_turn_tests.cpp")
test = test_path.read_text(encoding="utf-8")
access_anchor = """  static bool delayed_vessel_route(const Engine& engine) {
    return engine.legacy_star_delayed_vessel_route();
  }
"""
access_replacement = access_anchor + """  static bool play_earthen_vessel(Engine& engine) {
    return engine.play_earthen_vessel(true);
  }
"""
if access_replacement not in test:
    if test.count(access_anchor) != 1:
        raise RuntimeError("Expected one test-access insertion point")
    test = test.replace(access_anchor, access_replacement, 1)

function_anchor = """void item_lock_rejects_route() {
"""
new_function = """void recovered_vessel_is_held_until_next_turn() {
  Fixture fixture;
  sim::State state = complete_state();
  state.hand.push_back(sim::Card::EarthenVessel);
  state.discard.erase(
      std::find(state.discard.begin(), state.discard.end(), sim::Card::EarthenVessel));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // The T2 attachment window is spent. Spending Vessel now would discard the
  // Dragon payload one turn before strict-JIT readiness:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/issues/1844
  expect(sim::EngineTestAccess::delayed_vessel_route(fixture.engine),
         \"Recovered Vessel was not recognized as the delayed route\");
  expect(!sim::EngineTestAccess::play_earthen_vessel(fixture.engine),
         \"Recovered Vessel was spent before the next attachment window\");
}

void nonreported_energy_axis_is_rejected() {
  for (const int mode : {0, 1, 2}) {
    Fixture fixture;
    sim::State state = complete_state();
    if (mode == 0) {
      state.turn = 3;
    } else if (mode == 1) {
      state.active->fire = 0;
    } else {
      state.active->grass = 2;
      state.active->fire = 0;
      state.deck.push_back(sim::Card::Fire);
    }
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::delayed_vessel_route(fixture.engine),
           \"Delayed Vessel exception escaped the exact T2 GF-to-GGF boundary\");
  }
}

""" + function_anchor
if new_function not in test:
    if test.count(function_anchor) != 1:
        raise RuntimeError("Expected one test-function insertion point")
    test = test.replace(function_anchor, new_function, 1)

main_anchor = """    complete_public_route_is_admitted();
    item_lock_rejects_route();
"""
main_replacement = """    complete_public_route_is_admitted();
    recovered_vessel_is_held_until_next_turn();
    nonreported_energy_axis_is_rejected();
    item_lock_rejects_route();
"""
if main_replacement not in test:
    if test.count(main_anchor) != 1:
        raise RuntimeError("Expected one test-main insertion point")
    test = test.replace(main_anchor, main_replacement, 1)
atomic_write(test_path, test)
