from __future__ import annotations

import os
from pathlib import Path

LOCK_PATH = Path(".issue-875.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-875.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text and old not in text:
        return
    if text.count(old) != 1:
        raise SystemExit(f"issue 875 anchor missing or duplicated in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    descriptor = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(descriptor, str(os.getpid()).encode("ascii"))
        cmake = Path("CMakeLists.txt")

        replace_once(
            cmake,
            """add_executable(regidrago_release_assertion_tests tests/release_assertion_tests.cpp)
target_compile_options(regidrago_release_assertion_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

enable_testing()
""",
            """add_executable(regidrago_release_assertion_tests tests/release_assertion_tests.cpp)
target_compile_options(regidrago_release_assertion_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

# Stable exact-state route contracts replace shuffled-seed assertions whose
# prerequisite states drifted with the canonical recipe and action policy:
# https://github.com/FlareZ123/pokemon-sims/issues/875
add_executable(regidrago_stable_route_contract_tests tests/stable_route_contract_tests.cpp)
target_compile_options(regidrago_stable_route_contract_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

enable_testing()
""",
        )

        replace_once(
            cmake,
            """add_test(NAME regidrago_future_wonder_tag_crispin
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_future_wonder_tag_crispin.cmake)
add_test(NAME regidrago_active_tapu_paid_retreat
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/check_active_tapu_paid_retreat.cmake)
""",
            """# Exact public-state contracts for Wonder Tag banking, paid retreat, and
# Rule Box Ability lock semantics:
# https://github.com/FlareZ123/pokemon-sims/issues/809
# https://github.com/FlareZ123/pokemon-sims/issues/802
# https://github.com/FlareZ123/pokemon-sims/issues/875
add_test(NAME regidrago_stable_route_contracts COMMAND regidrago_stable_route_contract_tests)
""",
        )

        replace_once(
            cmake,
            "add_test(NAME trace_rulebox_lock_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit-rulebox-ability-lock/go-second --seed 1 --require-ready-by 3)\n",
            "",
        )
    finally:
        os.close(descriptor)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
