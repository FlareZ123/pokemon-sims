#include "core/guzma_hala_policy.inc"
#include "part_issue_3040_turo_regidrago_staging_override.inc"

  void choose_supporter_issue3040_fallback() {
    if (supporter_allowed() && issue_3040_play_turo_regidrago_staging_route()) return;
    // Guzma & Hala enters the established selector as a normal Engine policy call.
    // No new preprocessor replacement is introduced for the card. The card resolver
    // owns printed behavior while this policy admits only multi-axis compression or
    // a Rule Box Ability-lock Stadium unlock.
    // https://www.pokemon.com/uk/pokemon-tcg/pokemon-cards/sm-series/sm12/229/
    // https://compendium.pokegym.net/category/5-trainers/guzma-and-hala/
    // https://github.com/FlareZ123/pokemon-sims/issues/3386
    if (supporter_allowed() && play_guzma_hala()) return;
    choose_supporter_issue1070_original();
  }

// Preserve the established issue-1070 selector unchanged, while routing only its
// final fallback through the confirmed issue-3040 Turo staging decision.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
// Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
#define choose_supporter_issue1070_original choose_supporter_issue3040_fallback
#include "part_issue_1070_tate_after_vstar_search_original.inc"
#undef choose_supporter_issue1070_original
