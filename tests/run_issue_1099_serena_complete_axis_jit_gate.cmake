if(NOT DEFINED SIMULATOR)
  message(FATAL_ERROR "SIMULATOR is required")
endif()

execute_process(
  COMMAND "${SIMULATOR}" --simulate-this --scenario strict-jit/go-first --seed 44 --require-ready-by 3
  RESULT_VARIABLE status
  OUTPUT_VARIABLE trace
  ERROR_VARIABLE error
)
if(NOT status EQUAL 0)
  message(FATAL_ERROR "Issue 1099 seed 44 trace failed: ${error}\n${trace}")
endif()

# Serena consumes its Supporter slot before later actions. With Active Regidrago
# VSTAR at F and two Grass still missing, one Serena draw plus one manual attachment
# cannot finish Apex Dragon's GGF cost on turn two. Preserve the Dragon and use the
# observably dead Latias ex role as the mandatory discard instead:
# Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
# Official Supporter and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1099
if(trace MATCHES "T2 \\| DISCARD \\|.*Mega Dragonite ex \\(Serena chosen discard\\)")
  message(FATAL_ERROR "Seed 44 still discarded a strict-JIT Dragon before GGF could finish:\n${trace}")
endif()
if(NOT trace MATCHES "T2 \\| DISCARD \\|.*Latias ex \\(Serena chosen discard\\)")
  message(FATAL_ERROR "Seed 44 did not use the observably dead Latias ex Serena cost:\n${trace}")
endif()
if(NOT trace MATCHES "T3 \\| DISCARD \\|.*Quick Ball cost")
  message(FATAL_ERROR "Seed 44 lost its legal ready-turn Quick Ball payload outlet:\n${trace}")
endif()
if(NOT trace MATCHES "T3 \\| READY \\|")
  message(FATAL_ERROR "Seed 44 lost T3 readiness after the Serena DCI fix:\n${trace}")
endif()
