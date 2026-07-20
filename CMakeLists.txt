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
# A prior-turn GGF Regidrago V must hold the one-use Blender until the VSTAR
# card and Active-position axes can complete, producing the verified T4 route:
# https://api.pokemontcg.io/v2/cards/sv8-164
# https://api.pokemontcg.io/v2/cards/swsh12-136
# https://github.com/FlareZ123/pokemon-sims/issues/718
# https://github.com/FlareZ123/pokemon-sims/pull/931
add_test(NAME trace_issue_718_strict_first_seed1 COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 1 --require-ready-by 4)
add_test(NAME trace_flex_second_t2 COMMAND regidrago_sim --simulate-this --scenario matchup-flex-jit/go-second --seed 3 --require-ready-by 2)
add_test(NAME trace_turn2_item_lock_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit-turn2-item-lock/go-second --seed 1 --require-ready-by 3)
# The confirmed strict-DCI regression must use the direct turn-one VSTAR search and
# reach readiness on turn two before scheduled Item lock removes search Items:
# https://api.pokemontcg.io/v2/cards/sm6-113
# https://api.pokemontcg.io/v2/cards/swsh12-136
# https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
# https://github.com/FlareZ123/pokemon-sims/issues/925
add_test(NAME trace_issue_925_turn2_item_lock_seed25 COMMAND regidrago_sim --simulate-this --scenario strict-jit-turn2-item-lock/go-second --seed 25 --require-ready-by 2)
# Issue #989 preserves the same earliest ready turns while skipping unused Wonder Tag -> Crispin.
# https://github.com/FlareZ123/pokemon-sims/issues/989
# https://api.pokemontcg.io/v2/cards/cel25c-60_A
# https://api.pokemontcg.io/v2/cards/sv7-133
# https://www.pokemon.com/us/pokemon-tcg/rules
add_test(NAME trace_issue_989_item_lock_seed25 COMMAND regidrago_sim --simulate-this --scenario strict-jit-turn2-item-lock/go-second --seed 25 --require-ready-by 2)
add_test(NAME trace_issue_989_strict_first_seed107 COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 107 --require-ready-by 3)
add_test(NAME trace_issue_989_strict_first_seed122 COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 122 --require-ready-by 4)
# Quick Ball must precede Earthen Vessel only when Tapu Lele-GX into Crispin
# compresses the Energy axis and every separate discard cost remains payable:
# https://api.pokemontcg.io/v2/cards/swsh1-179
# https://api.pokemontcg.io/v2/cards/cel25c-60_A
# https://api.pokemontcg.io/v2/cards/sv7-133
# https://api.pokemontcg.io/v2/cards/sv4-163
# https://www.pokemon.com/us/pokemon-tcg/rules
# https://github.com/FlareZ123/pokemon-sims/issues/962
add_test(NAME trace_issue_962_strict_first_seed104 COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 104 --require-ready-by 3)
# One of two held Regidrago VSTAR copies may pay Quick Ball while the other stays
# protected for the Regidrago V that the search establishes, producing the legal T3 route:
# https://api.pokemontcg.io/v2/cards/swsh1-179
# https://api.pokemontcg.io/v2/cards/swsh12-135
# https://api.pokemontcg.io/v2/cards/swsh12-136
# https://www.pokemon.com/us/pokemon-tcg/rules
# https://github.com/FlareZ123/pokemon-sims/issues/964
add_test(NAME trace_issue_964_strict_first_seed43 COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 43 --require-ready-by 3)
# The source-bound audit manifest includes this Rule Box Ability lock scenario in
# the six-trace inventory, so CTest must enforce its ready-by deadline as well:
# https://github.com/FlareZ123/pokemon-sims/blob/main/results/baseline_manifest.json
# https://github.com/FlareZ123/pokemon-sims/issues/917
add_test(NAME trace_rulebox_lock_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit-rulebox-ability-lock/go-second --seed 20 --require-ready-by 3)
add_test(NAME trace_no_control_first_t4 COMMAND regidrago_sim --simulate-this --scenario no-discard-control/go-first --seed 6 --require-ready-by 4)
# Seed 19 must hold the random top-three attack when held Fire already guarantees
# the last GGF attachment, while multi-Energy and no-discard-control routes stay live:
# https://api.pokemontcg.io/v2/cards/swsh12-135
# https://api.pokemontcg.io/v2/cards/swsh12-136
# https://www.pokemon.com/us/pokemon-tcg/rules
# https://github.com/FlareZ123/pokemon-sims/issues/1079
add_test(NAME trace_issue_1079_celestial_roar_hold
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_issue_1079_celestial_roar_hold.cmake)
# Star Alchemy must choose the deterministic scheduled-lock Crispin route over
# Oricorio plus unknown Celestial Roar top cards:
# https://api.pokemontcg.io/v2/cards/swsh12-156
# https://api.pokemontcg.io/v2/cards/sv7-133
# https://api.pokemontcg.io/v2/cards/swsh12-135
# https://github.com/FlareZ123/pokemon-sims/issues/1085
add_test(NAME trace_issue_1085_fss_crispin_next_turn
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_issue_1085_fss_crispin_next_turn.cmake)
# Wonder Tag must select Serena when a held manual attachment completes GGF and
# Serena can establish the strict-JIT payload without spending Legacy Star:
# https://api.pokemontcg.io/v2/cards/cel25c-60_A
# https://api.pokemontcg.io/v2/cards/swsh12-164
# https://api.pokemontcg.io/v2/cards/swsh12-136
# https://github.com/FlareZ123/pokemon-sims/issues/1086
add_test(NAME trace_issue_1086_wonder_tag_serena_payload
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_issue_1086_wonder_tag_serena_payload.cmake)
# A held Arven suppresses blind Gladion only when its searched Item can legally
# resolve the missing-Regidrago-V route after the Supporter play:
# https://api.pokemontcg.io/v2/cards/sm4-95
# https://api.pokemontcg.io/v2/cards/sv1-166
# https://api.pokemontcg.io/v2/cards/swsh1-179
# https://api.pokemontcg.io/v2/cards/sm6-113
# https://github.com/FlareZ123/pokemon-sims/issues/1090
# https://github.com/FlareZ123/pokemon-sims/pull/1095
add_test(NAME trace_issue_1090_gladion_arven_route
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_issue_1090_gladion_arven_route.cmake)
# Quick Ball may discard a strict-JIT Dragon only when the selected Regidrago VSTAR
# can finish both its card and GGF Energy axes during that same turn:
# https://api.pokemontcg.io/v2/cards/swsh1-179
# https://api.pokemontcg.io/v2/cards/swsh12-136
# https://www.pokemon.com/us/pokemon-tcg/rules
# https://github.com/FlareZ123/pokemon-sims/issues/1092
add_test(NAME trace_issue_1092_quick_ball_energy_jit_gate
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_issue_1092_quick_ball_energy_jit_gate.cmake)
# Serena may discard a strict-JIT Dragon only when its paid Supporter state can
# finish the VSTAR card, GGF Energy, and Active-position axes during that turn:
# https://api.pokemontcg.io/v2/cards/swsh12-164
# https://api.pokemontcg.io/v2/cards/swsh12-136
# https://api.pokemontcg.io/v2/cards/sv8-76
# https://www.pokemon.com/us/pokemon-tcg/rules
# https://github.com/FlareZ123/pokemon-sims/issues/1099
add_test(NAME trace_issue_1099_serena_complete_axis_jit_gate
  COMMAND ${CMAKE_COMMAND}
    -DSIMULATOR=$<TARGET_FILE:regidrago_sim>
    -P ${CMAKE_CURRENT_SOURCE_DIR}/tests/run_issue_1099_serena_complete_axis_jit_gate.cmake)
