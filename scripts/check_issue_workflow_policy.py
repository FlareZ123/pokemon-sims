from __future__ import annotations

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]
WORKFLOW_DIR = ROOT / ".github" / "workflows"
ALLOWLIST_PATH = ROOT / "docs" / "PERMANENT_ISSUE_WORKFLOWS.txt"


def load_allowlist() -> set[str]:
    names = {
        line.strip()
        for line in ALLOWLIST_PATH.read_text(encoding="utf-8").splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    }
    invalid = sorted(
        name
        for name in names
        if Path(name).name != name
        or not name.startswith("issue-")
        or Path(name).suffix not in {".yml", ".yaml"}
    )
    if invalid:
        raise ValueError(
            "Invalid permanent issue-workflow allowlist entries: " + ", ".join(invalid)
        )
    return names


def main() -> int:
    allowlisted = load_allowlist()

    # Pull-request workflows installed on the base branch can run for every matching PR,
    # so completed issue-specific files must be absent unless explicitly permanent:
    # https://docs.github.com/en/actions/using-workflows/events-that-trigger-workflows#pull_request
    # Write-capable GITHUB_TOKEN permissions can mutate repository content and PR branches:
    # https://docs.github.com/en/actions/security-for-github-actions/security-guides/automatic-token-authentication
    # Repository incident and required cleanup contract:
    # https://github.com/FlareZ123/pokemon-sims/issues/1360
    installed = {
        path.name
        for pattern in ("issue-*.yml", "issue-*.yaml")
        for path in WORKFLOW_DIR.glob(pattern)
    }

    unapproved = sorted(installed - allowlisted)
    missing = sorted(allowlisted - installed)
    if not unapproved and not missing:
        print("Issue workflow policy satisfied.")
        return 0

    if unapproved:
        print("Unapproved issue-specific workflows: " + ", ".join(unapproved))
    if missing:
        print("Stale permanent-workflow allowlist entries: " + ", ".join(missing))
    return 1


if __name__ == "__main__":
    raise SystemExit(main())
