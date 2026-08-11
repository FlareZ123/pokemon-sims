if(NOT DEFINED SIMULATOR)
  message(FATAL_ERROR "SIMULATOR is required")
endif()

set(ISSUE_1099_SCENARIO "strict-jit/go-first")
set(ISSUE_1099_SEED 132)

execute_process(
  COMMAND "${SIMULATOR}" --simulate-this --scenario "${ISSUE_1099_SCENARIO}" --seed "${ISSUE_1099_SEED}"
  RESULT_VARIABLE issue_1099_status
  OUTPUT_VARIABLE issue_1099_trace
  ERROR_VARIABLE issue_1099_error
)
if(NOT issue_1099_status EQUAL 0)
  message(FATAL_ERROR "Issue 1099 seed ${ISSUE_1099_SEED} trace failed: ${issue_1099_error}\n${issue_1099_trace}")
endif()

# The exact-state Serena fixture preserves the incomplete-Energy-axis boundary.
# This independent source-bound trace verifies the same DCI ordering later in a game:
# Serena must spend the observably dead Latias ex before a Dragon that supplies the
# same-turn Quick Ball payload after Active Regidrago VSTAR already has GGF:
# Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
# Official Supporter and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1099
if(issue_1099_trace MATCHES "T5 \\| DISCARD \\|.*Mega Dragonite ex \\(Serena chosen discard\\)")
  message(FATAL_ERROR "Seed ${ISSUE_1099_SEED} still discarded a strict-JIT Dragon before GGF could finish:\n${issue_1099_trace}")
endif()
if(NOT issue_1099_trace MATCHES "T5 \\| DISCARD \\|.*Latias ex \\(Serena chosen discard\\)")
  message(FATAL_ERROR "Seed ${ISSUE_1099_SEED} did not use the observably dead Latias ex Serena cost:\n${issue_1099_trace}")
endif()
if(NOT issue_1099_trace MATCHES "T5 \\| DISCARD \\|.*Quick Ball cost")
  message(FATAL_ERROR "Seed ${ISSUE_1099_SEED} lost its legal ready-turn Quick Ball payload outlet:\n${issue_1099_trace}")
endif()
if(NOT issue_1099_trace MATCHES "T5 \\| READY \\|")
  message(FATAL_ERROR "Seed ${ISSUE_1099_SEED} lost T5 readiness after the Serena DCI fix:\n${issue_1099_trace}")
endif()
