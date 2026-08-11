from __future__ import annotations

import os
import shutil
import subprocess
import tempfile
from pathlib import Path

REPO_ROOT = Path(__file__).resolve().parents[1]


# This experiment deliberately patches only a temporary source copy. The canonical
# TCG Live engine and registered regidrago-shell recipe remain untouched because
# Double Dragon Energy is an XY card currently unavailable in Pokémon TCG Live:
# https://support.pokemon.com/hc/en-us/articles/6489934466708-Pok%C3%A9mon-TCG-Live-Migration-FAQ-from-the-Pok%C3%A9mon-TCG-Online
# Paper DDE card text: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
# Existing paper-only mechanics contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
# Existing Live/paper boundary: https://github.com/FlareZ123/pokemon-sims/issues/2332

def patch_steven_dde_targeting(source: str) -> str:
    insertion_needle = (
        "    } // DDE-only is one physical Basic attachment short: "
        "https://github.com/FlareZ123/pokemon-sims/issues/2424\n"
        "    const bool held_treasure_crispin_grass_route =\n"
    )
    insertion = (
        "    } // DDE-only is one physical Basic attachment short: "
        "https://github.com/FlareZ123/pokemon-sims/issues/2424\n"
        "    const bool steven_dde_completes_next_turn = [&]() {\n"
        "      if (!need_energy() || projected_regi == nullptr ||\n"
        "          (hand_count(Card::DoubleDragonEnergy) == 0 &&\n"
        "           deck_count_after_search_started(Card::DoubleDragonEnergy) == 0)) {\n"
        "        return false;\n"
        "      }\n"
        "      Pokemon projected = *projected_regi;\n"
        "      return attach_energy_card(projected, Card::DoubleDragonEnergy) &&\n"
        "          pays_apex_energy_cost(projected);\n"
        "    }();\n"
        "    if (steven_dde_completes_next_turn &&\n"
        "        hand_count(Card::DoubleDragonEnergy) == 0) {\n"
        "      // Steven searches any three cards. When one DDE is the one-card\n"
        "      // next-turn Energy completion, reserve it ahead of Crispin so the\n"
        "      // following Supporter action remains available for Burnet or Tate.\n"
        "      // Steven: https://api.pokemontcg.io/v2/cards/sm7-145\n"
        "      // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/\n"
        "      // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136\n"
        "      // DDE model: https://github.com/FlareZ123/pokemon-sims/issues/2238\n"
        "      // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n"
        "      add_wanted(Card::DoubleDragonEnergy);\n"
        "    }\n"
        "    const bool held_treasure_crispin_grass_route =\n"
    )
    if source.count(insertion_needle) != 1:
        raise RuntimeError("Steven DDE insertion anchor changed")
    source = source.replace(insertion_needle, insertion, 1)

    crispin_needle = (
        "    if (!held_routes_complete_next_turn && need_energy() &&\n"
        "        hand_count(Card::Crispin) == 0 && crispin_can_advance_energy_axis() &&\n"
    )
    crispin_replacement = (
        "    if (!held_routes_complete_next_turn && !steven_dde_completes_next_turn &&\n"
        "        need_energy() && hand_count(Card::Crispin) == 0 &&\n"
        "        crispin_can_advance_energy_axis() &&\n"
    )
    if source.count(crispin_needle) != 1:
        raise RuntimeError("Steven Crispin-selection anchor changed")
    return source.replace(crispin_needle, crispin_replacement, 1)


def compile_cpp(compiler: str, source: Path, output: Path, include_root: Path) -> None:
    subprocess.run(
        [compiler, "-std=c++20", "-O2", "-I", str(include_root), str(source), "-o", str(output)],
        cwd=include_root,
        check=True,
    )


def main() -> int:
    compiler = os.environ.get("CXX", "c++")
    with tempfile.TemporaryDirectory(prefix="dde-2grass-") as directory:
        temp_root = Path(directory)
        shutil.copytree(REPO_ROOT / "src", temp_root / "src")
        (temp_root / "tests").mkdir()
        (temp_root / "experiments").mkdir()
        shutil.copy2(
            REPO_ROOT / "tests" / "experiment_dde_steven_route_tests.cpp",
            temp_root / "tests" / "experiment_dde_steven_route_tests.cpp",
        )
        shutil.copy2(
            REPO_ROOT / "experiments" / "dde_2grass_swap.cpp",
            temp_root / "experiments" / "dde_2grass_swap.cpp",
        )

        steven_source = temp_root / "src" / "trace_engine_v2" / "part_011.inc"
        original = steven_source.read_text(encoding="utf-8")
        steven_source.write_text(
            patch_steven_dde_targeting(original), encoding="utf-8", newline=""
        )

        test_binary = temp_root / "experiment-dde-steven-test"
        experiment_binary = temp_root / "experiment-dde-2grass"
        compile_cpp(
            compiler,
            temp_root / "tests" / "experiment_dde_steven_route_tests.cpp",
            test_binary,
            temp_root,
        )
        subprocess.run([str(test_binary)], cwd=REPO_ROOT, check=True)

        compile_cpp(
            compiler,
            temp_root / "experiments" / "dde_2grass_swap.cpp",
            experiment_binary,
            temp_root,
        )
        subprocess.run([str(experiment_binary)], cwd=REPO_ROOT, check=True)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
