from __future__ import annotations

import importlib.util
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DOC_PATH = REPO_ROOT / "docs" / "CARD_AUDIT.md"
SCRIPT_PATH = REPO_ROOT / "scripts" / "audit_card_data.py"
UPSTREAM_COMMIT_URL = (
    "https://github.com/PokemonTCG/pokemon-tcg-data/commit/"
    "0af6250a22495e4a3e9f60ff45fc3fedc2e0563d"
)
ARCHIVE_SHA256 = "3444c74e47cdb92d83ba760e9eeefa8bbaedd9d7f396068c0e1ed390a686af08"


def load_audit_module():
    spec = importlib.util.spec_from_file_location("audit_card_data", SCRIPT_PATH)
    if spec is None or spec.loader is None:
        raise AssertionError("Unable to load scripts/audit_card_data.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module


def main() -> int:
    audit = load_audit_module()
    documented = DOC_PATH.read_text(encoding="utf-8")

    if sum(copies for _, copies in audit.REQUESTED.values()) != 60:
        raise AssertionError("The audit request no longer represents a 60-card deck.")

    # The raw JSON is a local reproduction artifact because its source field records the
    # caller-provided path. Documentation must not claim that file is tracked:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/scripts/audit_card_data.py#L135-L166
    if "intentionally untracked" not in documented:
        raise AssertionError("CARD_AUDIT.md must state that data/card_audit.json is untracked.")
    if "The reproducible raw audit is `data/card_audit.json`" in documented:
        raise AssertionError("CARD_AUDIT.md still claims that the raw audit is tracked.")

    # Pin the exact retrievable upstream snapshot and the supplied archive digest so the
    # accepted evidence source cannot silently drift:
    # https://github.com/PokemonTCG/pokemon-tcg-data/commit/0af6250a22495e4a3e9f60ff45fc3fedc2e0563d
    if UPSTREAM_COMMIT_URL not in documented:
        raise AssertionError("CARD_AUDIT.md must pin the accepted upstream commit.")
    if ARCHIVE_SHA256 not in documented:
        raise AssertionError("CARD_AUDIT.md must pin the accepted archive SHA-256.")

    if r"--out data\card_audit.json" not in documented:
        raise AssertionError("CARD_AUDIT.md must retain the local reproduction command.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
