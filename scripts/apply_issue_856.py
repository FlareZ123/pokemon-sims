from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path
from typing import Callable

ERIKA_URL = "https://api.pokemontcg.io/v2/cards/sv3pt5-160"
ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/856"


def atomic_transform(path: Path, transform: Callable[[str], str]) -> None:
    with path.open("r+", encoding="utf-8") as locked:
        fcntl.flock(locked.fileno(), fcntl.LOCK_EX)
        original = locked.read()
        updated = transform(original)
        if updated == original:
            return
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=path.parent, delete=False
        ) as temporary:
            temporary.write(updated)
            temporary.flush()
            os.fsync(temporary.fileno())
            temporary_path = Path(temporary.name)
        os.replace(temporary_path, path)


def replace_once(path: Path, old: str, new: str) -> None:
    def transform(text: str) -> str:
        if new in text:
            return text
        if text.count(old) != 1:
            raise SystemExit(f"Unable to locate unique insertion point in {path}")
        return text.replace(old, new, 1)

    atomic_transform(path, transform)


replace_once(
    Path("src/regidrago_sim.cpp"),
    "// Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76\n"
    "// Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146\n",
    "// Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76\n"
    f"// Erika's Invitation: {ERIKA_URL}\n"
    f"// Confirmed source-registry fix: {ISSUE_URL}\n"
    "// Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146\n",
)
replace_once(
    Path("docs/RULE_SOURCES.md"),
    "| Latias ex — Skyliner | https://api.pokemontcg.io/v2/cards/sv8-76 |\n"
    "| Hisuian Heavy Ball | https://api.pokemontcg.io/v2/cards/swsh10-146 |\n",
    "| Latias ex — Skyliner | https://api.pokemontcg.io/v2/cards/sv8-76 |\n"
    f"| Erika's Invitation — opponent-dependent effect is deliberately inert in the single-player model | {ERIKA_URL} |\n"
    "| Hisuian Heavy Ball | https://api.pokemontcg.io/v2/cards/swsh10-146 |\n",
)


def update_contract(text: str) -> str:
    if "ERIKA_SOURCE_URL" in text:
        return text
    replacements = (
        (
            'DOC_PATH = REPO_ROOT / "docs" / "CARD_AUDIT.md"\n',
            'DOC_PATH = REPO_ROOT / "docs" / "CARD_AUDIT.md"\n'
            'RULE_SOURCES_PATH = REPO_ROOT / "docs" / "RULE_SOURCES.md"\n'
            'SOURCE_WRAPPER_PATH = REPO_ROOT / "src" / "regidrago_sim.cpp"\n',
        ),
        (
            'ARCHIVE_SHA256 = "3444c74e47cdb92d83ba760e9eeefa8bbaedd9d7f396068c0e1ed390a686af08"\n',
            'ARCHIVE_SHA256 = "3444c74e47cdb92d83ba760e9eeefa8bbaedd9d7f396068c0e1ed390a686af08"\n'
            f'ERIKA_SOURCE_URL = "{ERIKA_URL}"\n',
        ),
        (
            '    documented = DOC_PATH.read_text(encoding="utf-8")\n\n',
            '    documented = DOC_PATH.read_text(encoding="utf-8")\n'
            '    rule_sources = RULE_SOURCES_PATH.read_text(encoding="utf-8")\n'
            '    source_wrapper = SOURCE_WRAPPER_PATH.read_text(encoding="utf-8")\n\n'
            "    # Erika's printed opponent-hand condition is the source for its deliberately inert\n"
            "    # single-player treatment. Keep that exact print in both declared registries:\n"
            f"    # {ERIKA_URL}\n"
            f"    # {ISSUE_URL}\n"
            '    for registry_name, registry in (("docs/RULE_SOURCES.md", rule_sources),\n'
            '                                     ("src/regidrago_sim.cpp", source_wrapper)):\n'
            '        if ERIKA_SOURCE_URL not in registry:\n'
            '            raise AssertionError(f"{registry_name} must register Erika\'s Invitation exact print.")\n\n',
        ),
    )
    updated = text
    for old, new in replacements:
        if updated.count(old) != 1:
            raise SystemExit(f"Unable to locate unique contract insertion point: {old!r}")
        updated = updated.replace(old, new, 1)
    return updated


atomic_transform(Path("tests/card_audit_contract_tests.py"), update_contract)
