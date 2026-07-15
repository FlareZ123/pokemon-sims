from __future__ import annotations

import re
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]
DOC_PATH = REPO_ROOT / "docs" / "MODEL_ASSUMPTIONS.md"
MARKDOWN_FILE_REFERENCE = re.compile(r"`([^`\n]+\.md)`")


def missing_references(document: str) -> list[str]:
    missing = []
    for reference in MARKDOWN_FILE_REFERENCE.findall(document):
        candidate = (DOC_PATH.parent / reference).resolve()
        try:
            candidate.relative_to(REPO_ROOT.resolve())
        except ValueError as error:
            raise AssertionError(
                f"Markdown file reference escapes the repository: {reference}"
            ) from error
        if not candidate.is_file():
            missing.append(reference)
    return missing


def main() -> int:
    documented = DOC_PATH.read_text(encoding="utf-8")

    # MODEL_ASSUMPTIONS is a repository specification, so every backtick Markdown
    # file reference in it must resolve to a tracked repository file:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md
    missing = missing_references(documented)
    if missing:
        raise AssertionError(
            f"MODEL_ASSUMPTIONS.md has missing Markdown file references: {missing}."
        )

    # Preserve coverage for the exact failure mode fixed by issue #605:
    # https://github.com/FlareZ123/pokemon-sims/issues/605
    if missing_references("planned in `SIM-PLAN.md`.") != ["SIM-PLAN.md"]:
        raise AssertionError("The contract must detect a missing backtick Markdown reference.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
