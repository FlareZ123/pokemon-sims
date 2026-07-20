from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def replace_once(path: str, old: str, new: str) -> None:
    target = ROOT / path
    text = target.read_text(encoding="utf-8")
    if old not in text:
        raise RuntimeError(f"Expected marker missing in {path}: {old[:80]!r}")
    target.write_text(text.replace(old, new, 1), encoding="utf-8")


def insert_before(path: str, marker: str, insertion: str) -> None:
    target = ROOT / path
    text = target.read_text(encoding="utf-8")
    if insertion.strip() in text:
        return
    if marker not in text:
        raise RuntimeError(f"Expected marker missing in {path}: {marker!r}")
    target.write_text(text.replace(marker, insertion + marker, 1), encoding="utf-8")


replace_once(
    "src/regidrago_sim.cpp",
    "// Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63\n",
    "// Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63\n"
    "// Dawn: https://api.pokemontcg.io/v2/cards/me2-87\n"
    "// Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117\n"
    "// Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1\n"
    "// Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2\n",
)

insert_before(
    "docs/CARD_AUDIT.md",
    "## Paper legality versus Pokémon TCG Live availability\n",
    """### Test-only Dawn, Forest of Vitality, Pineco, and Forretress ex support

The simulator also supports a temporary variant recipe containing Dawn `me2-87`, Forest of Vitality `me1-117`, Pineco `sv4pt5-1`, and Forretress ex `sv4pt5-2`. The recipe remains outside `baseline_recipe()`, the CLI aggregate scenario matrix, and committed baseline results. Dawn's Basic, Stage 1, and Stage 2 searches are independent optional selections. Forest of Vitality preserves the first-turn evolution prohibition and allows Pineco to evolve into Grass-type Forretress ex on its entry turn from turn two onward.

Direct card sources: https://api.pokemontcg.io/v2/cards/me2-87 https://api.pokemontcg.io/v2/cards/me1-117 https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2 https://github.com/FlareZ123/pokemon-sims/issues/1096

""",
)

replace_once(
    "docs/RULES_TRACEABILITY.md",
    "| `R-GAME-EVOLUTION` | `evolve_forretress_ex` | Pineco may evolve into Forretress ex only after the first turn and only when Pineco entered play on an earlier turn: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2 https://www.pokemon.com/us/pokemon-tcg/rules |",
    "| `R-GAME-EVOLUTION` | `evolve_forretress_ex` | Pineco follows ordinary prior-turn evolution timing. Forest of Vitality permits Grass-type Pineco to evolve into Grass-type Forretress ex during its entry turn after the first turn: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2 https://api.pokemontcg.io/v2/cards/me1-117 https://www.pokemon.com/us/pokemon-tcg/rules |",
)

rules_path = ROOT / "docs" / "RULES_TRACEABILITY.md"
rules_text = rules_path.read_text(encoding="utf-8")
if "| `R-DAWN-01` |" not in rules_text:
    lines = rules_text.splitlines()
    for index, line in enumerate(lines):
        if line.startswith("| `R-FORRETRESS-01` |"):
            lines[index] = "| `R-FORRETRESS-01` | Pineco `sv4pt5-1` evolves into Forretress ex `sv4pt5-2`; **Exploding Energy** searches up to five Basic Grass Energy, attaches them in any distribution, shuffles, then Knocks Out Forretress ex when the deck was searched. | `bench_pineco_if_useful`, `evolve_forretress_ex`, `resolve_exploding_energy`, `use_exploding_energy_for_setup` | The Ability is once during the turn, supports zero through five selected Grass Energy, performs legal arbitrary distribution, creates K1 deck knowledge, shuffles, and self-Knocks Out. The test-only recipe remains outside canonical results: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2 https://www.pokemon.com/us/pokemon-tcg/rules https://github.com/FlareZ123/pokemon-sims/issues/972 |"
            lines[index + 1:index + 1] = [
                "| `R-DAWN-01` | Dawn `me2-87`: search for up to one Basic Pokémon, one Stage 1 Pokémon, and one Stage 2 Pokémon, reveal them, put them into hand, then shuffle. | `choose_supporter_after_search_started`, `advance_forretress_combo` | Each category is independently optional. The test-only selector prefers Pineco, Forretress ex, and a modeled Stage 2 Dragon when that route is live: https://api.pokemontcg.io/v2/cards/me2-87 https://github.com/FlareZ123/pokemon-sims/issues/1096 |",
                "| `R-FOREST-VITALITY-01` | Forest of Vitality `me1-117`: Grass Pokémon may evolve into Grass Pokémon during the turn they were played, except on the first turn. | `advance_forretress_combo`, `evolve_forretress_ex` | It obeys one-Stadium-per-turn and same-name restrictions, resolves Chaotic Swell replacement, and can replace Path before Rule Box entry Abilities resolve: https://api.pokemontcg.io/v2/cards/me1-117 https://www.pokemon.com/us/pokemon-tcg/rules https://github.com/FlareZ123/pokemon-sims/issues/1096 |",
            ]
            break
    else:
        raise RuntimeError("R-FORRETRESS-01 row missing")
    rules_path.write_text("\n".join(lines) + "\n", encoding="utf-8")

sources_path = ROOT / "docs" / "RULE_SOURCES.md"
sources_text = sources_path.read_text(encoding="utf-8")
if "| Dawn | https://api.pokemontcg.io/v2/cards/me2-87 |" not in sources_text:
    marker = "| Powerglass | https://api.pokemontcg.io/v2/cards/sv6pt5-63 |\n"
    insertion = marker + (
        "| Dawn | https://api.pokemontcg.io/v2/cards/me2-87 |\n"
        "| Forest of Vitality | https://api.pokemontcg.io/v2/cards/me1-117 |\n"
        "| Pineco | https://api.pokemontcg.io/v2/cards/sv4pt5-1 |\n"
        "| Forretress ex — Exploding Energy | https://api.pokemontcg.io/v2/cards/sv4pt5-2 |\n"
    )
    if marker not in sources_text:
        raise RuntimeError("Powerglass source row missing")
    sources_path.write_text(sources_text.replace(marker, insertion, 1), encoding="utf-8")

print("Updated issue #1096 direct-source documentation.")
