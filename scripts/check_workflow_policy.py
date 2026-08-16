from __future__ import annotations

import re
from pathlib import Path

from baseline_provenance import simulator_policy_source_digest


ROOT = Path(__file__).resolve().parents[1]
WORKFLOW_DIR = ROOT / ".github" / "workflows"
ALLOWLIST_PATH = ROOT / "docs" / "PERMANENT_WORKFLOWS.txt"


def load_allowlist(path: Path = ALLOWLIST_PATH) -> set[str]:
    names = {
        line.strip()
        for line in path.read_text(encoding="utf-8").splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    }
    invalid = sorted(
        name
        for name in names
        if Path(name).name != name or Path(name).suffix not in {".yml", ".yaml"}
    )
    if invalid:
        raise ValueError(
            "Invalid permanent-workflow allowlist entries: " + ", ".join(invalid)
        )
    return names


def installed_workflows(workflow_dir: Path = WORKFLOW_DIR) -> set[str]:
    return {
        path.name
        for pattern in ("*.yml", "*.yaml")
        for path in workflow_dir.glob(pattern)
    }


def workflow_policy_errors(
    workflow_dir: Path = WORKFLOW_DIR,
    allowlist_path: Path = ALLOWLIST_PATH,
) -> list[str]:
    allowlisted = load_allowlist(allowlist_path)
    installed = installed_workflows(workflow_dir)
    errors: list[str] = []

    # Pull-request workflows installed on the base branch can run for every matching
    # PR, so the default branch owns an explicit inventory instead of relying on a
    # filename convention that one-off agent workflows can bypass:
    # https://docs.github.com/en/actions/using-workflows/events-that-trigger-workflows#pull_request
    # Repository incident and cleanup contract:
    # https://github.com/FlareZ123/pokemon-sims/issues/1360
    unapproved = sorted(installed - allowlisted)
    missing = sorted(allowlisted - installed)
    if unapproved:
        errors.append("Unapproved permanent workflows: " + ", ".join(unapproved))
    if missing:
        errors.append("Stale permanent-workflow allowlist entries: " + ", ".join(missing))

    for name in sorted(installed & allowlisted):
        text = (workflow_dir / name).read_text(encoding="utf-8")

        # Permanent validation workflows are intentionally read-only. A future
        # workflow that genuinely needs write access should require an explicit
        # policy change in the same reviewed PR instead of inheriting an agent's
        # recovery permissions:
        # https://docs.github.com/en/actions/security-for-github-actions/security-guides/automatic-token-authentication
        if re.search(r"(?m)^\s*contents:\s*write\s*$", text):
            errors.append(f"{name}: permanent workflows must not grant contents: write")

        # Branch-specific recovery logic belongs on the temporary branch that owns
        # it. Permanent workflows should express behavior through event filters or
        # stable repository state, never one historical contributor head branch:
        # https://docs.github.com/en/actions/using-workflows/events-that-trigger-workflows#pull_request
        if re.search(r"github\.head_ref\s*==", text):
            errors.append(f"{name}: hard-coded github.head_ref equality is forbidden")

    return errors


def main() -> int:
    print(f"SIMULATOR_POLICY_SOURCE_SHA256={simulator_policy_source_digest(ROOT)}")
    errors = workflow_policy_errors()
    if not errors:
        print("Workflow policy satisfied.")
        return 0

    for error in errors:
        print(error)
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
