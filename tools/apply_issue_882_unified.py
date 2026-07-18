from __future__ import annotations

import importlib.util
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LEGACY_PATH = ROOT / "tools" / "apply_issue_882.py"

spec = importlib.util.spec_from_file_location("apply_issue_882_legacy", LEGACY_PATH)
if spec is None or spec.loader is None:
    raise SystemExit("Unable to load the issue 882 applicator")
legacy = importlib.util.module_from_spec(spec)
spec.loader.exec_module(legacy)

original_replace_once = legacy.replace_once
CURRENT_ERIKA_CYCLE = """        // Serena may discard up to three cards before drawing to five. Erika's
        // Invitation is the canonical inert replacement for retired Mawile-GX in
        // matchup-flex cycling:
        // https://api.pokemontcg.io/v2/cards/swsh12-164
        // https://api.pokemontcg.io/v2/cards/sv3pt5-160
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
        // https://github.com/FlareZ123/pokemon-sims/issues/852
        const bool can_cycle = *extra == Card::Dipplin || *extra == Card::ErikasInvitation || *extra == Card::Guzma ||
"""
LEGACY_MAWILE_CYCLE = """        const bool can_cycle = *extra == Card::Dipplin || *extra == Card::MawileGX || *extra == Card::Guzma ||
"""


def unified_replace_once(path: Path, old: str, new: str) -> None:
    if path.name == "CMakeLists.txt":
        # Current main discovers each tests/*.cpp source through the unified runner:
        # https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt#L21-L47
        # https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py#L109-L139
        return
    if path.name == "part_012.inc":
        # Main replaced retired Mawile-GX with Erika's Invitation after this issue
        # branch was filed. Preserve that approved audit change while applying #882:
        # https://api.pokemontcg.io/v2/cards/sv3pt5-160
        # https://github.com/FlareZ123/pokemon-sims/issues/852
        old = old.replace(LEGACY_MAWILE_CYCLE, CURRENT_ERIKA_CYCLE)
        new = new.replace(LEGACY_MAWILE_CYCLE, CURRENT_ERIKA_CYCLE)
    original_replace_once(path, old, new)


legacy.replace_once = unified_replace_once
legacy.main()
