from __future__ import annotations

import os
import tempfile
from pathlib import Path


PATH = Path("src/trace_engine_v2/part_014c_latias_bench_override.inc")
LOCK = PATH.with_suffix(PATH.suffix + ".issue2295.lock")
FUNCTION_MARKER = "bool maybe_pay_basic_retreat_for_held_blender_finish()"
PAYMENT_ANCHOR = "    const Card payment = hand_count(Card::Grass) > 0 ? Card::Grass : Card::Fire;"
INSERTED_MARKER = "    const bool projected_burnet_priority ="
INSERTION = """    // Project the established #1646 Burnet-priority condition before paying any
    // real resource. Promotion makes target the Active GGF Regidrago VSTAR and
    // the paid retreat consumes this turn's manual attachment. Every other input
    // is already public or K1-known, so this prevents committing the paid route
    // when the simulator correctly prefers the cheaper live Burnet payload outlet.
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official attachment, retreat, Item, and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Existing Burnet-over-Blender priority: https://github.com/FlareZ123/pokemon-sims/issues/1646
    // K1, strict-JIT, DCI/AMR, Supporter contention, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    const bool projected_burnet_priority =
        scenario_.locks == LockMode::None && state_.turn == 3 &&
        supporter_allowed() && target->fire >= 1 && target->grass >= 2 &&
        hand_count(Card::ProfessorBurnet) > 0 &&
        count_of(state_.discard, Card::EarthenVessel) > 0 &&
        count_of(state_.discard, Card::QuickBall) > 0 &&
        count_of(state_.discarded_this_turn, Card::QuickBall) > 0 &&
        !payload_deck_candidates().empty();
    if (projected_burnet_priority) return false;

"""


def main() -> int:
    descriptor = os.open(LOCK, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))
        text = PATH.read_text(encoding="utf-8")
        function_start = text.find(FUNCTION_MARKER)
        if function_start < 0 or text.find(FUNCTION_MARKER, function_start + 1) >= 0:
            raise RuntimeError("#2295 helper function marker is missing or ambiguous")

        payment_index = text.find(PAYMENT_ANCHOR, function_start)
        if payment_index < 0:
            raise RuntimeError("#2295 payment anchor missing after helper marker")
        next_payment = text.find(PAYMENT_ANCHOR, payment_index + len(PAYMENT_ANCHOR))
        if next_payment >= 0:
            raise RuntimeError("#2295 payment anchor is ambiguous")

        inserted_index = text.find(INSERTED_MARKER, function_start, payment_index)
        if inserted_index >= 0:
            return 0

        updated = text[:payment_index] + INSERTION + text[payment_index:]
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
