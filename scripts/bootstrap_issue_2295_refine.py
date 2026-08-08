from __future__ import annotations

import os
import tempfile
from pathlib import Path


PATH = Path("src/trace_engine_v2/part_014c_latias_bench_override.inc")
LOCK = PATH.with_suffix(PATH.suffix + ".issue2295.lock")
OLD = """    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || target->grass < 2 || target->fire < 1) return false;

    const Card payment = hand_count(Card::Grass) > 0 ? Card::Grass : Card::Fire;
"""
NEW = """    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || target->grass < 2 || target->fire < 1) return false;

    // Project the post-retreat board before paying any real resource. The existing
    // #1646 policy holds Blender when Professor Burnet is the cheaper live
    // current-turn payload outlet after a GGF VSTAR becomes Active.
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official attachment, retreat, Item, and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Existing Burnet-over-Blender priority: https://github.com/FlareZ123/pokemon-sims/issues/1646
    // K1, strict-JIT, DCI/AMR, Supporter contention, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    Engine projected = *this;
    Pokemon* projected_target = projected.best_benched_vstar_for_promotion();
    if (projected_target == nullptr) return false;
    projected.state_.manual_energy_used = true;
    std::swap(*projected.state_.active, *projected_target);
    if (projected.issue_1646_hold_blender_for_burnet_finish_visible()) return false;

    const Card payment = hand_count(Card::Grass) > 0 ? Card::Grass : Card::Fire;
"""


def main() -> int:
    descriptor = os.open(LOCK, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))
        text = PATH.read_text(encoding="utf-8")
        if text.count(OLD) != 1:
            raise RuntimeError("#2295 refinement anchor mismatch")
        updated = text.replace(OLD, NEW, 1)
        with tempfile.NamedTemporaryFile(
            mode="w", encoding="utf-8", newline="\n", dir=PATH.parent, delete=False
        ) as handle:
            handle.write(updated)
            temporary = Path(handle.name)
        os.replace(temporary, PATH)
    finally:
        os.close(descriptor)
        LOCK.unlink(missing_ok=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
