# Exact current-main regression for issue 809. The stronger route searches the
# remaining Tapu Lele-GX after K1 reveals one prized copy, banks the remaining
# Crispin with Wonder Tag, and reaches the same T2 ready state without spending
# Forest Seal Stone or Vital Dance.
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
# Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
# Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
# Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
# First-turn Supporter rule: https://www.pokemon.com/us/pokemon-tcg/rules
# Resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# Regression: https://github.com/FlareZ123/pokemon-sims/issues/809
if(NOT DEFINED SIMULATOR)
  message(FATAL_ERROR "SIMULATOR was not supplied")
endif()

execute_process(
  COMMAND "${SIMULATOR}" --simulate-this
          --scenario no-discard-control/go-first
          --seed 7
          --require-ready-by 2
  RESULT_VARIABLE trace_result
  OUTPUT_VARIABLE trace
  ERROR_VARIABLE trace_error
  TIMEOUT 60
)
if(NOT trace_result EQUAL 0)
  message(FATAL_ERROR "Issue 809 trace failed with ${trace_result}\n${trace}\n${trace_error}")
endif()

foreach(required_text
    "first-ready turn: 2"
    "Searched a Basic Pokémon: Tapu Lele-GX."
    "T1 | WONDER TAG | rules: R-TAPU-01 | Searched and revealed Crispin."
    "T2 | READY | rules: R-RVS-01; P-JIT-01")
  string(FIND "${trace}" "${required_text}" required_position)
  if(required_position EQUAL -1)
    message(FATAL_ERROR "Issue 809 trace is missing: ${required_text}\n${trace}")
  endif()
endforeach()

foreach(forbidden_text
    "Searched a Basic Pokémon: Oricorio GRI 55."
    "T2 | STAR ALCHEMY")
  string(FIND "${trace}" "${forbidden_text}" forbidden_position)
  if(NOT forbidden_position EQUAL -1)
    message(FATAL_ERROR "Issue 809 trace spent the dominated resource: ${forbidden_text}\n${trace}")
  endif()
endforeach()

message(STATUS "Issue 809 trace:\n${trace}")
