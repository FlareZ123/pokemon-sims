from __future__ import annotations

import importlib.util
import subprocess
import sys
import tempfile
from pathlib import Path
from types import ModuleType

ROOT = Path(__file__).resolve().parents[1]
_MISSING_MODULE = object()


def load_module(path: Path, module_name: str) -> ModuleType:
    specification = importlib.util.spec_from_file_location(module_name, path)
    if specification is None or specification.loader is None:
        raise RuntimeError(f"Could not load {path}")
    module = importlib.util.module_from_spec(specification)
    previous_module = sys.modules.get(module_name, _MISSING_MODULE)
    # Python's direct-file import recipe registers the module before exec_module():
    # https://docs.python.org/3/library/importlib.html#importing-a-source-file-directly
    # Restore the caller's module table after the synthetic test import:
    # https://docs.python.org/3/library/sys.html#sys.modules
    sys.modules[module_name] = module
    try:
        specification.loader.exec_module(module)
    finally:
        if previous_module is _MISSING_MODULE:
            sys.modules.pop(module_name, None)
        else:
            sys.modules[module_name] = previous_module
    return module


def exercise_generator(module: ModuleType, *, fail: bool) -> None:
    with tempfile.TemporaryDirectory() as directory_name:
        directory = Path(directory_name)
        final_path = directory / "matrix.csv"

        def fake_run(command: list[str], *, check: bool = True) -> subprocess.CompletedProcess[str]:
            del check
            temporary_path = Path(command[command.index("--out") + 1])
            temporary_path.write_text("matrix\n", encoding="utf-8")
            Path(f"{temporary_path}.lock").touch()
            if fail:
                raise subprocess.CalledProcessError(1, command, output="expected failure")
            return subprocess.CompletedProcess(command, 0, "")

        module.run = fake_run
        try:
            module.generate_matrix_atomic(Path("simulator"), final_path, 1, 1300)
        except subprocess.CalledProcessError:
            if not fail:
                raise
        else:
            if fail:
                raise RuntimeError("The failure control unexpectedly succeeded.")

        # Both source-bound generator entry points own only their random temporary
        # path and matching simulator lock. Final-output locking remains in the simulator:
        # https://github.com/FlareZ123/pokemon-sims/blob/main/scripts/generate_multi_deck_comparison.py
        # https://github.com/FlareZ123/pokemon-sims/blob/main/scripts/regenerate_setup_baselines.py
        # https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_015.inc#L113-L170
        # https://github.com/FlareZ123/pokemon-sims/issues/1300
        if list(directory.glob(".*.tmp")) or list(directory.glob(".*.tmp.lock")):
            raise RuntimeError("A generator temporary file or lock remained.")
        if fail and final_path.exists():
            raise RuntimeError("A failed generator published a final matrix.")
        if not fail and final_path.read_text(encoding="utf-8") != "matrix\n":
            raise RuntimeError("A successful generator did not publish the matrix.")


def assert_no_tracked_source_locks() -> None:
    # Source-update locks are process coordination artifacts and must never become
    # source, package, or evidence state:
    # https://github.com/FlareZ123/pokemon-sims/issues/1492
    # https://github.com/FlareZ123/pokemon-sims/issues/1300
    result = subprocess.run(
        ["git", "ls-files", "src"],
        cwd=ROOT,
        check=True,
        text=True,
        capture_output=True,
    )
    tracked_locks = sorted(
        path for path in result.stdout.splitlines() if path.endswith(".lock")
    )
    if tracked_locks:
        raise RuntimeError(f"Tracked source lock artifacts remain: {tracked_locks}")

    ignore_text = (ROOT / ".gitignore").read_text(encoding="utf-8")
    if "/src/*.lock" not in ignore_text or "/src/**/*.lock" not in ignore_text:
        raise RuntimeError("The source-lock ignore contract is missing.")


def assert_source_locks_do_not_change_provenance() -> None:
    provenance = load_module(
        ROOT / "scripts/baseline_provenance.py", "issue1492_provenance"
    )
    with tempfile.TemporaryDirectory() as directory_name:
        repo_root = Path(directory_name)
        source_dir = repo_root / "src" / "trace_engine_v2"
        source_dir.mkdir(parents=True)
        (repo_root / "CMakeLists.txt").write_text(
            "add_executable(regidrago_sim src/regidrago_sim.cpp)\n", encoding="utf-8"
        )
        source = repo_root / "src" / "regidrago_sim.cpp"
        source.write_text("int main() { return 0; }\n", encoding="utf-8")
        initial_digest = provenance.simulator_policy_source_digest(repo_root)

        # Runtime lock creation and cleanup cannot change source-bound evidence:
        # https://github.com/FlareZ123/pokemon-sims/issues/1492
        # https://github.com/FlareZ123/pokemon-sims/issues/1300
        source_lock = repo_root / "src" / "regidrago_sim.cpp.lock"
        nested_lock = source_dir / "part_001.inc.lock"
        source_lock.write_text("writer-a", encoding="utf-8")
        nested_lock.write_text("writer-b", encoding="utf-8")
        locked_digest = provenance.simulator_policy_source_digest(repo_root)
        source_lock.unlink()
        nested_lock.unlink()
        cleaned_digest = provenance.simulator_policy_source_digest(repo_root)
        if locked_digest != initial_digest or cleaned_digest != initial_digest:
            raise RuntimeError("Source lock artifacts changed simulator provenance.")

        source.write_text("int main() { return 1; }\n", encoding="utf-8")
        if provenance.simulator_policy_source_digest(repo_root) == initial_digest:
            raise RuntimeError("A real simulator source change did not change provenance.")


def main() -> None:
    modules = [
        load_module(ROOT / "scripts/generate_multi_deck_comparison.py", "issue1300_multi"),
        load_module(ROOT / "scripts/regenerate_setup_baselines.py", "issue1300_baseline"),
    ]
    for module in modules:
        exercise_generator(module, fail=False)
        exercise_generator(module, fail=True)
    assert_no_tracked_source_locks()
    assert_source_locks_do_not_change_provenance()
    print("Issue 1300 and 1492 lock cleanup tests passed.")


if __name__ == "__main__":
    main()
