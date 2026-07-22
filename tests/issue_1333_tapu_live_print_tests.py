from __future__ import annotations

import importlib.util
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
AUDIT_SCRIPT = REPO_ROOT / "scripts" / "audit_card_data.py"
RULE_SOURCES = REPO_ROOT / "docs" / "RULE_SOURCES.md"
SUPPORT_URL = (
    "https://support.pokemon.com/hc/en-us/articles/"
    "6489934466708-Pok%C3%A9mon-TCG-Live-Migration-FAQ-from-the-"
    "Pok%C3%A9mon-TCG-Online"
)
CARD_URL = "https://api.pokemontcg.io/v2/cards/sm2-60"


def load_audit_module():
    spec = importlib.util.spec_from_file_location("issue_1333_audit", AUDIT_SCRIPT)
    if spec is None or spec.loader is None:
        raise AssertionError("Unable to load scripts/audit_card_data.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main() -> int:
    audit = load_audit_module()
    if audit.REQUESTED["Tapu Lele-GX"] != ("sm2-60", 2):
        raise AssertionError("The canonical TCG Live deck must use Tapu Lele-GX sm2-60.")

    old_id = "cel25c-" + "60_A"
    searchable_roots = [
        REPO_ROOT / "src",
        REPO_ROOT / "tests",
        REPO_ROOT / "docs",
        REPO_ROOT / "scripts",
        REPO_ROOT / "data",
    ]
    stale: list[str] = []
    for root in searchable_roots:
        if not root.exists():
            continue
        for path in root.rglob("*"):
            if not path.is_file() or path.suffix not in {
                ".cpp",
                ".inc",
                ".py",
                ".md",
                ".txt",
                ".json",
                ".cmake",
            }:
                continue
            if old_id in path.read_text(encoding="utf-8", errors="replace"):
                stale.append(str(path.relative_to(REPO_ROOT)))
    if stale:
        raise AssertionError("Stale Classic Collection source IDs: " + ", ".join(stale))

    audit_text = AUDIT_SCRIPT.read_text(encoding="utf-8")
    source_text = RULE_SOURCES.read_text(encoding="utf-8")
    for required in (SUPPORT_URL, CARD_URL):
        if required not in audit_text:
            raise AssertionError(f"Canonical audit is missing direct source: {required}")
        if required not in source_text:
            raise AssertionError(f"Rule source registry is missing direct source: {required}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
