from __future__ import annotations

import os
from pathlib import Path

FSS = Path("src/trace_engine_v2/part_010_fss_override.inc")
QB = Path("src/trace_engine_v2/part_009b1.inc")


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


def main() -> int:
    fss = FSS.read_text(encoding="utf-8")
    fss = replace_once(
        fss,
        "    const Pokemon* regi = target_regi();\n"
        "    // Star Alchemy takes one Grass while held Crispin takes a second Grass and Fire.\n",
        "    // Star Alchemy takes one Grass while held Crispin takes a second Grass and Fire.\n",
        "remove FSS policy-target dependency",
    )
    fss = replace_once(
        fss,
        "    return regi != nullptr && regi->card == Card::RegidragoV &&\n"
        "        regi->entered_turn == state_.turn && regi->grass == 0 && regi->fire == 0 &&\n"
        "        state_.active->card != Card::RegidragoV &&\n"
        "        state_.active->card != Card::RegidragoVstar;",
        "    return state_.active->card != Card::RegidragoV &&\n"
        "        state_.active->card != Card::RegidragoVstar &&\n"
        "        std::any_of(state_.bench.begin(), state_.bench.end(),\n"
        "                    [this](const Pokemon& pokemon) {\n"
        "                      return pokemon.card == Card::RegidragoV &&\n"
        "                          pokemon.entered_turn == state_.turn &&\n"
        "                          pokemon.grass == 0 && pokemon.fire == 0;\n"
        "                    });",
        "FSS public Bench route",
    )
    atomic_write(FSS, fss)

    qb = QB.read_text(encoding="utf-8")
    qb = replace_once(
        qb,
        "    const Pokemon* regi = target_regi();\n"
        "    if (regi == nullptr || regi->card != Card::RegidragoV ||\n"
        "        regi->entered_turn != state_.turn ||\n"
        "        state_.active->card == Card::RegidragoV ||\n"
        "        state_.active->card == Card::RegidragoVstar) {\n"
        "      return false;\n"
        "    }\n\n"
        "    const bool fire_finishes = regi->grass >= 2 && regi->fire == 0 &&\n"
        "        hand_count(Card::Fire) > 0;\n"
        "    const bool grass_finishes = regi->grass >= 1 && regi->fire >= 1 &&\n"
        "        hand_count(Card::Grass) > 0;",
        "    if (state_.active->card == Card::RegidragoV ||\n"
        "        state_.active->card == Card::RegidragoVstar) {\n"
        "      return false;\n"
        "    }\n"
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
        "Quick Ball public Bench route",
    )
    atomic_write(QB, qb)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
