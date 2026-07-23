from __future__ import annotations

import os
from pathlib import Path

FSS = Path("src/trace_engine_v2/part_010_fss_override.inc")
QB = Path("src/trace_engine_v2/part_009b1.inc")
TEST = Path("tests/issue_1185_quick_ball_strict_surplus_energy_tests.cpp")


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if new in text:
        return text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Unexpected {label} count: {count}")
    return text.replace(old, new, 1)


def patch_fss(text: str) -> str:
    text = replace_once(
        text,
        "\n    const Pokemon* regi = target_regi();\n"
        "    // Star Alchemy takes one Grass while held Crispin takes a second Grass and Fire.\n",
        "\n    // Star Alchemy takes one Grass while held Crispin takes a second Grass and Fire.\n",
        "unused FSS target",
    )
    return replace_once(
        text,
        "    // DCI and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n"
        "    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1403\n"
        "    // The generic missing-axis predicates project held connectors and therefore\n"
        "    // read false in this unevolved two-turn state. This route is admitted from\n"
        "    // the exact public board, hand, and K1 deck contents instead:\n"
        "    // K1 and decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n"
        "    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1403\n",
        "    // The generic missing-axis predicates project held connectors and therefore\n"
        "    // read false in this unevolved two-turn state. This route is admitted from\n"
        "    // the exact public board, hand, and K1 deck contents instead:\n"
        "    // DCI, K1, and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n"
        "    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1403\n",
        "FSS source comments",
    )


def patch_qb(text: str) -> str:
    text = replace_once(
        text,
        "    const auto regi = std::find_if(\n"
        "        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {\n"
        "          return pokemon.card == Card::RegidragoV &&\n"
        "              pokemon.entered_turn == state_.turn;\n"
        "        });\n"
        "    if (regi == state_.bench.end()) return false;\n\n"
        "    const bool fire_finishes = regi->grass >= 2 && regi->fire == 0 &&\n"
        "        hand_count(Card::Fire) > 0;\n"
        "    const bool grass_finishes = regi->grass >= 1 && regi->fire >= 1 &&\n"
        "        hand_count(Card::Grass) > 0;",
        "    const bool energy_route_finishes = std::any_of(\n"
        "        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {\n"
        "          if (pokemon.card != Card::RegidragoV ||\n"
        "              pokemon.entered_turn != state_.turn) {\n"
        "            return false;\n"
        "          }\n"
        "          const bool fire_finishes = pokemon.grass >= 2 && pokemon.fire == 0 &&\n"
        "              hand_count(Card::Fire) > 0;\n"
        "          const bool grass_finishes = pokemon.grass >= 1 && pokemon.fire >= 1 &&\n"
        "              hand_count(Card::Grass) > 0;\n"
        "          return fire_finishes || grass_finishes;\n"
        "        });",
        "Quick Ball multi-Regidrago scan",
    )
    text = replace_once(
        text,
        "    // the current-turn Dragon payload on T4. The generic Active-position and Energy\n",
        "    // the next-turn Dragon payload on T4. The generic Active-position and Energy\n",
        "Quick Ball timing comment",
    )
    return replace_once(
        text,
        "    return fire_finishes || grass_finishes;\n",
        "    return energy_route_finishes;\n",
        "Quick Ball route result",
    )


def patch_test(text: str) -> str:
    return text.replace("\n\n\nsim::State seed23_fss_state()", "\n\nsim::State seed23_fss_state()", 1).replace(
        "\n\n\nvoid test_seed23_route_admission_controls()",
        "\n\nvoid test_seed23_route_admission_controls()",
        1,
    )


def main() -> int:
    atomic_write(FSS, patch_fss(FSS.read_text(encoding="utf-8")))
    atomic_write(QB, patch_qb(QB.read_text(encoding="utf-8")))
    atomic_write(TEST, patch_test(TEST.read_text(encoding="utf-8")))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
