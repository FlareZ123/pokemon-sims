# Exact current-main regression for issue 802. Tapu Lele-GX has a one-Colorless
# Retreat Cost. The public T4 state holds Fire Energy, has an unused manual
# attachment and retreat, and has an already-GGF Benched Regidrago VSTAR with a
# permitted Dragon payload in discard.
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Official retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# Resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# Regression: https://github.com/FlareZ123/pokemon-sims/issues/802
if(NOT DEFINED SIMULATOR)
  message(FATAL_ERROR "SIMULATOR was not supplied")
endif()

execute_process(
  COMMAND "${SIMULATOR}" --simulate-this
          --scenario no-discard-control/go-first
          --seed 1
          --require-ready-by 4
  RESULT_VARIABLE trace_result
  OUTPUT_VARIABLE trace
  ERROR_VARIABLE trace_error
  TIMEOUT 60
)
if(NOT trace_result EQUAL 0)
  message(FATAL_ERROR "Issue 802 trace failed with ${trace_result}\n${trace}\n${trace_error}")
endif()

foreach(required_text
    "first-ready turn: 4"
    "T4 | ATTACH | rules: R-GAME-ENERGY | Fire Energy manually to Tapu Lele-GX for its Retreat Cost."
    "T4 | RETREAT | rules: R-GAME-RETREAT | Paid Tapu Lele-GX's one-Energy Retreat Cost and promoted the ready Benched Regidrago VSTAR."
    "T4 | READY | rules: R-RVS-01; P-JIT-01")
  string(FIND "${trace}" "${required_text}" required_position)
  if(required_position EQUAL -1)
    message(FATAL_ERROR "Issue 802 trace is missing: ${required_text}\n${trace}")
  endif()
endforeach()

foreach(forbidden_text
    "Latias ex gives the Basic Active no Retreat Cost"
    "Tate & Liza switch mode")
  string(FIND "${trace}" "${forbidden_text}" forbidden_position)
  if(NOT forbidden_position EQUAL -1)
    message(FATAL_ERROR "Issue 802 trace used a weaker alternate route: ${forbidden_text}\n${trace}")
  endif()
endforeach()

message(STATUS "Issue 802 trace:\n${trace}")
