from __future__ import annotations

import contextlib
import os
from pathlib import Path
import tempfile


SOURCE = Path("src/trace_engine_v2/part_issue_1030_steven_turo_override.inc")
LOCK = SOURCE.with_suffix(SOURCE.suffix + ".issue1378.lock")


@contextlib.contextmanager
def exclusive_lock(path: Path):
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = path.open("a+", encoding="utf-8")
    try:
        import fcntl

        fcntl.flock(handle.fileno(), fcntl.LOCK_EX)
        yield
    finally:
        try:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
        finally:
            handle.close()


def atomic_write(path: Path, text: str) -> None:
    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=path.parent, delete=False, newline=""
    ) as temporary:
        temporary.write(text)
        temporary.flush()
        os.fsync(temporary.fileno())
        temporary_path = Path(temporary.name)
    os.replace(temporary_path, path)


def replace_once(text: str, old: str, new: str) -> str:
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Expected one patch anchor, found {count}: {old[:80]!r}")
    return text.replace(old, new, 1)


def main() -> None:
    with exclusive_lock(LOCK):
        text = SOURCE.read_text(encoding="utf-8")
        if "issues/1378" in text and "supported_lock" in text:
            return

        text = replace_once(
            text,
            """  bool issue_1223_full_item_lock_steven_grass_route_available() const {\n    if (!deck_seen_ || !supporter_allowed() ||\n        scenario_.dci != DciProfile::StrictJit ||\n        scenario_.locks != LockMode::FullItem ||\n""",
            """  bool issue_1223_full_item_lock_steven_grass_route_available() const {\n    // The reviewed #1223 decomposition also applies in the equivalent no-lock K1\n    // state while every card, Energy, Bench, Latias, payload, and horizon guard\n    // below remains required: https://github.com/FlareZ123/pokemon-sims/issues/1378\n    const bool supported_lock = scenario_.locks == LockMode::FullItem ||\n                                scenario_.locks == LockMode::None;\n    if (!deck_seen_ || !supporter_allowed() ||\n        scenario_.dci != DciProfile::StrictJit || !supported_lock ||\n""",
        )
        text = replace_once(
            text,
            """    // held under permanent Item lock, the deterministic package is Regidrago V,\n    // Crispin, and Grass Energy. A second VSTAR duplicates a completed axis, while\n    // the searched Grass preserves the manual-attachment plus Crispin GGF schedule:\n""",
            """    // held under permanent Item lock or the equivalent proven no-lock K1 state, the\n    // deterministic package is Regidrago V, Crispin, and Grass Energy. A second\n    // VSTAR duplicates a completed axis, while the searched Grass preserves the\n    // manual-attachment plus Crispin GGF schedule:\n""",
        )
        text = replace_once(
            text,
            "    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1223\n",
            "    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1223 https://github.com/FlareZ123/pokemon-sims/issues/1378\n",
        )
        text = replace_once(
            text,
            "    // Route specification: https://github.com/FlareZ123/pokemon-sims/issues/1223\n",
            "    // Route specifications: https://github.com/FlareZ123/pokemon-sims/issues/1223 https://github.com/FlareZ123/pokemon-sims/issues/1378\n",
        )
        atomic_write(SOURCE, text)

    try:
        LOCK.unlink()
    except FileNotFoundError:
        pass


if __name__ == "__main__":
    main()
