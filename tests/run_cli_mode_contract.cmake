# Executable-level contract for the four documented CLI execution modes:
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-aggregate-smoke-test
# https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
# https://github.com/FlareZ123/pokemon-sims/issues/633
if(NOT DEFINED SIMULATOR)
  message(FATAL_ERROR "SIMULATOR is required")
endif()
if(NOT DEFINED TEST_DIR)
  message(FATAL_ERROR "TEST_DIR is required")
endif()

file(REMOVE_RECURSE "${TEST_DIR}")
file(MAKE_DIRECTORY "${TEST_DIR}")

function(expect_cli_error label)
  execute_process(
    COMMAND "${SIMULATOR}" ${ARGN}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
  )
  if(NOT result EQUAL 2)
    message(FATAL_ERROR "${label}: expected exit 2, got ${result}\nstdout:\n${output}\nstderr:\n${error}")
  endif()
  if(NOT output STREQUAL "")
    message(FATAL_ERROR "${label}: incompatible options reached simulation output:\n${output}")
  endif()
endfunction()

function(expect_cli_success label)
  execute_process(
    COMMAND "${SIMULATOR}" ${ARGN}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
  )
  if(NOT result EQUAL 0)
    message(FATAL_ERROR "${label}: expected exit 0, got ${result}\nstdout:\n${output}\nstderr:\n${error}")
  endif()
endfunction()

set(aggregate_output "${TEST_DIR}/aggregate.csv")
set(ignored_aggregate_deadline "${TEST_DIR}/ignored-aggregate-deadline.csv")
set(ignored_aggregate_scenario "${TEST_DIR}/ignored-aggregate-scenario.csv")
set(ignored_aggregate_start_seed "${TEST_DIR}/ignored-aggregate-start-seed.csv")
set(ignored_trace_output "${TEST_DIR}/ignored-trace-output.csv")
set(ignored_find_output "${TEST_DIR}/ignored-find-output.csv")

# Preserve each supported mode and its mode-specific controls.
expect_cli_success(aggregate-valid --trials 1 --seed 0 --out "${aggregate_output}")
if(NOT EXISTS "${aggregate_output}")
  message(FATAL_ERROR "aggregate-valid: expected aggregate CSV was not created")
endif()
expect_cli_success(single-trace-valid --simulate-this --scenario strict-jit/go-second --seed 3)
expect_cli_success(find-ready-valid --find-ready 1 --start-seed 1 --scenario strict-jit/go-second)
expect_cli_success(self-test-valid --self-test)

# Aggregate mode may use trials, seed, and output. Trace-only selectors must fail
# before the aggregate CSV is written:
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-aggregate-smoke-test
expect_cli_error(aggregate-deadline --trials 1 --require-ready-by 1 --out "${ignored_aggregate_deadline}")
expect_cli_error(aggregate-scenario --trials 1 --scenario strict-jit/go-second --out "${ignored_aggregate_scenario}")
expect_cli_error(aggregate-start-seed --trials 1 --start-seed 1 --out "${ignored_aggregate_start_seed}")

# Single-trace mode may use scenario, seed, and a readiness deadline. Aggregate
# output controls and the find-ready start seed must fail before trace output:
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
expect_cli_error(trace-trials --simulate-this --trials 1)
expect_cli_error(trace-output --simulate-this --out "${ignored_trace_output}")
expect_cli_error(trace-start-seed --simulate-this --start-seed 1)

# Find-ready uses its requested count and start seed. Aggregate controls and the
# single-trace seed must fail, and two explicit mode selectors may not coexist:
# https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
expect_cli_error(find-trials --find-ready 1 --trials 1)
expect_cli_error(find-output --find-ready 1 --out "${ignored_find_output}")
expect_cli_error(find-seed --find-ready 1 --seed 999)
expect_cli_error(conflicting-trace-modes --simulate-this --find-ready 1)

# The active ready-state implementation requires turn 2 or later. A turn-one
# deadline is still a valid finite single-trace assertion, while accepting it as a
# find-ready predicate would make every seed miss forever:
# https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc#L168-L177
# https://github.com/FlareZ123/pokemon-sims/issues/683
execute_process(
  COMMAND "${SIMULATOR}" --find-ready 1 --require-ready-by 1
  RESULT_VARIABLE impossible_deadline_result
  OUTPUT_VARIABLE impossible_deadline_output
  ERROR_VARIABLE impossible_deadline_error
  TIMEOUT 30
)
if(NOT impossible_deadline_result EQUAL 2)
  message(FATAL_ERROR "find-ready-impossible-deadline: expected exit 2, got ${impossible_deadline_result}\nstdout:\n${impossible_deadline_output}\nstderr:\n${impossible_deadline_error}")
endif()
if(NOT impossible_deadline_output STREQUAL "")
  message(FATAL_ERROR "find-ready-impossible-deadline: invalid search reached simulation output:\n${impossible_deadline_output}")
endif()
string(FIND "${impossible_deadline_error}" "readiness begins on turn 2" impossible_deadline_message)
if(impossible_deadline_message EQUAL -1)
  message(FATAL_ERROR "find-ready-impossible-deadline: missing clear diagnostic\n${impossible_deadline_error}")
endif()

