from __future__ import annotations

import runpy
import subprocess
from pathlib import Path


WRAPPER = Path("scripts/run_issue_1393_fix.py")


def install_one_shot_pre_commit_hook() -> None:
    hook_dir = (Path.cwd() / ".git" / "hooks").resolve()
    hook_dir.mkdir(parents=True, exist_ok=True)
    hook = hook_dir / "pre-commit"
    hook.write_text(
        "#!/bin/sh\n"
        "set -eu\n"
        "git restore --source=HEAD --staged --worktree -- "
        ".github/workflows/issue-1393-crispin-before-gladion.yml "
        "scripts/apply_issue_1393_fix.py scripts/run_issue_1393_fix.py "
        "scripts/run_issue_1393_fix_v2.py\n"
        "rm -f .git/hooks/pre-commit\n",
        encoding="utf-8",
        newline="\n",
    )
    hook.chmod(0o755)
    subprocess.run(
        ["git", "config", "core.hooksPath", str(hook_dir)],
        check=True,
    )


def main() -> int:
    namespace = runpy.run_path(str(WRAPPER), run_name="issue_1393_wrapper")
    namespace["install_one_shot_post_commit_hook"] = install_one_shot_pre_commit_hook
    return int(namespace["main"]())


if __name__ == "__main__":
    raise SystemExit(main())
