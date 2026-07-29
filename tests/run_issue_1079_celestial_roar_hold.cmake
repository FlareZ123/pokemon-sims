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


# Current-main seed 2026072802 is the regression witness. Matchup-flex JIT has
# spent the T2 manual attachment, holds the sole missing Fire Energy, and has no
# T2 Regidrago VSTAR route. Celestial Roar cannot improve the marginal next-draw
# VSTAR chance or bank a later JIT payload, so it must preserve the top three.
# The fixed hidden order then draws Gladion on T3 and the preserved VSTAR on T4,
# where Professor Burnet establishes the same-turn payload:
# Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
# Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
# Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
# Core attachment, evolution, Supporter, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# Strict and matchup-flex JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
# Earliest-route and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# Reopened confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/1079
run_trace("matchup-flex-jit/go-first" 2026072802 matchup_flex_regression)
if(NOT matchup_flex_regression MATCHES "T2 \\| HOLD ATTACK \\|")
  message(FATAL_ERROR "Matchup-flex regression seed did not hold Celestial Roar:\n${matchup_flex_regression}")
endif()
if(matchup_flex_regression MATCHES "T2 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "Matchup-flex regression seed still used Celestial Roar:\n${matchup_flex_regression}")
endif()
if(NOT matchup_flex_regression MATCHES "T4 \\| READY \\|")
  message(FATAL_ERROR "Matchup-flex regression seed did not preserve the T4 ready route:\n${matchup_flex_regression}")
endif()

# With two Energy still missing, one future manual attachment does not guarantee GGF.
# Celestial Roar must remain available as a live acceleration route:
# https://api.pokemontcg.io/v2/cards/swsh12-135
# https://github.com/FlareZ123/pokemon-sims/issues/1079
run_trace("strict-jit/go-second" 4 strict_seed_4)
if(NOT strict_seed_4 MATCHES "T1 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "Strict seed 4 incorrectly suppressed a live Celestial Roar route:\n${strict_seed_4}")
endif()

# Once no-discard-control already has its Dragon payload in discard, held Fire and
# a held Regidrago VSTAR guarantee the next legal T2 ready window. Seed 91 must
# preserve the random top three because the attack cannot improve Energy or payload:
# Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
# Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
# Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
# Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
# Core attachment, evolution, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# No-control timing and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1623
run_trace("no-discard-control/go-second" 91 no_control_seed_91)
if(NOT no_control_seed_91 MATCHES "T1 \| HOLD ATTACK \|")
  message(FATAL_ERROR "No-control seed 91 did not hold Celestial Roar:
${no_control_seed_91}")
endif()
if(no_control_seed_91 MATCHES "T1 \| ATTACK \|.*Celestial Roar")
  message(FATAL_ERROR "No-control seed 91 still used Celestial Roar:
${no_control_seed_91}")
endif()
if(NOT no_control_seed_91 MATCHES "T2 \| READY \|")
  message(FATAL_ERROR "No-control seed 91 lost T2 readiness:
${no_control_seed_91}")
endif()

# No-discard-control may still bank an early Dragon payload when that axis is
# missing, so the existing seed-19 attack remains a required positive control:
# https://api.pokemontcg.io/v2/cards/swsh12-135
# https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# https://github.com/FlareZ123/pokemon-sims/issues/1079
# https://github.com/FlareZ123/pokemon-sims/issues/1623
run_trace("no-discard-control/go-second" 19 control_seed_19)
if(NOT control_seed_19 MATCHES "T1 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "No-discard-control seed 19 lost its permitted payload-banking attack:\n${control_seed_19}")
endif()
