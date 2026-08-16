# Forretress Composition Package

The canonical namespace-scope composition owner is `src/trace_engine_v2/core/forretress/package.inc`.

Keep its textual order stable:

1. `runtime.inc`
2. public scenario aliases and forward declarations
3. `scenario_extension.inc`
4. `garbodor_scenario_extension.inc`

The historical `src/trace_engine_v2/part_forretress_ex_combo.inc` path is a compatibility include. Keep it as a forwarding seam until repository-wide and source-contract consumers are migrated.

C++ textual include semantics: https://eel.is/c++draft/cpp.include
Cleanup ownership plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
Scenario model: https://github.com/FlareZ123/pokemon-sims/issues/2808
Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
Boost Shake: https://api.pokemontcg.io/v2/cards/swsh7-142
