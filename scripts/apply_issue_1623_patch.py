from __future__ import annotations

import os
import tempfile
from pathlib import Path
from typing import BinaryIO


def _lock(file: BinaryIO) -> None:
    if os.name == "nt":
        import msvcrt

        file.seek(0, os.SEEK_END)
        if file.tell() == 0:
            file.write(b"\0")
            file.flush()
        file.seek(0)
        msvcrt.locking(file.fileno(), msvcrt.LK_LOCK, 1)
        return

    import fcntl

    fcntl.flock(file.fileno(), fcntl.LOCK_EX)


def _unlock(file: BinaryIO) -> None:
    if os.name == "nt":
        import msvcrt

        file.seek(0)
        msvcrt.locking(file.fileno(), msvcrt.LK_UNLCK, 1)
        return

    import fcntl

    fcntl.flock(file.fileno(), fcntl.LOCK_UN)


def locked_atomic_write(path: Path, content: str) -> None:
    lock_path = path.with_name(f".{path.name}.lock")
    lock_path.parent.mkdir(parents=True, exist_ok=True)
    with lock_path.open("a+b") as lock_file:
        _lock(lock_file)
        temp_name: str | None = None
        try:
            with tempfile.NamedTemporaryFile(
                mode="w",
                encoding="utf-8",
                newline="",
                dir=path.parent,
                prefix=f".{path.name}.",
                suffix=".tmp",
                delete=False,
            ) as temp_file:
                temp_file.write(content)
                temp_file.flush()
                os.fsync(temp_file.fileno())
                temp_name = temp_file.name
            os.replace(temp_name, path)
        finally:
            if temp_name is not None and os.path.exists(temp_name):
                os.unlink(temp_name)
            _unlock(lock_file)
    lock_path.unlink(missing_ok=True)


source_path = Path("src/trace_engine_v2/part_celestial_roar_override.inc")
source = source_path.read_text(encoding="utf-8")
source_anchor = """    const bool held_energy_and_payload_route_guarantee_next_window =
        strict_payload_timing() && state_.manual_energy_used &&
        state_.turn < scenario_.max_turn && missing_grass + missing_fire == 1 &&
        ((missing_grass == 1 && hand_count(Card::Grass) > 0) ||
         (missing_fire == 1 && hand_count(Card::Fire) > 0)) &&
        held_supporter_payload_route;
"""
source_replacement = """    const bool held_needed_energy =
        (missing_grass == 1 && hand_count(Card::Grass) > 0) ||
        (missing_fire == 1 && hand_count(Card::Fire) > 0);
    const bool strict_held_payload_route =
        strict_payload_timing() && held_supporter_payload_route;
    // No-discard-control must also preserve the random top-three attack when a
    // payload is already banked, Regidrago VSTAR is held, and one held Basic
    // Energy guarantees GGF at the next legal manual-attachment and evolution
    // window. In that state Celestial Roar cannot improve the ready turn and can
    // only discard still-useful connectors:
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core attachment, evolution, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // No-control payload timing and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1623
    const bool no_control_banked_payload_route =
        !strict_payload_timing() && !need_payload() &&
        hand_count(Card::RegidragoVstar) > 0;
    const bool held_energy_and_payload_route_guarantee_next_window =
        state_.manual_energy_used && state_.turn < scenario_.max_turn &&
        missing_grass + missing_fire == 1 && held_needed_energy &&
        (strict_held_payload_route || no_control_banked_payload_route);
"""
if source.count(source_anchor) != 1:
    raise SystemExit(
        f"issue-1623 source anchor count: {source.count(source_anchor)}"
    )
source = source.replace(source_anchor, source_replacement, 1)
source = source.replace(
    '            "Held Energy and a held Supporter payload route already cover the next legal ready window.");',
    '            "Held Energy and a guaranteed payload route already cover the next legal ready window.");',
    1,
)
locked_atomic_write(source_path, source)


test_path = Path("tests/run_issue_1079_celestial_roar_hold.cmake")
test_source = test_path.read_text(encoding="utf-8")
test_anchor = """# No-discard-control may bank an early Dragon payload, so the held-route suppression
# must remain limited to strict and matchup-flex JIT:
# https://api.pokemontcg.io/v2/cards/swsh12-135
# https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# https://github.com/FlareZ123/pokemon-sims/issues/1079
run_trace("no-discard-control/go-second" 19 control_seed_19)
"""
test_replacement = """# Once no-discard-control already has its Dragon payload in discard, held Fire and
# a held Regidrago VSTAR guarantee the next legal T2 ready window. Seed 91 must
# preserve the random top three because the attack cannot improve Energy or payload:
# Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
# Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
# Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
# Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
# Core attachment, evolution, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# No-control timing and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1623
run_trace("no-discard-control/go-second" 91 no_control_seed_91)
if(NOT no_control_seed_91 MATCHES "T1 \\| HOLD ATTACK \\|")
  message(FATAL_ERROR "No-control seed 91 did not hold Celestial Roar:\n${no_control_seed_91}")
endif()
if(no_control_seed_91 MATCHES "T1 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "No-control seed 91 still used Celestial Roar:\n${no_control_seed_91}")
endif()
if(NOT no_control_seed_91 MATCHES "T2 \\| READY \\|")
  message(FATAL_ERROR "No-control seed 91 lost T2 readiness:\n${no_control_seed_91}")
endif()

# No-discard-control may still bank an early Dragon payload when that axis is
# missing, so the existing seed-19 attack remains a required positive control:
# https://api.pokemontcg.io/v2/cards/swsh12-135
# https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# https://github.com/FlareZ123/pokemon-sims/issues/1079
# https://github.com/FlareZ123/pokemon-sims/issues/1623
run_trace("no-discard-control/go-second" 19 control_seed_19)
"""
if test_source.count(test_anchor) != 1:
    raise SystemExit(
        f"issue-1623 test anchor count: {test_source.count(test_anchor)}"
    )
test_source = test_source.replace(test_anchor, test_replacement, 1)
locked_atomic_write(test_path, test_source)
