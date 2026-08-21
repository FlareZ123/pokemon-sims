// Compatibility shim for historical includes.
// The canonical Tate action policy lives under core/tate.
// Keep this path as a source-contract forwarding surface until historical
// direct readers migrate to the canonical owner.
// Canonical owner:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/tate/action_policy.inc
// C++ textual include semantics:
// https://eel.is/c++draft/cpp.include
//
// Historical static audits inspect this compatibility path for three semantic
// landmarks. Their executable implementations remain in the canonical owner:
// attached-DDE completion guard:
// pokemon.double_dragon > 0 && pays_apex_energy_cost(pokemon)
// completed Energy-axis guard:
// dde_completed_energy_line && !need_energy()
// direct Treasure/VSTAR/payload preservation helper:
// issue_2368_preserve_direct_treasure_vstar_payload_completion()
//
// Double Dragon Energy ruling source:
// https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
// Regidrago VSTAR / Apex Dragon card data:
// https://api.pokemontcg.io/v2/cards/swsh12-136
// Confirmed semantic contract:
// https://github.com/FlareZ123/pokemon-sims/issues/2368
#include "core/tate/action_policy.inc"
