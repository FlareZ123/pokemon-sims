from pathlib import Path


def replace_once(path: str, old: str, new: str) -> None:
    file_path = Path(path)
    source = file_path.read_text(encoding="utf-8")
    if source.count(old) != 1:
        raise SystemExit(f"{path} replacement count: {source.count(old)}")
    file_path.write_text(source.replace(old, new, 1), encoding="utf-8")


replace_once(
    "tests/issue_1447_vessel_timing_tests.cpp",
    '''  // Crispin plus the T2 manual attachment creates GF. Earthen Vessel can then
  // discard a held or newly drawn Dragon on T3, search the final Grass, and
  // preserve the singleton Brilliant Blender at the same earliest ready turn:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1447
  expect(outcome.first_ready_turn == 3,
         "Strict-JIT seed 104 lost its earliest T3 ready turn.");
  expect(!trace_contains(trace,
                         "T2 | DISCARD | rules: R-EV-01 | Dialga-GX"),
         "Earthen Vessel still discarded Dialga-GX before the ready turn.");
  expect(trace_contains(trace,
                        "T3 | DISCARD | rules: R-EV-01 |") &&
             trace_contains(trace, "(Earthen Vessel cost)"),
         "The preserved Vessel did not establish a T3 strict-JIT Dragon payload.");
  expect(!trace_contains(trace, "R-BLENDER-01"),
         "The same-deadline route still consumed Brilliant Blender.");
  expect(trace_contains(trace, "T3 | READY"),
         "The source-bound trace did not report T3 readiness.");
''',
    '''  // Issue #1552 supersedes the older T3 Vessel hold for this exact seed. A
  // public T1 Earthen Vessel search establishes K1 and pays route-replaced
  // Mysterious Treasure. On T2, Quick Ball discards Dialga-GX, searches Tapu
  // Lele-GX, Wonder Tag finds Crispin, and Crispin plus the manual Fire complete
  // GGF. The Dragon enters discard on the same ready turn, so strict JIT is met:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original timing boundary: https://github.com/FlareZ123/pokemon-sims/issues/1447
  // Confirmed faster route: https://github.com/FlareZ123/pokemon-sims/issues/1552
  expect(outcome.first_ready_turn == 2,
         "Strict-JIT seed 104 lost its earliest T2 ready turn.");
  expect(trace_contains(trace,
                        "T1 | DISCARD | rules: R-EV-01; P-DCI-01; P-COMPRESS-01 | Mysterious Treasure") &&
             trace_contains(trace,
                            "T2 | DISCARD | rules: R-QB-01; P-DCI-01; P-JIT-01 | Dialga-GX") &&
             trace_contains(trace, "T2 | WONDER TAG") &&
             trace_contains(trace, "Crispin") &&
             trace_contains(trace, "T2 | READY"),
         "The source-bound trace did not execute the T1 Vessel to T2 Quick Ball route.");
  expect(!trace_contains(trace, "R-BLENDER-01"),
         "The faster route consumed Brilliant Blender.");
''',
)

replace_once(
    "tests/issue_1516_quick_ball_tapu_crispin_tests.cpp",
    '''  // Strict JIT needs the distinct Tapu Lele-GX to Crispin connector, so the
  // no-control payload-Prize guard must remain inactive:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Existing route: https://github.com/FlareZ123/pokemon-sims/issues/962
  expect(result.outcome.first_ready_turn == 3 && !result.outcome.setup_failed,
         "Strict-JIT seed 104 lost its T3 route.");
  expect(trace_contains(result.trace,
                        "Searched a Basic Pokémon: Tapu Lele-GX") &&
             trace_contains(result.trace, "WONDER TAG") &&
             trace_contains(result.trace, "Crispin"),
         "Strict-JIT seed 104 lost its distinct Tapu-Crispin connector.");
''',
    '''  // Issue #1552 preserves the distinct Tapu Lele-GX to Crispin connector and
  // advances it one turn. T1 Vessel establishes K1, then T2 Quick Ball places the
  // strict-JIT Dragon payload while Tapu and Crispin complete the Energy axis:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Existing connector boundary: https://github.com/FlareZ123/pokemon-sims/issues/962
  // Confirmed faster route: https://github.com/FlareZ123/pokemon-sims/issues/1552
  expect(result.outcome.first_ready_turn == 2 && !result.outcome.setup_failed,
         "Strict-JIT seed 104 lost its T2 route.");
  expect(trace_contains(result.trace,
                        "Searched a Basic Pokémon: Tapu Lele-GX") &&
             trace_contains(result.trace, "WONDER TAG") &&
             trace_contains(result.trace, "Crispin") &&
             trace_contains(result.trace, "T2 | READY"),
         "Strict-JIT seed 104 lost its faster Tapu-Crispin connector.");
''',
)

replace_once(
    "tests/run_issue_1092_quick_ball_energy_jit_gate.cmake",
    '''# Connector control: Quick Ball may still pay a redundant Dragon when its searched
# Tapu Lele-GX advances setup through Wonder Tag into Crispin. That live Basic route
# reaches GG on T2 and preserves the established T3 Blender finish:
# Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
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
''',
    '''# Connector control: issue #1552 advances the same live Basic route to T2.
# T1 Earthen Vessel establishes K1; T2 Quick Ball discards the Dragon payload,
# searches Tapu Lele-GX, and Wonder Tag finds Crispin before the manual Fire:
# Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
# Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
# Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
# Original gate: https://github.com/FlareZ123/pokemon-sims/issues/1092
# Confirmed faster route: https://github.com/FlareZ123/pokemon-sims/issues/1552
run_trace("strict-jit/go-first" 104 issue_1092_seed_104_connector)
if(NOT issue_1092_seed_104_connector MATCHES "T2 \\| PLAY ITEM \\|.*Tapu Lele-GX")
  message(FATAL_ERROR "Seed 104 lost the live Quick Ball into Tapu Lele-GX connector:\n${issue_1092_seed_104_connector}")
endif()
if(NOT issue_1092_seed_104_connector MATCHES "T2 \\| WONDER TAG \\|.*Crispin")
  message(FATAL_ERROR "Seed 104 lost Wonder Tag into Crispin:\n${issue_1092_seed_104_connector}")
endif()
if(NOT issue_1092_seed_104_connector MATCHES "T2 \\| READY \\|")
  message(FATAL_ERROR "Seed 104 lost its established T2 ready route:\n${issue_1092_seed_104_connector}")
endif()
''',
)
