from __future__ import annotations

import subprocess
import tempfile
from pathlib import Path


REPOSITORY_ROOT = Path(__file__).resolve().parents[1]
WORKFLOW_PATH = REPOSITORY_ROOT / ".github/workflows/refresh-k1-stack-outputs.yml"
GENERATED_PREFIX = "Refresh source-bound matrices for "
EXPECTED_BRANCHES = [
    "fix/2070-oricorio-treasure-tapu",
    "fix/2072-wonder-tag-burnet-legacy-star",
    "fix/2073-fss-treasure-energy",
    "fix/2074-wonder-tag-complete-route",
    "fix/2076-arven-fss-blender-contention",
    "fix/2077-ultra-ball-post-payload-outlet",
    "fix/2078-one-discard-known-dead-costs",
    "fix/2079-evolution-incense-payload-outlet",
    "fix/2080-fss-k1-hold-targets",
    "fix/2081-late-steven-vstar-vessel",
    "fix/2082-wonder-tag-steven-fss",
    "fix/2083-late-steven-payload-treasure",
    "fix/2084-duplicate-held-arven",
    "fix/2085-late-steven-held-blender",
    "fix/2086-late-steven-held-burnet",
    "fix/2087-fss-latias-burnet",
    "fix/2088-fss-bank-tate",
    "fix/2089-arven-quick-ball-latias",
    "fix/2090-quick-ball-dialga-vessel",
    "fix/2091-blender-known-dead-costs",
    "fix/2092-arven-redundant-fss-regression",
    "fix/2093-rulebox-late-steven",
    "fix/2094-treasure-tapu-projection",
    "fix/2096-arven-powerglass",
    "fix/2097-fss-complete-quick-ball",
    "fix/2098-wonder-tag-redundant-crispin",
    "fix/2099-wonder-tag-gladion-held-energy",
    "fix/2100-wonder-tag-arven-held-burnet",
    "fix/2101-wonder-tag-steven-consistency",
    "fix/2102-late-steven-t3-compression",
    "fix/2103-wonder-tag-arven-payload",
]


def run(repo: Path, *args: str, capture: bool = False) -> str:
    completed = subprocess.run(
        ["git", *args],
        cwd=repo,
        check=True,
        text=True,
        capture_output=capture,
    )
    return completed.stdout.strip() if capture else ""


def commit_file(repo: Path, path: str, content: str, subject: str) -> str:
    target = repo / path
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(content, encoding="utf-8")
    run(repo, "add", path)
    run(repo, "commit", "-m", subject)
    return run(repo, "rev-parse", "HEAD", capture=True)


def assert_workflow_contract() -> None:
    workflow = WORKFLOW_PATH.read_text(encoding="utf-8")

    # The issue reproduction and required stack order are recorded here: https://github.com/FlareZ123/pokemon-sims/issues/2147
    positions = [workflow.index(f"            {branch}") for branch in EXPECTED_BRANCHES]
    assert positions == sorted(positions)
    assert "strategy:" not in workflow
    assert "matrix:" not in workflow

    # Revision-range semantics: https://git-scm.com/docs/gitrev-list
    assert 'git rev-list --reverse "$parent_original..$child_original"' in workflow
    assert 'parent_original="$child_original"' in workflow

    # Cherry-pick replays each issue-specific commit on the rebuilt parent: https://git-scm.com/docs/git-cherry-pick
    assert 'git cherry-pick "$commit"' in workflow
    assert f'if [[ "$subject" == "{GENERATED_PREFIX}"* ]]' in workflow

    # Exact leases prevent overwriting concurrent branch movement: https://git-scm.com/docs/git-push#Documentation/git-push.txt---force-with-leaseltrefnamegtltexpectgt
    assert 'git push --force-with-lease="refs/heads/$branch:$child_original_sha"' in workflow
    assert 'parent_head=$(git rev-parse HEAD)' in workflow


def assert_synthetic_stack_rebuild() -> None:
    with tempfile.TemporaryDirectory() as temporary_directory:
        repo = Path(temporary_directory)
        run(repo, "init", "-b", "main")
        run(repo, "config", "user.name", "Issue 2147 test")
        run(repo, "config", "user.email", "issue-2147@example.invalid")

        main_sha = commit_file(repo, "state.txt", "main\n", "main")
        original_refs: list[str] = []
        issue_heads: list[str] = []
        previous_issue_head = main_sha

        for index in range(1, 4):
            branch = f"stack-{index}"
            run(repo, "switch", "-c", branch, previous_issue_head)
            issue_head = commit_file(repo, f"issue-{index}.txt", f"issue {index}\n", f"issue {index}")
            issue_heads.append(issue_head)
            commit_file(repo, "generated.txt", f"stale {index}\n", f"{GENERATED_PREFIX}{branch}")
            original_ref = f"refs/original/{branch}"
            run(repo, "update-ref", original_ref, "HEAD")
            original_refs.append(original_ref)
            previous_issue_head = issue_head

        parent_original = main_sha
        parent_head = main_sha
        rebuilt_heads: list[str] = []

        for index, child_original in enumerate(original_refs, start=1):
            # The child range is measured against the original parent snapshot: https://git-scm.com/docs/gitrev-list
            commits = run(
                repo,
                "rev-list",
                "--reverse",
                f"{parent_original}..{child_original}",
                capture=True,
            ).splitlines()
            preserved = [
                commit
                for commit in commits
                if not run(repo, "show", "-s", "--format=%s", commit, capture=True).startswith(GENERATED_PREFIX)
            ]
            assert preserved == [issue_heads[index - 1]]

            run(repo, "switch", "--detach", parent_head)
            run(repo, "branch", "-f", "refresh-work", parent_head)
            run(repo, "switch", "refresh-work")
            for commit in preserved:
                run(repo, "cherry-pick", commit)
            commit_file(repo, "generated.txt", f"fresh {index}\n", f"{GENERATED_PREFIX}stack-{index}")

            parent_original = child_original
            parent_head = run(repo, "rev-parse", "HEAD", capture=True)
            rebuilt_heads.append(parent_head)

        for parent, child in zip(rebuilt_heads, rebuilt_heads[1:]):
            run(repo, "merge-base", "--is-ancestor", parent, child)

        run(repo, "switch", "--detach", rebuilt_heads[-1])
        for index in range(1, 4):
            assert (repo / f"issue-{index}.txt").read_text(encoding="utf-8") == f"issue {index}\n"


if __name__ == "__main__":
    assert_workflow_contract()
    assert_synthetic_stack_rebuild()
    print("issue 2147 workflow regression passed")
