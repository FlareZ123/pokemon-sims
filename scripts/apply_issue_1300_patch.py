from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LOCK_PATH = ROOT / ".apply_issue_1300_patch.lock"


def atomic_write(path: Path, text: str) -> None:
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=path.parent
    )
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary_name, path)
    except BaseException:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Expected one source anchor in {path}, found {count}")
    atomic_write(path, text.replace(old, new, 1))


def patch_generator(path: Path) -> None:
    replace_once(
        path,
        """    temporary_path = Path(temporary_name)
    try:
""",
        """    temporary_path = Path(temporary_name)
    # The simulator locks the exact temporary output path. Remove that random
    # generator-only lock after either success or failure:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_015.inc#L113-L170
    # https://github.com/FlareZ123/pokemon-sims/issues/1300
    temporary_lock_path = Path(f"{temporary_path}.lock")
    try:
""",
    )
    replace_once(
        path,
        """    finally:
        temporary_path.unlink(missing_ok=True)
""",
        """    finally:
        temporary_path.unlink(missing_ok=True)
        temporary_lock_path.unlink(missing_ok=True)
""",
    )


def apply_patch() -> None:
    patch_generator(ROOT / "scripts/generate_multi_deck_comparison.py")
    patch_generator(ROOT / "scripts/regenerate_setup_baselines.py")

    cmake = ROOT / "CMakeLists.txt"
    replace_once(
        cmake,
        """add_test(NAME regidrago_cli_mode_contract
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/cli-mode-contract
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_cli_mode_contract.cmake)

""",
        """add_test(NAME regidrago_cli_mode_contract
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/cli-mode-contract
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_cli_mode_contract.cmake)
# Both source-bound matrix generators must remove simulator-created locks for
# their random temporary output paths after success and failure:
# https://github.com/FlareZ123/pokemon-sims/issues/1300
add_test(NAME regidrago_issue_1300_matrix_lock_cleanup
  COMMAND ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/issue_1300_matrix_lock_cleanup_tests.py)

""",
    )

    test = ROOT / "tests/issue_1300_matrix_lock_cleanup_tests.py"
    atomic_write(
        test,
        '''from __future__ import annotations

import importlib.util
import subprocess
import tempfile
from pathlib import Path
from types import ModuleType

ROOT = Path(__file__).resolve().parents[1]


def load_module(path: Path, module_name: str) -> ModuleType:
    specification = importlib.util.spec_from_file_location(module_name, path)
    if specification is None or specification.loader is None:
        raise RuntimeError(f"Could not load {path}")
    module = importlib.util.module_from_spec(specification)
    specification.loader.exec_module(module)
    return module


def exercise_generator(module: ModuleType, *, fail: bool) -> None:
    with tempfile.TemporaryDirectory() as directory_name:
        directory = Path(directory_name)
        final_path = directory / "matrix.csv"

        def fake_run(command: list[str], *, check: bool = True) -> subprocess.CompletedProcess[str]:
            del check
            temporary_path = Path(command[command.index("--out") + 1])
            temporary_path.write_text("matrix\\n", encoding="utf-8")
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

        # The generators own only the random temporary path and its matching
        # simulator lock. Final-output locking remains in the simulator:
        # https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_015.inc#L113-L170
        # https://github.com/FlareZ123/pokemon-sims/issues/1300
        if list(directory.glob(".*.tmp")) or list(directory.glob(".*.tmp.lock")):
            raise RuntimeError("A generator temporary file or lock remained.")
        if fail and final_path.exists():
            raise RuntimeError("A failed generator published a final matrix.")
        if not fail and final_path.read_text(encoding="utf-8") != "matrix\\n":
            raise RuntimeError("A successful generator did not publish the matrix.")


def main() -> None:
    modules = [
        load_module(ROOT / "scripts/generate_multi_deck_comparison.py", "issue1300_multi"),
        load_module(ROOT / "scripts/regenerate_setup_baselines.py", "issue1300_baseline"),
    ]
    for module in modules:
        exercise_generator(module, fail=False)
        exercise_generator(module, fail=True)
    print("Issue 1300 matrix lock cleanup tests passed.")


if __name__ == "__main__":
    main()
''',
    )

    results = ROOT / "results"
    for lock_path in results.glob(".*.tmp.lock"):
        lock_path.unlink()


def main() -> None:
    LOCK_PATH.touch(exist_ok=True)
    with LOCK_PATH.open("r+") as lock_handle:
        fcntl.flock(lock_handle.fileno(), fcntl.LOCK_EX)
        apply_patch()
        fcntl.flock(lock_handle.fileno(), fcntl.LOCK_UN)
    LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
