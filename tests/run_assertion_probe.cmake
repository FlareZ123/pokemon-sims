if(NOT DEFINED ASSERTION_PROBE)
  message(FATAL_ERROR "ASSERTION_PROBE was not supplied")
endif()

# Run the probe as a child so its intentional assertion failure becomes a stable
# CTest result instead of a platform-specific signal result:
# https://cmake.org/cmake/help/latest/command/execute_process.html
execute_process(COMMAND "${ASSERTION_PROBE}" RESULT_VARIABLE assertion_result)
if("${assertion_result}" STREQUAL "0")
  message(FATAL_ERROR "The false assertion returned success; NDEBUG disabled the test check")
endif()

# The existing base-branch pull-request workflow executes this CTest in Release
# and sanitizer builds. Derive the sibling simulator path and run the current-main
# trace/matrix audit without adding a workflow that exists only on the PR branch:
# https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/ci.yml
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-aggregate-smoke-test
get_filename_component(audit_build_dir "${ASSERTION_PROBE}" DIRECTORY)
get_filename_component(audit_executable_suffix "${ASSERTION_PROBE}" EXT)
set(SIMULATOR "${audit_build_dir}/regidrago_sim${audit_executable_suffix}")
set(AUDIT_OUTPUT_DIR "${audit_build_dir}/current-main-trace-matrix-audit")
include("${CMAKE_CURRENT_LIST_DIR}/run_current_main_trace_matrix_audit.cmake")
