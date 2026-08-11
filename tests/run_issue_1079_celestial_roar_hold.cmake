if(NOT DEFINED SIMULATOR)
  message(FATAL_ERROR "SIMULATOR is required")
endif()

set(STRICT_SECOND_SCENARIO "strict-jit/go-second")
set(MATCHUP_FLEX_FIRST_SCENARIO "matchup-flex-jit/go-first")
set(NO_CONTROL_SECOND_SCENARIO "no-discard-control/go-second")
set(STRICT_HOLD_SEED 19)
set(MATCHUP_FLEX_K0_SEED 2026072802)
set(STRICT_ACCELERATION_SEED 4)
set(NO_CONTROL_HOLD_SEED 91)
set(NO_CONTROL_PAYLOAD_SEED 19)

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
# can evolve. #1079 therefore holds Celestial Roar. On T2, #2408 prefers Professor
# Burnet's K1 deck-to-discard payload route over equal-turn Serena, preserving Serena
# plus held Dialga-GX while retaining the same T2 strict-JIT ready turn:
# Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
# Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
# Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
# Manual attachment, evolution, Supporter, search, and discard procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Strict-JIT timing and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# Celestial Roar hold bug: https://github.com/FlareZ123/pokemon-sims/issues/1079
# Equal-turn resource bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
run_trace("${STRICT_SECOND_SCENARIO}" ${STRICT_HOLD_SEED} strict_seed_19)
if(NOT strict_seed_19 MATCHES "T1 \\| HOLD ATTACK \\|")
  message(FATAL_ERROR "Strict seed 19 did not hold Celestial Roar:\n${strict_seed_19}")
endif()
if(strict_seed_19 MATCHES "T1 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "Strict seed 19 still used Celestial Roar:\n${strict_seed_19}")
endif()
if(NOT strict_seed_19 MATCHES "T2 \\| PLAY SUPPORTER \\|.*Professor Burnet")
  message(FATAL_ERROR "Strict seed 19 lost its resource-preserving Burnet payload route:\n${strict_seed_19}")
endif()
if(strict_seed_19 MATCHES "T2 \\| DISCARD \\|.*Dialga-GX.*Serena")
  message(FATAL_ERROR "Strict seed 19 regressed to spending Serena and held Dialga-GX:\n${strict_seed_19}")
endif()
if(NOT strict_seed_19 MATCHES "T2 \\| READY \\|")
  message(FATAL_ERROR "Strict seed 19 did not retain T2 readiness:\n${strict_seed_19}")
endif()

# Audited-main seed 2026072802 is the K0 regression witness. Matchup-flex JIT has
# spent the T2 manual attachment, holds the sole missing Fire Energy, and has no
# observed VSTAR route. Randomly processing three unknown cards cannot improve the
# marginal chance that a later draw is Regidrago VSTAR, while the attack can discard
# VSTAR copies and connectors. Holding preserves the fixed T4 Burnet route:
# Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
# Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
# Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
# Core attachment, evolution, Supporter, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
# Hidden-order boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
# Earliest-route and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# Reopened regression: https://github.com/FlareZ123/pokemon-sims/issues/1079
# Known-route control: https://github.com/FlareZ123/pokemon-sims/issues/1174
run_trace("${MATCHUP_FLEX_FIRST_SCENARIO}" ${MATCHUP_FLEX_K0_SEED} matchup_flex_k0_regression)
if(NOT matchup_flex_k0_regression MATCHES "T2 \\| HOLD ATTACK \\|")
  message(FATAL_ERROR "Matchup-flex K0 regression seed did not hold Celestial Roar:\n${matchup_flex_k0_regression}")
endif()
if(matchup_flex_k0_regression MATCHES "T2 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "Matchup-flex K0 regression seed still used Celestial Roar:\n${matchup_flex_k0_regression}")
endif()
if(NOT matchup_flex_k0_regression MATCHES "T4 \\| READY \\|")
  message(FATAL_ERROR "Matchup-flex K0 regression seed did not preserve the T4 ready route:\n${matchup_flex_k0_regression}")
endif()

# With two Energy still missing, one future manual attachment does not guarantee GGF.
# Celestial Roar must remain available as a live acceleration route:
# https://api.pokemontcg.io/v2/cards/swsh12-135
# https://github.com/FlareZ123/pokemon-sims/issues/1079
run_trace("${STRICT_SECOND_SCENARIO}" ${STRICT_ACCELERATION_SEED} strict_seed_4)
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
run_trace("${NO_CONTROL_SECOND_SCENARIO}" ${NO_CONTROL_HOLD_SEED} no_control_seed_91)
# CMake quoted-argument escapes and MATCHES regex semantics: https://cmake.org/cmake/help/latest/manual/cmake-language.7.html#quoted-argument https://cmake.org/cmake/help/latest/command/if.html#matches
if(NOT no_control_seed_91 MATCHES "T1 \\| HOLD ATTACK \\|")
  message(FATAL_ERROR "No-control seed 91 did not hold Celestial Roar:\n${no_control_seed_91}")
endif()
if(no_control_seed_91 MATCHES "T1 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "No-control seed 91 still used Celestial Roar:\n${no_control_seed_91}")
endif()
if(NOT no_control_seed_91 MATCHES "T2 \\| READY \\|")
  message(FATAL_ERROR "No-control seed 91 lost T2 readiness:\n${no_control_seed_91}")
endif()

# No-discard-control may still bank an early Dragon payload when that axis is
# missing, so the existing seed-19 attack remains a required positive control:
# https://api.pokemontcg.io/v2/cards/swsh12-135
# https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# https://github.com/FlareZ123/pokemon-sims/issues/1079
# https://github.com/FlareZ123/pokemon-sims/issues/1623
run_trace("${NO_CONTROL_SECOND_SCENARIO}" ${NO_CONTROL_PAYLOAD_SEED} control_seed_19)
if(NOT control_seed_19 MATCHES "T1 \\| ATTACK \\|.*Celestial Roar")
  message(FATAL_ERROR "No-discard-control seed 19 lost its permitted payload-banking attack:\n${control_seed_19}")
endif()
