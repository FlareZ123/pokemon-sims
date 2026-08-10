from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"


def atomic_write_locked(path: Path, text: str) -> None:
    lock_path = path.with_name(f"{path.name}.lock")
    lock_fd = os.open(lock_path, os.O_CREAT | os.O_EXCL | os.O_RDWR)
    try:
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", newline="\n", dir=path.parent, delete=False
        ) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_path = Path(tmp.name)
        os.replace(tmp_path, path)
    finally:
        os.close(lock_fd)
        lock_path.unlink(missing_ok=True)


source = SOURCE.read_text(encoding="utf-8")
old_gate = '''    if (state_.active->card != Card::Oricorio &&
        state_.active->card != Card::TapuLeleGX) {
      return false;
    }
'''
new_gate = '''    // This is a semantic mobility gate, not a witness-card list. A single held
    // Basic Energy can fund this route exactly when the Active is a Basic with
    // printed Retreat Cost 1; higher-cost Basics require more payment and must be
    // rejected before any once-per-turn resource is spent:
    // Official Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Simulator Retreat Cost metadata: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_classification.inc
    // State-driven route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
    if (!is_basic(state_.active->card) || retreat_cost(state_.active->card) != 1) {
      return false;
    }
'''
if source.count(old_gate) != 1:
    raise RuntimeError(f"identity gate count {source.count(old_gate)}")
source = source.replace(old_gate, new_gate, 1)

old_initial = '''        !state_.active || hand_count(Card::BrilliantBlender) == 0 ||
        !payload_might_be_in_deck() || !can_play_payload_this_turn()) {
'''
new_initial = '''        !state_.active || hand_count(Card::BrilliantBlender) == 0 ||
        !payload_might_be_in_deck()) {
'''
if source.count(old_initial) != 1:
    raise RuntimeError(f"pre-retreat payload gate count {source.count(old_initial)}")
source = source.replace(old_initial, new_initial, 1)

old_comment = '''    // Oricorio GRI 55 and Tapu Lele-GX each have a one-Colorless Retreat Cost.
    // Either modeled Basic Energy can pay that cost while the Apex-ready Benched
    // Regidrago VSTAR keeps GGF. The preflight above proves Brilliant Blender can
    // then establish a legal current-turn Dragon payload before readiness is checked:
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
'''
new_comment = '''    // The semantic gate above proves the Basic Active has printed Retreat Cost 1,
    // so either modeled Basic Energy can pay that cost while the Apex-ready Benched
    // Regidrago VSTAR keeps GGF. Payload legality is checked on the exact projected
    // post-retreat state by play_brilliant_blender(), rather than by a pre-retreat
    // reachability predicate that can depend on the current Active identity.
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Mawile-GX: https://api.pokemontcg.io/v2/cards/sm11-141
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
'''
if source.count(old_comment) != 1:
    raise RuntimeError(f"route comment count {source.count(old_comment)}")
source = source.replace(old_comment, new_comment, 1)
atomic_write_locked(SOURCE, source)
