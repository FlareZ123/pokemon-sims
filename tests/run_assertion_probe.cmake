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