# A held Brilliant Blender must establish the current-turn payload before the
# turn-ending late Steven-to-Mysterious-Treasure bridge is considered:
# https://api.pokemontcg.io/v2/cards/sv8-164
# https://api.pokemontcg.io/v2/cards/sm7-145
# https://api.pokemontcg.io/v2/cards/sm6-113
# https://api.pokemontcg.io/v2/cards/swsh12-136
# https://www.pokemon.com/us/pokemon-tcg/rules
# https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# https://github.com/FlareZ123/pokemon-sims/issues/1111
add_test(NAME trace_issue_1111_held_blender_preempts_steven
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-second --seed 7180 --require-ready-by 3)

# A held Professor Burnet completes the current strict-JIT payload axis before
# the turn-ending late Steven-to-Mysterious-Treasure bridge is considered:
# https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
# https://api.pokemontcg.io/v2/cards/sm7-145
# https://api.pokemontcg.io/v2/cards/sm6-113
# https://api.pokemontcg.io/v2/cards/swsh12-136
# https://www.pokemon.com/us/pokemon-tcg/rules
# https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# https://github.com/FlareZ123/pokemon-sims/issues/1117
add_test(NAME trace_issue_1117_held_burnet_preempts_steven
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 8697 --require-ready-by 4)

add_test(NAME regidrago_sim_matrix_smoke COMMAND regidrago_sim --trials 100 --seed 20260705 --out ${CMAKE_CURRENT_BINARY_DIR}/smoke_results.csv)
set_tests_properties(regidrago_sim_matrix_smoke PROPERTIES RUN_SERIAL TRUE)

add_test(NAME card_audit_contract COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/card_audit_contract_tests.py")
add_test(NAME card_audit_category_contract COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/card_audit_category_contract_tests.py")
add_test(NAME setup_report_contract COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/setup_report_contract_tests.py")
add_test(NAME setup_docs_generator_contract COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/update_setup_docs_tests.py")
