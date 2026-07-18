from __future__ import annotations

import json
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def replace(relative_path: str, old: str, new: str, count: int = 1) -> None:
    path = ROOT / relative_path
    text = path.read_text(encoding="utf-8")
    if old not in text:
        raise RuntimeError(f"Missing issue-917 patch marker in {relative_path}: {old[:80]!r}")
    path.write_text(text.replace(old, new, count), encoding="utf-8")


def main() -> int:
    replace(
        "CMakeLists.txt",
        "add_test(NAME trace_turn2_item_lock_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit-turn2-item-lock/go-second --seed 1 --require-ready-by 3)\n"
        "add_test(NAME trace_no_control_first_t4 COMMAND regidrago_sim --simulate-this --scenario no-discard-control/go-first --seed 6 --require-ready-by 4)\n",
        "add_test(NAME trace_turn2_item_lock_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit-turn2-item-lock/go-second --seed 1 --require-ready-by 3)\n"
        "# The source-bound audit manifest includes this Rule Box Ability lock scenario in\n"
        "# the six-trace inventory, so CTest must enforce its ready-by deadline as well:\n"
        "# https://github.com/FlareZ123/pokemon-sims/blob/main/results/baseline_manifest.json\n"
        "# https://github.com/FlareZ123/pokemon-sims/issues/917\n"
        "add_test(NAME trace_rulebox_lock_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit-rulebox-ability-lock/go-second --seed 20 --require-ready-by 3)\n"
        "add_test(NAME trace_no_control_first_t4 COMMAND regidrago_sim --simulate-this --scenario no-discard-control/go-first --seed 6 --require-ready-by 4)\n",
    )

    replace(
        "README.md",
        "ctest --test-dir build --output-on-failure\n"
        "./build/regidrago_policy_tests\n"
        "./build/regidrago_tier2_tests\n",
        "ctest --test-dir build --output-on-failure\n"
        "./build/regidrago_unified_tests regidrago_policy_tests\n"
        "./build/regidrago_unified_tests regidrago_tier2_tests\n",
    )
    replace(
        "README.md",
        "`regidrago_policy_tests` checks core rule and connector legality. `regidrago_tier2_tests` checks comparison states, including",
        "The `regidrago_policy_tests` unified case checks core rule and connector legality. The `regidrago_tier2_tests` unified case checks comparison states, including",
    )

    replace(
        "docs/OPTIMAL_POLICY_FIXTURES.md",
        "`regidrago_policy_tests` executes **57** deterministic exact-state fixtures.",
        "`regidrago_unified_tests regidrago_policy_tests` executes **57** deterministic exact-state fixtures.",
    )
    replace(
        "docs/OPTIMAL_POLICY_FIXTURES.md",
        "./build/regidrago_policy_tests",
        "./build/regidrago_unified_tests regidrago_policy_tests",
    )
    replace(
        "docs/TIER2_POLICY_FIXTURES.md",
        "`regidrago_tier2_tests` executes **31** deterministic choice-differentiation, fast-compression, K1, and lock-aware fixtures.",
        "`regidrago_unified_tests regidrago_tier2_tests` executes **31** deterministic choice-differentiation, fast-compression, K1, and lock-aware fixtures.",
    )
    replace(
        "docs/TIER2_POLICY_FIXTURES.md",
        "./build/regidrago_tier2_tests",
        "./build/regidrago_unified_tests regidrago_tier2_tests",
    )
    replace(
        "docs/POLICY_DECISIONS.md",
        "- `regidrago_policy_tests`: 57 executable core exact-state rule and policy fixtures.",
        "- `regidrago_unified_tests regidrago_policy_tests`: 57 executable core exact-state rule and policy fixtures.",
    )
    replace(
        "docs/POLICY_DECISIONS.md",
        "- `regidrago_tier2_tests`: 31 executable choice-differentiation, fast-compression, K1, and lock-aware fixtures.",
        "- `regidrago_unified_tests regidrago_tier2_tests`: 31 executable choice-differentiation, fast-compression, K1, and lock-aware fixtures.",
    )

    simulator_audit = ROOT / "docs/SIMULATOR_AUDIT.md"
    text = simulator_audit.read_text(encoding="utf-8")
    text = text.replace(
        "`regidrago_policy_tests`, a 28-case exact-state fixture suite",
        "`regidrago_unified_tests regidrago_policy_tests`, a 57-case exact-state fixture suite",
    )
    text = text.replace("`main` remains unchanged.", "The current `main` branch contains these audited repairs.")
    text = text.replace(
        "Each matrix passed nine CTest targets: self-test, exact-state fixture suite, six deterministic traces, and aggregate smoke test.",
        "The current CI runs the complete unified CTest inventory, six deterministic trace regressions, the aggregate smoke test, and sanitizer coverage.",
    )
    simulator_audit.write_text(text, encoding="utf-8")

    (ROOT / "docs/AUDIT_STATUS.md").write_text(
        """# Audit Status

## Current source and test inventory

- Core exact-state policy fixtures: **57** in the `regidrago_unified_tests regidrago_policy_tests` case. The canonical runner table is https://github.com/FlareZ123/pokemon-sims/blob/main/tests/policy_fixture_v2/part_004a.inc#L134-L191
- Tier Two choice-differentiation fixtures: **31** in the `regidrago_unified_tests regidrago_tier2_tests` case. The canonical runner table is https://github.com/FlareZ123/pokemon-sims/blob/main/tests/tier2_parts/part_003b.inc#L40-L73
- Deterministic scenario trace regressions: six CTest cases registered in https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt
- Aggregate scenario smoke test: one CTest case registered in https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt

The detailed fixture names and executable evidence remain indexed in:

- Core policy index: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/OPTIMAL_POLICY_FIXTURES.md
- Tier Two policy index: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/TIER2_POLICY_FIXTURES.md
- Card-data provenance audit: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md

## Build and CI evidence

Pull requests run two compiled lanes from https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml

The Release lane builds the simulator and unified tests, runs three readable `--simulate-this` audits, generates the canonical 100,000-trial matrix, and then runs the complete CTest suite. It uploads the three traces, matrix, tested commit identifier, and CTest evidence directory.

The sanitizer lane builds with AddressSanitizer and UndefinedBehaviorSanitizer, runs the complete CTest suite, and uploads its CTest evidence directory. Card-audit and documentation contracts run inside both complete CTest suites after compilation.

## Remaining explicit model boundary

`FullRuleBoxAbility` is a scenario-level abstraction for a Path-to-the-Peak-style lock. Forest Seal Stone remains usable by an attached Pokémon V because the published ruling attributes Star Alchemy to the Tool rather than the Pokémon's own Ability: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
""",
        encoding="utf-8",
    )

    replace(
        "scripts/update_setup_docs.py",
        "https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt#L217-L223",
        "https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt",
    )
    replace(
        "tests/update_setup_docs_tests.py",
        "https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt#L217-L223",
        "https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt",
    )
    replace(
        "src/regidrago_sim.cpp",
        "https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L210-L220",
        "https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_004.inc#L210-L218",
    )
    replace(
        "tests/tier2_parts/part_000a2.inc",
        "https://github.com/FlareZ123/pokemon-sims/blob/main/tests/tier2_parts/part_000a1.inc#L21-L24",
        "https://github.com/FlareZ123/pokemon-sims/blob/main/tests/tier2_parts/part_000a1.inc#L21-L22",
    )

    tests_path = ROOT / "tests/update_setup_docs_tests.py"
    tests_text = tests_path.read_text(encoding="utf-8")
    contract = '''


def documented_build_targets(markdown: str) -> set[str]:
    return set(re.findall(r"\\./build/([A-Za-z0-9_-]+)", markdown))


def cmake_executable_targets(cmake_text: str) -> set[str]:
    return set(re.findall(r"add_executable\\(\\s*([A-Za-z0-9_-]+)", cmake_text))


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
        r"https://github\\.com/FlareZ123/pokemon-sims/blob/main/([^#\\s)]+)#L(\\d+)(?:-L(\\d+))?"
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
'''
    tests_text = tests_text.replace("\ndef main() -> int:\n", contract + "\n\ndef main() -> int:\n", 1)
    tests_text = tests_text.replace(
        "def main() -> int:\n    manifest = json.loads(",
        "def main() -> int:\n    assert_documented_build_commands_exist()\n"
        "    assert_same_repository_line_anchors_resolve()\n"
        "    manifest = json.loads(",
        1,
    )
    tests_path.write_text(tests_text, encoding="utf-8")

    subprocess.run(
        [sys.executable, str(ROOT / "scripts/update_setup_docs.py"), "--repo-root", str(ROOT)],
        check=True,
    )

    sys.path.insert(0, str(ROOT))
    from scripts.baseline_provenance import simulator_policy_source_digest

    manifest_path = ROOT / "results/baseline_manifest.json"
    manifest = json.loads(manifest_path.read_text(encoding="utf-8"))
    manifest["simulator_policy_source_sha256"] = simulator_policy_source_digest(ROOT)
    manifest_path.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    standard_ci = """name: CI

on:
  pull_request:
    branches: [main]
  push:
    branches: [main]
  workflow_dispatch:

permissions:
  contents: read

concurrency:
  group: ci-${{ github.event.pull_request.number || github.ref }}
  cancel-in-progress: true

jobs:
  release:
    runs-on: ubuntu-24.04
    timeout-minutes: 10
    steps:
      - uses: actions/checkout@v4
      - uses: actions/setup-python@v5
        with:
          python-version: \"3.12\"
      - name: Configure Release
        run: cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
      - name: Build simulator and unified tests
        run: cmake --build build --parallel 2
      - name: Run three independent simulate-this audits
        shell: bash
        run: |
          set -euo pipefail
          ./build/regidrago_sim --simulate-this --scenario strict-jit/go-second --seed 1 --require-ready-by 3 | tee trace-1.txt
          ./build/regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 2 --require-ready-by 4 | tee trace-2.txt
          ./build/regidrago_sim --simulate-this --scenario matchup-flex-jit/go-second --seed 3 --require-ready-by 2 | tee trace-3.txt
      - name: Generate canonical T2 and T3 matrix
        run: ./build/regidrago_sim --trials 100000 --seed 20260705 --out t2-t3-matrix.csv
      - name: Run complete Release suite
        run: ctest --test-dir build --parallel 4 --output-on-failure
      - name: Record tested source
        run: git rev-parse HEAD > tested-head.txt
      - name: Upload validation evidence
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: release-validation-${{ github.run_attempt }}
          path: |
            trace-*.txt
            t2-t3-matrix.csv
            tested-head.txt
            build/Testing
          if-no-files-found: warn

  sanitizers:
    runs-on: ubuntu-24.04
    timeout-minutes: 10
    steps:
      - uses: actions/checkout@v4
      - uses: actions/setup-python@v5
        with:
          python-version: \"3.12\"
      - name: Configure ASan and UBSan
        run: >-
          cmake -S . -B build-sanitizers -G Ninja -DCMAKE_BUILD_TYPE=Debug
          -DCMAKE_CXX_FLAGS=\"-fsanitize=address,undefined -fno-omit-frame-pointer\"
          -DCMAKE_EXE_LINKER_FLAGS=\"-fsanitize=address,undefined\"
      - name: Build sanitizer targets once
        run: cmake --build build-sanitizers --parallel 2
      - name: Run complete sanitizer suite
        env:
          # Detect the reference-parameter lifetime failure covered by issue #907:
          # https://eel.is/c++draft/class.temporary#6.10
          # https://github.com/FlareZ123/pokemon-sims/issues/907
          ASAN_OPTIONS: detect_stack_use_after_return=1:detect_leaks=1:halt_on_error=1
          UBSAN_OPTIONS: halt_on_error=1
        run: ctest --test-dir build-sanitizers --parallel 2 --output-on-failure
      - name: Upload sanitizer diagnostics
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: sanitizer-validation-${{ github.run_attempt }}
          path: build-sanitizers/Testing
          if-no-files-found: warn
"""
    (ROOT / ".github/workflows/ci.yml").write_text(standard_ci, encoding="utf-8")
    Path(__file__).unlink()
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
