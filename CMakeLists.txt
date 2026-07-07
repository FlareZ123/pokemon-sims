cmake_minimum_required(VERSION 3.22)
project(pokemon_sims LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(regidrago_sim src/regidrago_sim.cpp)
target_compile_options(regidrago_sim PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_policy_tests tests/regidrago_policy_tests.cpp)
target_compile_options(regidrago_policy_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_tier2_tests tests/regidrago_tier2_tests.cpp)
target_compile_options(regidrago_tier2_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_fss_rulebox_tests tests/forest_seal_stone_rulebox_tests.cpp)
target_compile_options(regidrago_fss_rulebox_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_arven_fss_rulebox_tests tests/arven_fss_rulebox_tests.cpp)
target_compile_options(regidrago_arven_fss_rulebox_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_known_dead_payload_tests tests/known_dead_payload_tests.cpp)
target_compile_options(regidrago_known_dead_payload_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_ultra_ball_tapu_tests tests/ultra_ball_tapu_connector_tests.cpp)
target_compile_options(regidrago_ultra_ball_tapu_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_ultra_ball_cost_tests tests/ultra_ball_cost_tests.cpp)
target_compile_options(regidrago_ultra_ball_cost_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_wonder_tag_dead_burnet_tests tests/wonder_tag_known_dead_burnet_tests.cpp)
target_compile_options(regidrago_wonder_tag_dead_burnet_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_oricorio_burnet_tests tests/oricorio_burnet_compression_tests.cpp)
target_compile_options(regidrago_oricorio_burnet_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_late_item_supporter_tests tests/late_item_supporter_tests.cpp)
target_compile_options(regidrago_late_item_supporter_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

enable_testing()
add_test(NAME regidrago_sim_self_test COMMAND regidrago_sim --self-test)
add_test(NAME regidrago_policy_fixtures COMMAND regidrago_policy_tests)
add_test(NAME regidrago_tier2_policy_fixtures COMMAND regidrago_tier2_tests)
add_test(NAME regidrago_fss_rulebox COMMAND regidrago_fss_rulebox_tests)
add_test(NAME regidrago_arven_fss_rulebox COMMAND regidrago_arven_fss_rulebox_tests)
add_test(NAME regidrago_known_dead_payload COMMAND regidrago_known_dead_payload_tests)
add_test(NAME regidrago_ultra_ball_tapu COMMAND regidrago_ultra_ball_tapu_tests)
add_test(NAME regidrago_ultra_ball_cost COMMAND regidrago_ultra_ball_cost_tests)
add_test(NAME regidrago_wonder_tag_dead_burnet COMMAND regidrago_wonder_tag_dead_burnet_tests)
add_test(NAME regidrago_oricorio_burnet COMMAND regidrago_oricorio_burnet_tests)
add_test(NAME regidrago_late_item_supporter COMMAND regidrago_late_item_supporter_tests)
add_test(NAME trace_strict_second_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-second --seed 6 --require-ready-by 3)
add_test(NAME trace_strict_first_t4 COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 2 --require-ready-by 4)
add_test(NAME trace_flex_second_t2 COMMAND regidrago_sim --simulate-this --scenario matchup-flex-jit/go-second --seed 4 --require-ready-by 2)
add_test(NAME trace_turn2_item_lock_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit-turn2-item-lock/go-second --seed 9 --require-ready-by 3)
add_test(NAME trace_rulebox_lock_t3 COMMAND regidrago_sim --simulate-this --scenario strict-jit-rulebox-ability-lock/go-second --seed 19 --require-ready-by 3)
add_test(NAME trace_no_control_first_t4 COMMAND regidrago_sim --simulate-this --scenario no-discard-control/go-first --seed 6 --require-ready-by 4)
add_test(NAME regidrago_sim_matrix_smoke COMMAND regidrago_sim --trials 100 --seed 20260705 --out ${CMAKE_CURRENT_BINARY_DIR}/smoke_results.csv)
set_tests_properties(regidrago_sim_matrix_smoke PROPERTIES RUN_SERIAL TRUE)
