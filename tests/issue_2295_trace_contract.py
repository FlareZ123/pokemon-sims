from __future__ import annotations

import subprocess
import sys
from pathlib import Path

SIMULATOR = Path(sys.argv[1]).resolve()
BLENDER_TRACE = "| PLAY ITEM | rules: R-BLENDER-01"
PAID_RETREAT_TRACE = (
    "| RETREAT | rules: R-GAME-RETREAT | Paid the one-Energy Basic Active "
    "Retreat Cost and promoted the Apex-ready Regidrago VSTAR."
)

# The route may use one manual attachment, one retreat, then the held ACE SPEC Item.
# A copied policy projection must stay invisible in the public trace so it cannot
# manufacture a second apparent Brilliant Blender play:
# Official turn / retreat / Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
# Confirmed route bug: https://github.com/FlareZ123/pokemon-sims/issues/2295
for seed in (29, 86):
    result = subprocess.run(
        [
            str(SIMULATOR),
            "--simulate-this",
            "--deck",
            "regidrago-shell",
            "--scenario",
            "strict-jit/go-first",
            "--seed",
            str(seed),
            "--require-ready-by",
            "4",
        ],
        check=False,
        capture_output=True,
        text=True,
    )
    if result.returncode != 0:
        raise SystemExit(
            f"seed {seed} failed with exit {result.returncode}\n{result.stdout}\n{result.stderr}"
        )

    blender_count = result.stdout.count(BLENDER_TRACE)
    if blender_count != 1:
        raise SystemExit(
            f"seed {seed} emitted {blender_count} Blender trace entries; expected exactly 1\n"
            f"{result.stdout}"
        )

    retreat_at = result.stdout.find(PAID_RETREAT_TRACE)
    blender_at = result.stdout.find(BLENDER_TRACE)
    if retreat_at < 0 or blender_at <= retreat_at:
        raise SystemExit(
            f"seed {seed} did not trace paid retreat before the real Blender play\n"
            f"{result.stdout}"
        )
