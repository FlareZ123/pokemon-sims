from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
TEMP_HELPERS = [
    "scripts/apply_issue_2238_dde.py",
    "scripts/fix_issue_2238_active_celestial.py",
    "scripts/fix_issue_2238_baseline_compat.py",
    "scripts/fix_issue_2238_blender_energy.py",
    "scripts/fix_issue_2238_bootstrap.py",
    "scripts/fix_issue_2238_issue1795.py",
    "scripts/fix_issue_2238_issue1795_test_fixture.py",
    "scripts/fix_issue_2238_issue1796.py",
]

# This is the last bootstrap helper in the validated chain. Remove every temporary
# issue-2238 helper before running the permanent source audits so their embedded
# card-source URLs cannot contaminate the repository card-audit surface.
for relative in TEMP_HELPERS:
    path = ROOT / relative
    if path.exists():
        path.unlink()

subprocess.run(
    [sys.executable, str(ROOT / "tests/card_audit_contract_tests.py")],
    cwd=ROOT,
    check=True,
)
subprocess.run(
    [sys.executable, str(ROOT / "tests/card_audit_category_contract_tests.py")],
    cwd=ROOT,
    check=True,
)

subprocess.run(["git", "add", "-u", "scripts"], cwd=ROOT, check=True)
if subprocess.run(["git", "diff", "--cached", "--quiet"], cwd=ROOT).returncode != 0:
    subprocess.run(
        ["git", "commit", "-m", "Remove temporary DDE validation helpers (#2238)"],
        cwd=ROOT,
        check=True,
    )
print("Issue-2238 temporary helpers removed; permanent card audits passed.")
