from pathlib import Path
import subprocess


def run(*args: str) -> None:
    subprocess.run(args, check=True)


run("git", "config", "user.name", "github-actions[bot]")
run("git", "config", "user.email", "41898282+github-actions[bot]@users.noreply.github.com")
run("git", "fetch", "origin", "main")
# Keep the already-reviewed DDE branch changes wherever Git can merge them normally.
# For genuine conflicts, current main wins. The conflicting generated provenance files
# are regenerated later in this validation job; the only overlapping hand-written
# DDE change is a source-citation comment restored explicitly below.
run("git", "merge", "-X", "theirs", "--no-edit", "origin/main")

source = Path("src/regidrago_sim.cpp")
text = source.read_text(encoding="utf-8")
if "// Double Dragon Energy:" not in text:
    anchor = "// Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163\n"
    if text.count(anchor) != 1:
        raise RuntimeError("DDE source-citation anchor changed during live-main merge")
    text = text.replace(
        anchor,
        anchor + "// Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ ; enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2238\n",
        1,
    )
    source.write_text(text, encoding="utf-8")

required = {
    "src/trace_engine_v2/part_000.inc": "DoubleDragonEnergy",
    "src/trace_engine_v2/part_004.inc": "pays_apex_energy_cost",
    "src/trace_engine_v2/part_013_legacy_star_override.inc": "DoubleDragonEnergy",
    "src/trace_engine_v2/part_celestial_roar_override.inc": "DoubleDragonEnergy",
    "tests/issue_2238_double_dragon_energy_tests.cpp": "test_dde_payment_combinations",
}
for filename, marker in required.items():
    if marker not in Path(filename).read_text(encoding="utf-8"):
        raise RuntimeError(f"live-main merge lost DDE marker {marker} in {filename}")

print("Merged latest origin/main into issue-2238 branch and preserved DDE implementation.")
