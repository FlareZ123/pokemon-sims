from __future__ import annotations

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LEGACY_PATH = ROOT / "scripts" / "apply_issue_869.py"
source = LEGACY_PATH.read_text(encoding="utf-8")

old_helper = '''def replace_once(path: str, old: str, new: str) -> None:
    file_path = Path(path)
    text = file_path.read_text(encoding="utf-8")
    if old not in text:
        raise SystemExit(f"Unable to find patch anchor in {path}")
    file_path.write_text(text.replace(old, new, 1), encoding="utf-8")
'''
new_helper = '''def atomic_write_text(file_path: Path, text: str) -> None:
    import fcntl
    import os
    import tempfile

    file_path.parent.mkdir(parents=True, exist_ok=True)
    lock_path = file_path.with_name(f".{file_path.name}.issue-869.lock")
    with lock_path.open("a+", encoding="utf-8") as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=file_path.parent, delete=False
        ) as temporary:
            temporary.write(text)
            temporary.flush()
            os.fsync(temporary.fileno())
            temporary_path = Path(temporary.name)
        os.replace(temporary_path, file_path)
    lock_path.unlink(missing_ok=True)


def replace_once(path: str, old: str, new: str) -> None:
    file_path = Path(path)
    text = file_path.read_text(encoding="utf-8")
    if new in text:
        return
    if text.count(old) != 1:
        raise SystemExit(f"Unable to find unique patch anchor in {path}")
    atomic_write_text(file_path, text.replace(old, new, 1))
'''
if source.count(old_helper) != 1:
    raise SystemExit("Unable to locate the issue 869 replacement helper")
source = source.replace(old_helper, new_helper, 1)

write_start = "test_path.write_text(r'''"
write_end = "''', encoding=\"utf-8\")\n\ncmake_path = \"CMakeLists.txt\""
if source.count(write_start) != 1 or source.count(write_end) != 1:
    raise SystemExit("Unable to locate the issue 869 generated-test write")
source = source.replace(write_start, "atomic_write_text(test_path, r'''", 1)
source = source.replace(write_end, "''')\n\ncmake_path = \"CMakeLists.txt\"", 1)

cmake_marker = '\ncmake_path = "CMakeLists.txt"\n'
if source.count(cmake_marker) != 1:
    raise SystemExit("Unable to locate the obsolete issue 869 CMake section")
source = source.split(cmake_marker, 1)[0] + "\n"

# Current main discovers each tests/*.cpp source through the unified runner:
# https://github.com/FlareZ123/pokemon-sims/blob/main/CMakeLists.txt#L21-L47
# https://github.com/FlareZ123/pokemon-sims/blob/main/tests/generate_unified_tests.py#L109-L139
exec(compile(source, str(LEGACY_PATH), "exec"), {"__name__": "__main__"})
