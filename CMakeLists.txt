cmake_minimum_required(VERSION 3.22)
project(pokemon_sims LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(regidrago_sim src/regidrago_sim.cpp)
target_compile_options(regidrago_sim PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

add_executable(regidrago_sim_tests tests/regidrago_sim_tests.cpp)
target_compile_options(regidrago_sim_tests PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

enable_testing()
add_test(NAME regidrago_sim_tests COMMAND regidrago_sim_tests)
add_test(
  NAME regidrago_sim_smoke
  COMMAND regidrago_sim
    --trials 100
    --variant-trials 25
    --seed 20260704
    --out ${CMAKE_CURRENT_BINARY_DIR}/smoke_results.csv
    --variants-out ${CMAKE_CURRENT_BINARY_DIR}/smoke_variants.csv
)
set_tests_properties(regidrago_sim_smoke PROPERTIES RUN_SERIAL TRUE)
