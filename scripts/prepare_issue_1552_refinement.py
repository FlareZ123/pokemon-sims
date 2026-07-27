from pathlib import Path

refinement_path = Path("scripts/refine_issue_1552_current_main_tests.py")
refinement = refinement_path.read_text(encoding="utf-8")
third_replacement = '\nreplace_once(\n    "tests/run_issue_1092_quick_ball_energy_jit_gate.cmake",\n'
if refinement.count(third_replacement) != 1:
    raise SystemExit(
        f"issue-1552 refinement split count: {refinement.count(third_replacement)}"
    )
refinement_path.write_text(
    refinement.split(third_replacement, 1)[0].rstrip() + "\n",
    encoding="utf-8",
)

cmake_path = Path("tests/run_issue_1092_quick_ball_energy_jit_gate.cmake")
cmake = cmake_path.read_text(encoding="utf-8")
start_marker = "# Connector control: Quick Ball may still pay a redundant Dragon"
run_marker = 'run_trace("strict-jit/go-first" 104 issue_1092_seed_104_connector)'
start = cmake.find(start_marker)
run = cmake.find(run_marker, start)
if start < 0 or run < 0:
    raise SystemExit("issue-1552 CMake seed-104 comment anchor missing")
new_comment = '''# Connector control: issue #1552 advances the same live Basic route to T2.
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
'''
cmake = cmake[:start] + new_comment + cmake[run:]
old_ready = '''if(NOT issue_1092_seed_104_connector MATCHES "T3 \\| READY \\|")
  message(FATAL_ERROR "Seed 104 lost its established T3 ready route:\\n${issue_1092_seed_104_connector}")
endif()
'''
new_ready = '''if(NOT issue_1092_seed_104_connector MATCHES "T2 \\| READY \\|")
  message(FATAL_ERROR "Seed 104 lost its established T2 ready route:\\n${issue_1092_seed_104_connector}")
endif()
'''
if cmake.count(old_ready) != 1:
    raise SystemExit(f"issue-1552 CMake ready anchor count: {cmake.count(old_ready)}")
cmake_path.write_text(cmake.replace(old_ready, new_ready, 1), encoding="utf-8")
