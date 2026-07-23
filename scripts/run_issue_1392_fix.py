from __future__ import annotations

import os
import runpy
from pathlib import Path


HELPER = Path("scripts/apply_issue_1392_fix.py")
LOCK = HELPER.with_suffix(HELPER.suffix + ".lock")


def main() -> int:
    LOCK.parent.mkdir(parents=True, exist_ok=True)
    with LOCK.open("a+", encoding="utf-8") as lock_handle:
        if os.name == "nt":
            import msvcrt

            lock_handle.seek(0)
            msvcrt.locking(lock_handle.fileno(), msvcrt.LK_LOCK, 1)
        else:
            import fcntl

            fcntl.flock(lock_handle.fileno(), fcntl.LOCK_EX)

        text = HELPER.read_text(encoding="utf-8")
        old = "recipe(sim::shell_recipe()),"
        new = "recipe(sim::baseline_recipe()),"
        if new not in text:
            if text.count(old) != 1:
                raise RuntimeError("Unexpected issue-1392 recipe helper anchor count")
            text = text.replace(old, new, 1)
            temporary = HELPER.with_suffix(HELPER.suffix + ".tmp")
            temporary.write_text(text, encoding="utf-8", newline="\n")
            os.replace(temporary, HELPER)

    try:
        LOCK.unlink()
    except FileNotFoundError:
        pass

    namespace = runpy.run_path(str(HELPER), run_name="issue_1392_helper")
    return int(namespace["main"]())


if __name__ == "__main__":
    raise SystemExit(main())
