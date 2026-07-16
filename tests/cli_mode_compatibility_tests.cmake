# CLI modes are documented as separate command forms:
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-aggregate-smoke-test
# https://github.com/FlareZ123/pokemon-sims/issues/633

if(NOT DEFINED SIMULATOR)
  message(FATAL_ERROR "SIMULATOR is required")
endif()
if(NOT DEFINED WORK_DIR)
  message(FATAL_ERROR "WORK_DIR is required")
endif()

file(MAKE_DIRECTORY "${WORK_DIR}")

function(expect_cli_error case_name)
  execute_process(
    COMMAND "${SIMULATOR}" ${ARGN}
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
  )
  if(NOT result EQUAL 2)
    message(FATAL_ERROR
      "${case_name} returned ${result}, expected 2\nstdout:\n${output}\nstderr:\n${error}")
  endif()
endfunction()

set(aggregate_output "${WORK_DIR}/ignored-aggregate.csv")
file(REMOVE "${aggregate_output}")
expect_cli_error(
  aggregate_rejects_trace_options
  --trials 1
  --out "${aggregate_output}"
  --scenario strict-jit/go-second
  --require-ready-by 3
  --start-seed 6
)
if(EXISTS "${aggregate_output}")
  message(FATAL_ERROR "aggregate mode created output before rejecting trace options")
endif()

set(single_output "${WORK_DIR}/ignored-single.csv")
file(REMOVE "${single_output}")
expect_cli_error(
  single_trace_rejects_aggregate_options
  --simulate-this
  --scenario strict-jit/go-second
  --seed 6
  --trials 1
  --out "${single_output}"
  --start-seed 6
)
if(EXISTS "${single_output}")
  message(FATAL_ERROR "single-trace mode created an ignored output file")
endif()

set(find_output "${WORK_DIR}/ignored-find.csv")
file(REMOVE "${find_output}")
expect_cli_error(
  find_ready_rejects_other_seed_and_aggregate_options
  --find-ready 1
  --scenario strict-jit/go-second
  --start-seed 6
  --seed 6
  --trials 1
  --out "${find_output}"
)
if(EXISTS "${find_output}")
  message(FATAL_ERROR "find-ready mode created an ignored output file")
endif()

expect_cli_error(
  conflicting_trace_modes
  --simulate-this
  --find-ready 1
  --scenario strict-jit/go-second
  --start-seed 6
)

expect_cli_error(
  self_test_rejects_execution_options
  --self-test
  --seed 6
)

execute_process(
  COMMAND "${SIMULATOR}"
    --find-ready 1
    --scenario strict-jit/go-second
    --start-seed 6
    --require-ready-by 4
  RESULT_VARIABLE valid_find_result
  OUTPUT_VARIABLE valid_find_output
  ERROR_VARIABLE valid_find_error
)
if(NOT valid_find_result EQUAL 0)
  message(FATAL_ERROR
    "documented find-ready mode failed with ${valid_find_result}\n"
    "stdout:\n${valid_find_output}\nstderr:\n${valid_find_error}")
endif()