# A find-ready deadline is part of the search predicate. The confirmed projected
# Steven route now makes the first searched seed ready on T2:
# https://github.com/FlareZ123/pokemon-sims/issues/641
# https://github.com/FlareZ123/pokemon-sims/issues/898
# https://github.com/FlareZ123/pokemon-sims/issues/867
# https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
execute_process(
  COMMAND "${SIMULATOR}" --find-ready 1 --start-seed 1
          --scenario strict-jit/go-second --require-ready-by 2
  RESULT_VARIABLE find_deadline_result
  OUTPUT_VARIABLE find_deadline_output
  ERROR_VARIABLE find_deadline_error
)
if(NOT find_deadline_result EQUAL 0)
  message(FATAL_ERROR "find-ready-deadline: expected exit 0, got ${find_deadline_result}\nstdout:\n${find_deadline_output}\nstderr:\n${find_deadline_error}")
endif()
string(FIND "${find_deadline_output}" "Seed: 1 | first-ready turn: 2" find_seed_one)
if(find_seed_one EQUAL -1)
  message(FATAL_ERROR "find-ready-deadline: qualifying seed 1 was not reported\n${find_deadline_output}")
endif()
string(FIND "${find_deadline_output}" "Seed: 2 |" find_late_seed_two)
if(NOT find_late_seed_two EQUAL -1)
  message(FATAL_ERROR "find-ready-deadline: a later seed was reported before seed 1\n${find_deadline_output}")
endif()

# uint64_t arithmetic wraps modulo 2^N. Requesting two results from the maximum
# seed must report finite range exhaustion before a hidden restart at seed zero:
# https://eel.is/c++draft/basic.fundamental#4
# https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
# https://github.com/FlareZ123/pokemon-sims/issues/683
execute_process(
  COMMAND "${SIMULATOR}" --find-ready 2 --start-seed 18446744073709551615
          --scenario strict-jit/go-second
  RESULT_VARIABLE exhausted_seed_result
  OUTPUT_VARIABLE exhausted_seed_output
  ERROR_VARIABLE exhausted_seed_error
  TIMEOUT 30
)
if(NOT exhausted_seed_result EQUAL 1)
  message(FATAL_ERROR "find-ready-seed-exhaustion: expected exit 1, got ${exhausted_seed_result}\nstdout:\n${exhausted_seed_output}\nstderr:\n${exhausted_seed_error}")
endif()
string(FIND "${exhausted_seed_error}" "find-ready search exhausted the uint64 seed range" exhausted_seed_message)
if(exhausted_seed_message EQUAL -1)
  message(FATAL_ERROR "find-ready-seed-exhaustion: missing clear diagnostic\n${exhausted_seed_error}")
endif()

# Single-trace mode still reports its chosen seed and returns failure when that
# seed misses the requested deadline:
# https://github.com/FlareZ123/pokemon-sims/issues/641
execute_process(
  COMMAND "${SIMULATOR}" --simulate-this --seed 2
          --scenario strict-jit/go-second --require-ready-by 2
  RESULT_VARIABLE trace_deadline_result
  OUTPUT_VARIABLE trace_deadline_output
  ERROR_VARIABLE trace_deadline_error
)
if(NOT trace_deadline_result EQUAL 1)
  message(FATAL_ERROR "single-trace-deadline: expected exit 1, got ${trace_deadline_result}\nstdout:\n${trace_deadline_output}\nstderr:\n${trace_deadline_error}")
endif()
string(FIND "${trace_deadline_output}" "Seed: 2 | first-ready turn: 3" trace_seed_two)
if(trace_seed_two EQUAL -1)
  message(FATAL_ERROR "single-trace-deadline: selected seed 2 was not reported\n${trace_deadline_output}")
endif()

# Turn one remains a valid finite assertion in single-trace mode even though the
# active model cannot record readiness until turn 2:
# https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc#L168-L177
# https://github.com/FlareZ123/pokemon-sims/issues/683
execute_process(
  COMMAND "${SIMULATOR}" --simulate-this --seed 2
          --scenario strict-jit/go-second --require-ready-by 1
  RESULT_VARIABLE trace_turn_one_result
  OUTPUT_VARIABLE trace_turn_one_output
  ERROR_VARIABLE trace_turn_one_error
)
if(NOT trace_turn_one_result EQUAL 1)
  message(FATAL_ERROR "single-trace-turn-one-deadline: expected exit 1, got ${trace_turn_one_result}\nstdout:\n${trace_turn_one_output}\nstderr:\n${trace_turn_one_error}")
endif()
string(FIND "${trace_turn_one_output}" "Seed: 2 | first-ready turn: 3" trace_turn_one_seed)
if(trace_turn_one_seed EQUAL -1)
  message(FATAL_ERROR "single-trace-turn-one-deadline: selected seed 2 was not reported\n${trace_turn_one_output}")
endif()

# Self-test is its own execution mode and must not silently override parsed work.
expect_cli_error(self-test-trials --self-test --trials 1)
expect_cli_error(self-test-trace --self-test --simulate-this)
expect_cli_error(self-test-find --self-test --find-ready 1)

foreach(path IN ITEMS
    "${ignored_aggregate_deadline}"
    "${ignored_aggregate_scenario}"
    "${ignored_aggregate_start_seed}"
    "${ignored_trace_output}"
    "${ignored_find_output}")
  if(EXISTS "${path}")
    message(FATAL_ERROR "incompatible CLI invocation created ${path}")
  endif()
endforeach()
