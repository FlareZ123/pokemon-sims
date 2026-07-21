from __future__ import annotations

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

        # The generators own only the random temporary path and its matching
        # simulator lock. Final-output locking remains in the simulator:
        # https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_015.inc#L113-L170
        # https://github.com/FlareZ123/pokemon-sims/issues/1300
        if list(directory.glob(".*.tmp")) or list(directory.glob(".*.tmp.lock")):
            raise RuntimeError("A generator temporary file or lock remained.")
        if fail and final_path.exists():
            raise RuntimeError("A failed generator published a final matrix.")
        if not fail and final_path.read_text(encoding="utf-8") != "matrix\n":
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
