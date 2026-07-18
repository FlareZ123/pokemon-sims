cmake_minimum_required(VERSION 3.22)
project(pokemon_sims LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

function(regidrago_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)
  endif()
endfunction()

add_executable(regidrago_sim src/regidrago_sim.cpp)
regidrago_warnings(regidrago_sim)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
file(GLOB_RECURSE REGIDRAGO_TEST_INPUTS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.inc")
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/generate_unified_tests.py"
  ${REGIDRAGO_TEST_INPUTS})
set(REGIDRAGO_GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(REGIDRAGO_GENERATED_CPP "${REGIDRAGO_GENERATED_DIR}/regidrago_unified_tests.cpp")
set(REGIDRAGO_GENERATED_CASES "${REGIDRAGO_GENERATED_DIR}/regidrago_unified_cases.cmake")
file(MAKE_DIRECTORY "${REGIDRAGO_GENERATED_DIR}")
execute_process(
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/generate_unified_tests.py"
          --source-root "${CMAKE_CURRENT_SOURCE_DIR}"
          --output-cpp "${REGIDRAGO_GENERATED_CPP}"
          --output-cmake "${REGIDRAGO_GENERATED_CASES}"
  RESULT_VARIABLE REGIDRAGO_GENERATOR_STATUS
  OUTPUT_VARIABLE REGIDRAGO_GENERATOR_OUTPUT
  ERROR_VARIABLE REGIDRAGO_GENERATOR_ERROR)
if(NOT REGIDRAGO_GENERATOR_STATUS EQUAL 0)
  message(FATAL_ERROR "Unified test generation failed:\n${REGIDRAGO_GENERATOR_OUTPUT}\n${REGIDRAGO_GENERATOR_ERROR}")
endif()
include("${REGIDRAGO_GENERATED_CASES}")

add_executable(regidrago_unified_tests "${REGIDRAGO_GENERATED_CPP}")
target_include_directories(regidrago_unified_tests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
regidrago_warnings(regidrago_unified_tests)
# The test runner validates behavior rather than optimized throughput. Keeping it at
# O0 turns 79 repeated simulator compilations into one short compile while the user-
# facing simulator remains optimized by the selected build type:
# https://cmake.org/cmake/help/latest/command/target_compile_options.html
if(MSVC)
  target_compile_options(regidrago_unified_tests PRIVATE /Od /UNDEBUG)
else()
  target_compile_options(regidrago_unified_tests PRIVATE -O0 -UNDEBUG)
endif()

add_executable(regidrago_release_assertion_tests tests/release_assertion_tests.cpp)
regidrago_warnings(regidrago_release_assertion_tests)
if(MSVC)
  target_compile_options(regidrago_release_assertion_tests PRIVATE /UNDEBUG)
else()
  target_compile_options(regidrago_release_assertion_tests PRIVATE -UNDEBUG)
endif()

enable_testing()
foreach(case_name IN LISTS REGIDRAGO_UNIFIED_CASES)
  add_test(NAME "regidrago_${case_name}" COMMAND regidrago_unified_tests "${case_name}")
endforeach()

add_test(NAME regidrago_sim_self_test COMMAND regidrago_sim --self-test)
add_test(NAME regidrago_release_assertions_enabled
  COMMAND ${CMAKE_COMMAND}
    -DASSERTION_PROBE=$<TARGET_FILE:regidrago_release_assertion_tests>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_assertion_probe.cmake)
add_test(NAME regidrago_cli_mode_contract
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -DTEST_DIR=${CMAKE_CURRENT_BINARY_DIR}/cli-mode-contract
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_cli_mode_contract.cmake)

add_test(NAME trace_strict_second_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-second --seed 1 --require-ready-by 3)
add_test(NAME trace_strict_first_t4 COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 2 --require-ready-by 4)
add_test(NAME trace_flex_second_t2 COMMAND regidrago_sim --simulate-this --scenario matchup-flex-jit/go-second --seed 3 --require-ready-by 2)
add_test(NAME trace_turn2_item_lock_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit-turn2-item-lock/go-second --seed 1 --require-ready-by 3)
add_test(NAME trace_no_control_first_t4 COMMAND regidrago_sim --simulate-this --scenario no-discard-control/go-first --seed 6 --require-ready-by 4)
add_test(NAME regidrago_sim_matrix_smoke COMMAND regidrago_sim --trials 100 --seed 20260705 --out ${CMAKE_CURRENT_BINARY_DIR}/smoke_results.csv)
set_tests_properties(regidrago_sim_matrix_smoke PROPERTIES RUN_SERIAL TRUE)

add_test(NAME card_audit_contract COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/card_audit_contract_tests.py")
add_test(NAME card_audit_category_contract COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/card_audit_category_contract_tests.py")
add_test(NAME setup_report_contract COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/setup_report_contract_tests.py")
add_test(NAME setup_docs_generator_contract COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/update_setup_docs_tests.py")
