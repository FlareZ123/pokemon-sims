from pathlib import Path

source = Path("src/regidrago_sim.cpp")
text = source.read_text(encoding="utf-8")
old = '''#define fss_target_after_search_started fss_target_after_search_started_original
#define attach_fss attach_fss_original
#include "trace_engine_v2/part_010.inc"
#undef attach_fss
#undef fss_target_after_search_started
'''
new = '''#define fss_target_after_search_started fss_target_after_search_started_original
#define attach_fss attach_fss_original
#define should_play_steven should_play_steven_original
#include "trace_engine_v2/part_010.inc"
#undef should_play_steven
#undef attach_fss
#undef fss_target_after_search_started
'''
if '#define should_play_steven should_play_steven_original' not in text:
    if old not in text:
        raise SystemExit("part_010 composition anchor missing")
    text = text.replace(old, new, 1)

communication_anchor = '''#include "trace_engine_v2/part_pokemon_communication.inc"
'''
communication_insert = communication_anchor + '''#include "trace_engine_v2/part_010_late_steven_override.inc"
'''
if 'part_010_late_steven_override.inc' not in text:
    if communication_anchor not in text:
        raise SystemExit("Pokemon Communication composition anchor missing")
    text = text.replace(communication_anchor, communication_insert, 1)
source.write_text(text, encoding="utf-8")

cmake = Path("CMakeLists.txt")
cmake_text = cmake.read_text(encoding="utf-8")
target_anchor = '''add_executable(regidrago_steven_missing_regi_tests tests/steven_missing_regi_tests.cpp)
target_compile_options(regidrago_steven_missing_regi_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
'''
target_insert = target_anchor + '''
add_executable(regidrago_late_steven_basic_connector_tests tests/late_steven_basic_connector_tests.cpp)
target_compile_options(regidrago_late_steven_basic_connector_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
'''
if "regidrago_late_steven_basic_connector_tests" not in cmake_text:
    if target_anchor not in cmake_text:
        raise SystemExit("CMake target anchor missing")
    cmake_text = cmake_text.replace(target_anchor, target_insert, 1)

test_anchor = '''add_test(NAME regidrago_steven_missing_regi COMMAND regidrago_steven_missing_regi_tests)
'''
test_insert = test_anchor + '''add_test(NAME regidrago_late_steven_basic_connector COMMAND regidrago_late_steven_basic_connector_tests)
'''
if "NAME regidrago_late_steven_basic_connector " not in cmake_text:
    if test_anchor not in cmake_text:
        raise SystemExit("CMake test anchor missing")
    cmake_text = cmake_text.replace(test_anchor, test_insert, 1)
cmake.write_text(cmake_text, encoding="utf-8")
