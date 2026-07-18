from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-779-v2.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-779-v2.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 779 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))

        source = Path("src/regidrago_sim.cpp")
        source_anchor = '#include "trace_engine_v2/part_010_steven_crispin_override.inc"\n'
        source_replacement = '#include "trace_engine_v2/part_010_late_steven_override.inc"\n#include "trace_engine_v2/part_010_steven_crispin_override.inc"\n'
        replace_once(source, source_anchor, source_replacement)

        steven_policy = Path("src/trace_engine_v2/part_010_steven_crispin_override.inc")
        policy_anchor = '''  bool should_play_steven() const {
    return should_play_steven_original() &&
        !steven_should_yield_to_crispin_mysterious_route();
  }
'''
        policy_replacement = '''  bool should_play_steven() const {
    return (should_play_steven_original() &&
            !steven_should_yield_to_crispin_mysterious_route()) ||
        late_steven_is_only_basic_connector();
  }
'''
        replace_once(steven_policy, policy_anchor, policy_replacement)

        cmake = Path("CMakeLists.txt")
        target_anchor = "add_executable(regidrago_steven_missing_regi_tests tests/steven_missing_regi_tests.cpp)\ntarget_compile_options(regidrago_steven_missing_regi_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)\n"
        target_replacement = target_anchor + "\nadd_executable(regidrago_late_steven_basic_connector_tests tests/late_steven_basic_connector_tests.cpp)\ntarget_compile_options(regidrago_late_steven_basic_connector_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)\n"
        replace_once(cmake, target_anchor, target_replacement)

        test_anchor = "add_test(NAME regidrago_steven_missing_regi COMMAND regidrago_steven_missing_regi_tests)\n"
        test_replacement = test_anchor + "add_test(NAME regidrago_late_steven_basic_connector COMMAND regidrago_late_steven_basic_connector_tests)\n"
        replace_once(cmake, test_anchor, test_replacement)
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
