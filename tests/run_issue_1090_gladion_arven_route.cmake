if(NOT DEFINED SIMULATOR)
  message(FATAL_ERROR "SIMULATOR is required")
endif()

execute_process(
  COMMAND "${SIMULATOR}" --simulate-this --scenario strict-jit/go-second --seed 862 --require-ready-by 3
  RESULT_VARIABLE status
  OUTPUT_VARIABLE trace
  ERROR_VARIABLE error)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "Issue #1090 trace failed (${status}):\n${trace}\n${error}")
endif()

foreach(required
    "T1 | PLAY SUPPORTER | rules: R-GLADION-01"
    "exchanged Gladion for Regidrago V"
    "T2 | PLAY SUPPORTER | rules: R-ARVEN-01"
    "T3 | PLAY SUPPORTER | rules: R-SERENA-01"
    "T3 | READY")
  string(FIND "${trace}" "${required}" found)
  if(found EQUAL -1)
    message(FATAL_ERROR "Issue #1090 trace is missing '${required}':\n${trace}")
  endif()
endforeach()

# Gladion legally reveals the Prize cards; Arven's searched Item must be genuinely
# payable before it can suppress that action:
# https://api.pokemontcg.io/v2/cards/sm4-95
# https://api.pokemontcg.io/v2/cards/sv1-166
# https://api.pokemontcg.io/v2/cards/swsh1-179
# https://api.pokemontcg.io/v2/cards/sm6-113
# https://github.com/FlareZ123/pokemon-sims/issues/1090
