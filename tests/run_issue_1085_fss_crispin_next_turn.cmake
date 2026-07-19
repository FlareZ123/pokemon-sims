if(NOT DEFINED SIMULATOR)
  message(FATAL_ERROR "SIMULATOR is required")
endif()

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
run_trace("strict-jit-turn2-item-lock/go-second" 25 issue_1085_seed_25)
if(NOT issue_1085_seed_25 MATCHES "T1 \\| STAR ALCHEMY \\|.*Crispin")
  message(FATAL_ERROR "Seed 25 did not allocate Star Alchemy to Crispin:\n${issue_1085_seed_25}")
endif()
if(NOT issue_1085_seed_25 MATCHES "T1 \\| PLAY SUPPORTER \\|.*Crispin")
  message(FATAL_ERROR "Seed 25 did not resolve the deterministic T1 Crispin line:\n${issue_1085_seed_25}")
endif()
if(issue_1085_seed_25 MATCHES "T1 \\| BENCH \\|.*Oricorio")
  message(FATAL_ERROR "Seed 25 still spent the Bench slot on Oricorio:\n${issue_1085_seed_25}")
endif()
if(issue_1085_seed_25 MATCHES "T1 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "Seed 25 still depended on Celestial Roar variance:\n${issue_1085_seed_25}")
endif()
if(NOT issue_1085_seed_25 MATCHES "T2 \\| PLAY SUPPORTER \\|.*Professor Burnet")
  message(FATAL_ERROR "Seed 25 lost the held T2 Burnet payload bridge:\n${issue_1085_seed_25}")
endif()
if(issue_1085_seed_25 MATCHES "LEGACY STAR")
  message(FATAL_ERROR "Seed 25 unnecessarily spent the game-wide VSTAR Power:\n${issue_1085_seed_25}")
endif()
if(NOT issue_1085_seed_25 MATCHES "T2 \\| READY \\|")
  message(FATAL_ERROR "Seed 25 did not retain deterministic T2 readiness:\n${issue_1085_seed_25}")
endif()

# Going first blocks a T1 Supporter. The established issue-1071 decomposition must
# therefore keep Star Alchemy on Oricorio for this paired seed:
# First-turn Supporter restriction: https://www.pokemon.com/us/pokemon-tcg/rules
# Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
# Existing route: https://github.com/FlareZ123/pokemon-sims/issues/1071
# Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1085
run_trace("strict-jit/go-first" 25 issue_1085_going_first_control)
if(NOT issue_1085_going_first_control MATCHES "T1 \\| STAR ALCHEMY \\|.*Oricorio")
  message(FATAL_ERROR "Going-first control lost the established Oricorio route:\n${issue_1085_going_first_control}")
endif()
if(issue_1085_going_first_control MATCHES "T1 \\| PLAY SUPPORTER \\|.*Crispin")
  message(FATAL_ERROR "Going-first control illegally played Crispin on T1:\n${issue_1085_going_first_control}")
endif()
