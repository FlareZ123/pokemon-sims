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

function(assert_serena_route scenario output_var)
  run_trace("${scenario}" 71 trace_output)
  if(NOT trace_output MATCHES "T2 \\| WONDER TAG \\|.*Serena")
    message(FATAL_ERROR "${scenario} seed 71 did not search Serena:\n${trace_output}")
  endif()
  if(NOT trace_output MATCHES "T2 \\| PLAY SUPPORTER \\|.*Serena")
    message(FATAL_ERROR "${scenario} seed 71 did not play Serena:\n${trace_output}")
  endif()
  if(NOT trace_output MATCHES "T2 \\| DISCARD \\|.*(Mega Dragonite ex|Dragapult ex|Hisuian Goodra VSTAR|Dialga-GX)")
    message(FATAL_ERROR "${scenario} seed 71 did not establish a held Dragon payload:\n${trace_output}")
  endif()
  if(trace_output MATCHES "T2 \\| WONDER TAG \\|.*Crispin")
    message(FATAL_ERROR "${scenario} seed 71 still selected unused Crispin:\n${trace_output}")
  endif()
  if(trace_output MATCHES "LEGACY STAR")
    message(FATAL_ERROR "${scenario} seed 71 still spent the game-wide VSTAR Power:\n${trace_output}")
  endif()
  if(NOT trace_output MATCHES "T2 \\| READY \\|")
    message(FATAL_ERROR "${scenario} seed 71 lost T2 readiness:\n${trace_output}")
  endif()
  set(${output_var} "${trace_output}" PARENT_SCOPE)
endfunction()

# Wonder Tag searches any Supporter. Fire in hand and the unused manual attachment
# already complete GGF, while Serena may discard a held Dragon payload. Serena is the
# deterministic current-turn completion route and preserves Legacy Star:
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
# Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
# Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
# Regidrago VSTAR and Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
# Supporter, evolution, and manual attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
# Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1086
assert_serena_route("strict-jit-turn2-item-lock/go-second" scheduled_lock_trace)
assert_serena_route("strict-jit-full-item-lock/go-second" full_lock_trace)

# When one manual attachment cannot complete GGF, Crispin remains the direct Energy
# connector and must retain priority over Serena:
# Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
# Regidrago VSTAR attack cost: https://api.pokemontcg.io/v2/cards/swsh12-136
# Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1086
run_trace("strict-jit/go-second" 41 crispin_control)
if(NOT crispin_control MATCHES "T1 \\| WONDER TAG \\|.*Crispin")
  message(FATAL_ERROR "Crispin-required control lost Wonder Tag into Crispin:\n${crispin_control}")
endif()
if(NOT crispin_control MATCHES "T1 \\| PLAY SUPPORTER \\|.*Crispin")
  message(FATAL_ERROR "Crispin-required control did not resolve Crispin:\n${crispin_control}")
endif()
if(NOT crispin_control MATCHES "T2 \\| READY \\|")
  message(FATAL_ERROR "Crispin-required control lost T2 readiness:\n${crispin_control}")
endif()
