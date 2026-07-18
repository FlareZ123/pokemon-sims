from __future__ import annotations

import json
import re
import shutil
import subprocess
import sys
import tempfile
from pathlib import Path


REPO_ROOT = Path(__file__).resolve().parents[1]


def ctest_trace_cases(cmake_text: str) -> dict[str, tuple[int, int]]:
    cases: dict[str, tuple[int, int]] = {}
    pattern = re.compile(
        r"add_test\(NAME\s+\S+\s+COMMAND\s+regidrago_sim\s+--simulate-this\s+"
        r"--scenario\s+(\S+)\s+--seed\s+(\d+)\s+--require-ready-by\s+(\d+)\)"
    )
    for scenario, seed, deadline in pattern.findall(cmake_text):
        cases[scenario] = (int(seed), int(deadline))
    return cases




def documented_build_targets(markdown: str) -> set[str]:
    return set(re.findall(r"\./build/([A-Za-z0-9_-]+)", markdown))


def cmake_executable_targets(cmake_text: str) -> set[str]:
    return set(re.findall(r"add_executable\(\s*([A-Za-z0-9_-]+)", cmake_text))


def assert_documented_build_commands_exist() -> None:
    # Every documented ./build command must name a current CMake executable target:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt
    # https://github.com/FlareZ123/pokemon-sims/issues/917
    cmake_text = (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    targets = cmake_executable_targets(cmake_text)
    referenced: set[str] = set()
    for path in [REPO_ROOT / "README.md", *(REPO_ROOT / "docs").rglob("*.md")]:
        referenced.update(documented_build_targets(path.read_text(encoding="utf-8")))
    missing = sorted(referenced - targets)
    if missing:
        raise AssertionError(f"Documented build commands name absent targets: {missing}")


def assert_same_repository_line_anchors_resolve() -> None:
    # Direct source links must not request lines beyond their current repository files:
    # https://github.com/FlareZ123/pokemon-sims
    # https://github.com/FlareZ123/pokemon-sims/issues/917
    pattern = re.compile(
        r"https://github\.com/FlareZ123/pokemon-sims/blob/main/([^#\s)]+)#L(\d+)(?:-L(\d+))?"
    )
    suffixes = {".md", ".py", ".cpp", ".inc", ".cmake", ".txt", ".yml", ".yaml"}
    failures: list[str] = []
    for path in REPO_ROOT.rglob("*"):
        if not path.is_file() or path.suffix not in suffixes:
            continue
        if any(part in {".git", "build", "build-sanitizers"} for part in path.parts):
            continue
        text = path.read_text(encoding="utf-8", errors="replace")
        for match in pattern.finditer(text):
            target = REPO_ROOT / match.group(1)
            requested = max(int(match.group(2)), int(match.group(3) or match.group(2)))
            if not target.is_file():
                failures.append(f"{path.relative_to(REPO_ROOT)}: missing {match.group(1)}")
                continue
            line_count = len(target.read_text(encoding="utf-8", errors="replace").splitlines())
            if requested > line_count:
                failures.append(
                    f"{path.relative_to(REPO_ROOT)}: {match.group(1)} requests L{requested}, EOF L{line_count}"
                )
    if failures:
        raise AssertionError("Broken same-repository line anchors: " + "; ".join(failures))


def main() -> int:
    assert_documented_build_commands_exist()
    assert_same_repository_line_anchors_resolve()
    manifest = json.loads(
        (REPO_ROOT / "results" / "baseline_manifest.json").read_text(encoding="utf-8")
    )
    ctest_cases = ctest_trace_cases(
        (REPO_ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
    )
    mismatches = [
        entry
        for entry in manifest["traces"]
        if entry["scenario"] in ctest_cases
        and ctest_cases[entry["scenario"]] != (entry["seed"], entry["deadline"])
    ]
    if not mismatches:
        raise AssertionError("Fixture requires at least one independent manifest/CTest case.")

    with tempfile.TemporaryDirectory() as temporary_directory:
        temporary_root = Path(temporary_directory)
        for relative in (
            Path("README.md"),
            Path("results/baseline_manifest.json"),
            Path("results/simulation_results.csv"),
        ):
            destination = temporary_root / relative
            destination.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(REPO_ROOT / relative, destination)
        shutil.copytree(
            REPO_ROOT / "results" / "traces",
            temporary_root / "results" / "traces",
        )

        # The generated audit must preserve the boundary between saved examples and
        # separately registered CTest cases when their seeds differ:
        # https://github.com/FlareZ123/pokemon-sims/blob/main/results/baseline_manifest.json
        # https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt
        subprocess.run(
            [
                sys.executable,
                str(REPO_ROOT / "scripts" / "update_setup_docs.py"),
                "--repo-root",
                str(temporary_root),
            ],
            check=True,
        )
        audit = (temporary_root / "docs" / "TRACE_AUDIT.md").read_text(
            encoding="utf-8"
        )

    false_claim = (
        "the CTest deadline regression uses the same scenario, seed, and ready turn"
    )
    if false_claim in audit:
        raise AssertionError("Generated audit equates independent trace and CTest cases.")
    if "The saved traces are independent audit examples." not in audit:
        raise AssertionError("Generated audit omits the independent-example boundary.")
    expected_rows = len(manifest["traces"])
    actual_rows = audit.count("Saved executable audit example reproduced from the manifest.")
    if actual_rows != expected_rows:
        raise AssertionError(
            f"Expected {expected_rows} manifest-only validation rows, found {actual_rows}."
        )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
