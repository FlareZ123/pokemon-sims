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

# A payload-only Quick Ball play advances no searched Basic axis. Seed 71 has
# Active Regidrago VSTAR at GG after the manual attachment is spent and no preferred
# Basic target, so the held payload and Quick Ball must wait while the later ready
# turn uses the strongest observable legal payload outlet available then:
# Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
# Regidrago VSTAR and GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
# Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
# Official Item, attachment, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# Strict-JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
# Energy-axis projection: https://github.com/FlareZ123/pokemon-sims/issues/737
# Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1092
run_trace("strict-jit/go-first" 71 issue_1092_seed_71)
if(issue_1092_seed_71 MATCHES "T2 \\| DISCARD \\|.*Quick Ball cost")
  message(FATAL_ERROR "Seed 71 still spent a payload-only Quick Ball cost before GGF could finish:\n${issue_1092_seed_71}")
endif()
if(issue_1092_seed_71 MATCHES "T2 \\| PLAY ITEM \\|.*Quick Ball")
  message(FATAL_ERROR "Seed 71 still played a payload-only Quick Ball before the strict-JIT Energy window:\n${issue_1092_seed_71}")
endif()
if(issue_1092_seed_71 MATCHES "LEGACY STAR")
  message(FATAL_ERROR "Seed 71 still spent the game-wide VSTAR Power after preserving its held resources:\n${issue_1092_seed_71}")
endif()
if(NOT issue_1092_seed_71 MATCHES "T3 \\| PLAY ITEM \\|.*Brilliant Blender")
  message(FATAL_ERROR "Seed 71 did not use the strongest observable T3 payload outlet:\n${issue_1092_seed_71}")
endif()
if(NOT issue_1092_seed_71 MATCHES "T3 \\| READY \\|")
  message(FATAL_ERROR "Seed 71 lost its T3 readiness after the target-aware gate:\n${issue_1092_seed_71}")
endif()

# Positive control: once the same-turn manual attachment has completed GGF, Quick
# Ball may discard a Dragon payload and satisfy strict JIT on that exact ready turn:
# Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
# Refined bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1092
run_trace("strict-jit/go-first" 44 issue_1092_seed_44_control)
if(NOT issue_1092_seed_44_control MATCHES "T3 \\| DISCARD \\|.*Quick Ball cost")
  message(FATAL_ERROR "Seed 44 lost the legal same-turn Quick Ball payload outlet:\n${issue_1092_seed_44_control}")
endif()
if(NOT issue_1092_seed_44_control MATCHES "T3 \\| READY \\|")
  message(FATAL_ERROR "Seed 44 lost same-turn strict-JIT readiness:\n${issue_1092_seed_44_control}")
endif()

# Connector control: Quick Ball may still pay a redundant Dragon when its searched
# Tapu Lele-GX advances setup through Wonder Tag into Crispin. That live Basic route
# reaches GG on T2 and preserves the established T3 Blender finish:
# Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
# Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
# Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
# Refined bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1092
run_trace("strict-jit/go-first" 104 issue_1092_seed_104_connector)
if(NOT issue_1092_seed_104_connector MATCHES "T2 \\| PLAY ITEM \\|.*Tapu Lele-GX")
  message(FATAL_ERROR "Seed 104 lost the live Quick Ball into Tapu Lele-GX connector:\n${issue_1092_seed_104_connector}")
endif()
if(NOT issue_1092_seed_104_connector MATCHES "T2 \\| WONDER TAG \\|.*Crispin")
  message(FATAL_ERROR "Seed 104 lost Wonder Tag into Crispin:\n${issue_1092_seed_104_connector}")
endif()
if(NOT issue_1092_seed_104_connector MATCHES "T3 \\| READY \\|")
  message(FATAL_ERROR "Seed 104 lost its established T3 ready route:\n${issue_1092_seed_104_connector}")
endif()
