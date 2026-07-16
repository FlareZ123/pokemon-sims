# Re-run three deterministic readable traces and the canonical aggregate matrix in
# pull-request CI, then compare every byte with the provenance-locked baseline.
# This validation file intentionally changes no simulator or policy behavior.
# Readable trace interface: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
# Aggregate interface: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-aggregate-smoke-test
# Knowledge contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
# Earliest-ready objective: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scope
# Baseline manifest: https://github.com/FlareZ123/pokemon-sims/blob/main/results/baseline_manifest.json
# Baseline drift issue: https://github.com/FlareZ123/pokemon-sims/issues/804
if(NOT DEFINED SIMULATOR)
  message(FATAL_ERROR "SIMULATOR was not supplied")
endif()
if(NOT DEFINED AUDIT_OUTPUT_DIR)
  message(FATAL_ERROR "AUDIT_OUTPUT_DIR was not supplied")
endif()

set(REPOSITORY_ROOT "${CMAKE_CURRENT_LIST_DIR}/..")
file(REMOVE_RECURSE "${AUDIT_OUTPUT_DIR}")
file(MAKE_DIRECTORY "${AUDIT_OUTPUT_DIR}")
set_property(GLOBAL PROPERTY CURRENT_MAIN_AUDIT_FAILURES "")

function(record_audit_failure text)
  get_property(existing GLOBAL PROPERTY CURRENT_MAIN_AUDIT_FAILURES)
  set_property(GLOBAL PROPERTY CURRENT_MAIN_AUDIT_FAILURES "${existing}\n${text}")
endfunction()

function(run_and_compare_trace label scenario seed deadline expected_name)
  set(actual_path "${AUDIT_OUTPUT_DIR}/${expected_name}")
  set(expected_path "${REPOSITORY_ROOT}/results/traces/${expected_name}")
  execute_process(
    COMMAND "${SIMULATOR}" --simulate-this --scenario "${scenario}"
            --seed "${seed}" --require-ready-by "${deadline}"
    RESULT_VARIABLE trace_result
    OUTPUT_FILE "${actual_path}"
    ERROR_VARIABLE trace_error
    TIMEOUT 60
  )
  if(NOT trace_result EQUAL 0)
    file(READ "${actual_path}" actual_trace)
    message(FATAL_ERROR "${label}: trace failed with ${trace_result}\n${actual_trace}\nstderr:\n${trace_error}")
  endif()
  file(READ "${actual_path}" actual_trace)
  message(STATUS "${label} current-main trace:\n${actual_trace}")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${actual_path}" "${expected_path}"
    RESULT_VARIABLE compare_result
  )
  if(NOT compare_result EQUAL 0)
    record_audit_failure("${label}: current executable differs from results/traces/${expected_name}")
  endif()
endfunction()

# Strict current-turn payload timing, going first.
# Strict-JIT specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
run_and_compare_trace(
  strict-jit-go-first
  strict-jit/go-first
  6
  4
  strict_jit_go_first_seed_6.txt
)

# Matchup-flex current-turn payload timing, going second.
# DCI/JIT treatment: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
run_and_compare_trace(
  matchup-flex-jit-go-second
  matchup-flex-jit/go-second
  3
  2
  matchup_flex_go_second_seed_3.txt
)

# No-discard-control payload banking, going first.
# Resource-policy specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
run_and_compare_trace(
  no-discard-control-go-first
  no-discard-control/go-first
  1
  4
  no_discard_control_go_first_seed_1.txt
)

# The sanitizer job repeats all three traces. Run the 1.4 million simulated hands
# only in the Release build and compare the full 14-condition matrix byte for byte.
string(FIND "${SIMULATOR}" "build-sanitizers" sanitizer_path_marker)
if(sanitizer_path_marker EQUAL -1)
  set(actual_matrix "${AUDIT_OUTPUT_DIR}/simulation_results.csv")
  set(expected_matrix "${REPOSITORY_ROOT}/results/simulation_results.csv")
  execute_process(
    COMMAND "${SIMULATOR}" --trials 100000 --seed 20260705 --out "${actual_matrix}"
    RESULT_VARIABLE matrix_result
    ERROR_VARIABLE matrix_error
    TIMEOUT 300
  )
  if(NOT matrix_result EQUAL 0)
    message(FATAL_ERROR "canonical matrix failed with ${matrix_result}\nstderr:\n${matrix_error}")
  endif()
  file(READ "${actual_matrix}" actual_matrix_text)
  message(STATUS "current-main 100000-trial matrix:\n${actual_matrix_text}")
  execute_process(
    COMMAND "${CMAKE_COMMAND}" -E compare_files "${actual_matrix}" "${expected_matrix}"
    RESULT_VARIABLE matrix_compare_result
  )
  if(NOT matrix_compare_result EQUAL 0)
    record_audit_failure("canonical matrix differs from results/simulation_results.csv")
  endif()
endif()

get_property(audit_failures GLOBAL PROPERTY CURRENT_MAIN_AUDIT_FAILURES)
if(NOT audit_failures STREQUAL "")
  message(FATAL_ERROR "Current-main baseline audit found drift:${audit_failures}\nSee https://github.com/FlareZ123/pokemon-sims/issues/804")
endif()
