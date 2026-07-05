cmake_minimum_required(VERSION 3.22)
project(pokemon_sims LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# The audited implementation is isolated from the pre-audit source for a clear review diff.
add_executable(regidrago_sim src/regidrago_sim_fixed.cpp)
target_compile_options(regidrago_sim PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow)

enable_testing()
add_test(NAME regidrago_sim_self_test COMMAND regidrago_sim --self-test)
add_test(
  NAME regidrago_sim_smoke
  COMMAND regidrago_sim
    --trials 500
    --variant-trials 200
    --seed 20260705
    --out ${CMAKE_CURRENT_BINARY_DIR}/smoke_results.csv
    --variants-out ${CMAKE_CURRENT_BINARY_DIR}/smoke_variants.csv
)
set_tests_properties(regidrago_sim_smoke PROPERTIES RUN_SERIAL TRUE)
