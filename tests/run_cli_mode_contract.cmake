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

# A find-ready deadline is part of the search predicate. The command must skip the
# late-ready seed 2 and continue to the first deadline-qualified seed 6:
# https://github.com/FlareZ123/pokemon-sims/issues/641
# https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_016.inc
execute_process(
  COMMAND "${SIMULATOR}" --find-ready 1 --start-seed 1
      --scenario strict-jit/go-second --require-ready-by 2
  RESULT_VARIABLE deadline_find_result
  OUTPUT_VARIABLE deadline_find_output
  ERROR_VARIABLE deadline_find_error
)
if(NOT deadline_find_result EQUAL 0)
  message(FATAL_ERROR
      "deadline-find-ready: expected exit 0, got ${deadline_find_result}\n"
      "stdout:\n${deadline_find_output}\nstderr:\n${deadline_find_error}")
endif()
if(NOT deadline_find_output MATCHES "Seed: 6 \\| first-ready turn: 2")
  message(FATAL_ERROR
      "deadline-find-ready: expected the first qualifying seed 6\n"
      "stdout:\n${deadline_find_output}")
endif()
if(deadline_find_output MATCHES "Seed: 2 \\|")
  message(FATAL_ERROR
      "deadline-find-ready: late-ready seed 2 must be skipped\n"
      "stdout:\n${deadline_find_output}")
endif()

# Single-trace mode still reports the selected late-ready seed and exits 1:
# https://github.com/FlareZ123/pokemon-sims/issues/641
execute_process(
  COMMAND "${SIMULATOR}" --simulate-this --seed 2
      --scenario strict-jit/go-second --require-ready-by 2
  RESULT_VARIABLE deadline_trace_result
  OUTPUT_VARIABLE deadline_trace_output
  ERROR_VARIABLE deadline_trace_error
)
if(NOT deadline_trace_result EQUAL 1)
  message(FATAL_ERROR
      "deadline-single-trace: expected exit 1, got ${deadline_trace_result}\n"
      "stdout:\n${deadline_trace_output}\nstderr:\n${deadline_trace_error}")
endif()
if(NOT deadline_trace_output MATCHES "Seed: 2 \\| first-ready turn: 3")
  message(FATAL_ERROR
      "deadline-single-trace: selected seed 2 was not printed\n"
      "stdout:\n${deadline_trace_output}")
endif()
if(NOT deadline_trace_error MATCHES
    "Trace did not reach a ready state by required turn 2")
  message(FATAL_ERROR
      "deadline-single-trace: missing deadline failure\n"
      "stderr:\n${deadline_trace_error}")
endif()

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
