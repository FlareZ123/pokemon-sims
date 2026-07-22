from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path


TARGET = Path("src/trace_engine_v2/part_010_steven_crispin_override.inc")
ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/1351"


def atomic_locked_write(path: Path, content: str) -> None:
    lock_path = path.with_suffix(path.suffix + ".lock")
    with lock_path.open("w", encoding="utf-8") as lock_file:
        fcntl.flock(lock_file.fileno(), fcntl.LOCK_EX)
        fd, temporary_name = tempfile.mkstemp(
            dir=path.parent, prefix=path.name + ".", suffix=".tmp", text=True
        )
        try:
            with os.fdopen(fd, "w", encoding="utf-8", newline="") as temporary:
                temporary.write(content)
                temporary.flush()
                os.fsync(temporary.fileno())
            os.replace(temporary_name, path)
        finally:
            if os.path.exists(temporary_name):
                os.unlink(temporary_name)
            fcntl.flock(lock_file.fileno(), fcntl.LOCK_UN)
    lock_path.unlink(missing_ok=True)


def main() -> int:
    text = TARGET.read_text(encoding="utf-8")
    if ISSUE_URL in text:
        return 0

    old_t1 = """  bool wonder_tag_can_bank_steven_for_known_t3_blender_route() const {
    if (state_.turn != 1 || !scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
"""
    new_t1 = """  bool wonder_tag_can_bank_steven_for_known_t3_blender_route() const {
    const bool supported_payload_profile =
        scenario_.dci == DciProfile::StrictJit ||
        scenario_.dci == DciProfile::NoDiscardControl;
    if (state_.turn != 1 || !scenario_.going_first ||
        !supported_payload_profile ||
"""

    old_t1_comment = """    // attachments. Steven then takes Regidrago VSTAR, the third Grass Energy, and
    // Latias ex. Held Brilliant Blender supplies the T3 payload, and Skyliner
    // promotes the Benched attacker without consuming the T3 Supporter play:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/946
"""
    new_t1_comment = """    // attachments. Strict JIT holds Brilliant Blender for T3; no-discard-control
    // may legally bank the payload with Blender on T1. Steven then takes Regidrago
    // VSTAR, the third Grass Energy, and Latias ex, whose Skyliner promotes the
    // Benched attacker without consuming the T3 Supporter play:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/issues/946
    // https://github.com/FlareZ123/pokemon-sims/issues/1351
"""

    old_t2 = """  bool banked_steven_has_known_t3_blender_route() const {
    if (state_.turn != 2 || !scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !supporter_allowed() || !deck_seen_ || !state_.active ||
        state_.active->card != Card::TapuLeleGX || state_.retreat_used ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 || !need_payload() ||
        !payload_might_be_in_deck() ||
"""
    new_t2 = """  bool banked_steven_has_known_t3_blender_route() const {
    const bool strict_jit_payload_continuation =
        scenario_.dci == DciProfile::StrictJit &&
        hand_count(Card::BrilliantBlender) > 0 && need_payload() &&
        payload_might_be_in_deck();
    const bool no_discard_payload_already_banked =
        scenario_.dci == DciProfile::NoDiscardControl && payload_ready() &&
        hand_count(Card::BrilliantBlender) == 0 &&
        std::find(state_.discard.begin(), state_.discard.end(),
                  Card::BrilliantBlender) != state_.discard.end();
    if (state_.turn != 2 || !scenario_.going_first ||
        (!strict_jit_payload_continuation && !no_discard_payload_already_banked) ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !supporter_allowed() || !deck_seen_ || !state_.active ||
        state_.active->card != Card::TapuLeleGX || state_.retreat_used ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::StevensResolve) == 0 ||
"""

    old_t2_comment = """        // Energy, and Latias. This route has no Crispin or future-draw dependency and
        // leaves the T3 Supporter play available:
        // https://api.pokemontcg.io/v2/cards/sm7-145
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://api.pokemontcg.io/v2/cards/sv8-164
        // https://api.pokemontcg.io/v2/cards/sv8-76
        // https://www.pokemon.com/us/pokemon-tcg/rules
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // https://github.com/FlareZ123/pokemon-sims/issues/946
"""
    new_t2_comment = """        // Energy, and Latias. This route has no Crispin or future-draw dependency and
        // leaves the T3 Supporter play available. Strict JIT retains Blender for
        // the T3 discard; no-discard-control proves Blender and the payload already
        // entered discard on T1 before this continuation is admitted:
        // https://api.pokemontcg.io/v2/cards/sm7-145
        // https://api.pokemontcg.io/v2/cards/swsh12-136
        // https://api.pokemontcg.io/v2/cards/sv8-164
        // https://api.pokemontcg.io/v2/cards/sv8-76
        // https://www.pokemon.com/us/pokemon-tcg/rules
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
        // https://github.com/FlareZ123/pokemon-sims/issues/946
        // https://github.com/FlareZ123/pokemon-sims/issues/1351
"""

    replacements = [
        (old_t1, new_t1, "turn-one profile gate"),
        (old_t1_comment, new_t1_comment, "turn-one source comment"),
        (old_t2, new_t2, "turn-two continuation gate"),
        (old_t2_comment, new_t2_comment, "turn-two source comment"),
    ]
    for old, new, label in replacements:
        if text.count(old) != 1:
            raise RuntimeError(f"Expected exactly one {label} anchor")
        text = text.replace(old, new, 1)

    atomic_locked_write(TARGET, text)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
