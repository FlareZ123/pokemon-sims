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

# Held Fire plus the next legal manual attachment guarantees GGF before Regidrago V
# can evolve. Held Serena plus Dialga-GX supplies the T2 strict-JIT payload, so seed 19
# must preserve the unresolved VSTAR and connector axes:
# Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
# Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
# Manual attachment, evolution, and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1079
run_trace("strict-jit/go-second" 19 strict_seed_19)
if(NOT strict_seed_19 MATCHES "T1 \\| HOLD ATTACK \\|")
  message(FATAL_ERROR "Strict seed 19 did not hold Celestial Roar:\n${strict_seed_19}")
endif()
if(strict_seed_19 MATCHES "T1 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "Strict seed 19 still used Celestial Roar:\n${strict_seed_19}")
endif()
if(NOT strict_seed_19 MATCHES "T2 \\| DISCARD \\|.*Dialga-GX")
  message(FATAL_ERROR "Strict seed 19 lost its held same-turn payload route:\n${strict_seed_19}")
endif()
if(NOT strict_seed_19 MATCHES "T2 \\| READY \\|")
  message(FATAL_ERROR "Strict seed 19 did not retain T2 readiness:\n${strict_seed_19}")
endif()

# With two Energy still missing, one future manual attachment does not guarantee GGF.
# Celestial Roar must remain available as a live acceleration route:
# https://api.pokemontcg.io/v2/cards/swsh12-135
# https://github.com/FlareZ123/pokemon-sims/issues/1079
run_trace("strict-jit/go-second" 4 strict_seed_4)
if(NOT strict_seed_4 MATCHES "T1 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "Strict seed 4 incorrectly suppressed a live Celestial Roar route:\n${strict_seed_4}")
endif()

# No-discard-control may bank an early Dragon payload, so the held-route suppression
# must remain limited to strict and matchup-flex JIT:
# https://api.pokemontcg.io/v2/cards/swsh12-135
# https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# https://github.com/FlareZ123/pokemon-sims/issues/1079
run_trace("no-discard-control/go-second" 19 control_seed_19)
if(NOT control_seed_19 MATCHES "T1 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "No-discard-control seed 19 lost its permitted payload-banking attack:\n${control_seed_19}")
endif()
