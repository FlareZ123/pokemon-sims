from __future__ import annotations

import tempfile
from pathlib import Path
import sys


sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.check_workflow_policy import workflow_policy_errors


def write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(text, encoding="utf-8")


def test_clean_inventory_passes() -> None:
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        workflows = root / "workflows"
        allowlist = root / "PERMANENT_WORKFLOWS.txt"
        write(allowlist, "ci.yml\nworkflow-policy.yml\n")
        write(workflows / "ci.yml", "permissions:\n  contents: read\n")
        write(workflows / "workflow-policy.yml", "permissions:\n  contents: read\n")
        assert workflow_policy_errors(workflows, allowlist) == []


def test_unapproved_workflow_is_rejected() -> None:
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        workflows = root / "workflows"
        allowlist = root / "PERMANENT_WORKFLOWS.txt"
        write(allowlist, "ci.yml\n")
        write(workflows / "ci.yml", "permissions:\n  contents: read\n")
        write(workflows / "agent-recovery.yml", "permissions:\n  contents: read\n")
        errors = workflow_policy_errors(workflows, allowlist)
        assert any("agent-recovery.yml" in error for error in errors)


def test_write_capable_permanent_workflow_is_rejected() -> None:
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        workflows = root / "workflows"
        allowlist = root / "PERMANENT_WORKFLOWS.txt"
        write(allowlist, "ci.yml\n")
        write(workflows / "ci.yml", "permissions:\n  contents: write\n")
        errors = workflow_policy_errors(workflows, allowlist)
        assert any("contents: write" in error for error in errors)


def test_hardcoded_head_branch_is_rejected() -> None:
    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        workflows = root / "workflows"
        allowlist = root / "PERMANENT_WORKFLOWS.txt"
        write(allowlist, "ci.yml\n")
        write(
            workflows / "ci.yml",
            "permissions:\n  contents: read\njobs:\n  test:\n    if: github.head_ref == 'fix/old-agent-branch'\n",
        )
        errors = workflow_policy_errors(workflows, allowlist)
        assert any("github.head_ref" in error for error in errors)


if __name__ == "__main__":
    test_clean_inventory_passes()
    test_unapproved_workflow_is_rejected()
    test_write_capable_permanent_workflow_is_rejected()
    test_hardcoded_head_branch_is_rejected()
