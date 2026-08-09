if(NOT DEFINED SIMULATOR)
  message(FATAL_ERROR "SIMULATOR is required")
endif()

set(ISSUE_1085_SEED 25)
set(ISSUE_1085_ITEM_LOCK_SCENARIO "strict-jit-turn2-item-lock/go-second")
set(ISSUE_1085_GO_FIRST_CONTROL_SCENARIO "strict-jit/go-first")

function(run_trace scenario seed output_var)
  execute_process(
    COMMAND "${SIMULATOR}" --simulate-this --scenario "${scenario}" --seed "${seed}"
    RESULT_VARIABLE status
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error
  )
  if(NOT status EQUAL 0)
    message(FATAL_ERROR "Trace failed for ${scenario} seed ${seed}: ${error}\n${output}")
  endif()
  set(${output_var} "${output}" PARENT_SCOPE)
endfunction()

function(require_trace_match trace_output pattern error_message)
  if(NOT "${trace_output}" MATCHES "${pattern}")
    message(FATAL_ERROR "${error_message}:\n${trace_output}")
  endif()
endfunction()

# Star Alchemy can search any card. In the scheduled T2 Item-lock route, Crispin plus
# the current and next legal manual attachments deterministically completes GGF, and
# the held Professor Burnet establishes the strict-JIT payload on T2. Oricorio plus
# Celestial Roar depends on unknown top cards and is therefore a weaker route:
# Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
# Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
# Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
# Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Supporter, attachment, attack, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# Future-card oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1085
run_trace("${ISSUE_1085_ITEM_LOCK_SCENARIO}" "${ISSUE_1085_SEED}" issue_1085_seed_25)
require_trace_match(
  "${issue_1085_seed_25}"
  "T1 \\| STAR ALCHEMY \\|.*Crispin"
  "Seed ${ISSUE_1085_SEED} did not allocate Star Alchemy to Crispin"
)
require_trace_match(
  "${issue_1085_seed_25}"
  "T1 \\| PLAY SUPPORTER \\|.*Crispin"
  "Seed ${ISSUE_1085_SEED} did not resolve the deterministic T1 Crispin line"
)
if(issue_1085_seed_25 MATCHES "T1 \\| BENCH \\|.*Oricorio")
  message(FATAL_ERROR "Seed ${ISSUE_1085_SEED} still spent the Bench slot on Oricorio:\n${issue_1085_seed_25}")
endif()
if(issue_1085_seed_25 MATCHES "T1 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "Seed ${ISSUE_1085_SEED} still depended on Celestial Roar variance:\n${issue_1085_seed_25}")
endif()
require_trace_match(
  "${issue_1085_seed_25}"
  "T2 \\| PLAY SUPPORTER \\|.*Professor Burnet"
  "Seed ${ISSUE_1085_SEED} lost the held T2 Burnet payload bridge"
)
if(issue_1085_seed_25 MATCHES "LEGACY STAR")
  message(FATAL_ERROR "Seed ${ISSUE_1085_SEED} unnecessarily spent the game-wide VSTAR Power:\n${issue_1085_seed_25}")
endif()
require_trace_match(
  "${issue_1085_seed_25}"
  "T2 \\| READY \\|"
  "Seed ${ISSUE_1085_SEED} did not retain deterministic T2 readiness"
)

# Going first blocks a T1 Supporter. The established issue-1071 decomposition must
# therefore keep Star Alchemy on Oricorio for this paired seed:
# First-turn Supporter restriction: https://www.pokemon.com/us/pokemon-tcg/rules
# Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
# Existing route: https://github.com/FlareZ123/pokemon-sims/issues/1071
# Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1085
run_trace("${ISSUE_1085_GO_FIRST_CONTROL_SCENARIO}" "${ISSUE_1085_SEED}" issue_1085_going_first_control)
require_trace_match(
  "${issue_1085_going_first_control}"
  "T1 \\| STAR ALCHEMY \\|.*Oricorio"
  "Going-first control lost the established Oricorio route"
)
if(issue_1085_going_first_control MATCHES "T1 \\| PLAY SUPPORTER \\|.*Crispin")
  message(FATAL_ERROR "Going-first control illegally played Crispin on T1:\n${issue_1085_going_first_control}")
endif()
