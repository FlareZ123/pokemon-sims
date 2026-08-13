#include "part_issue_3040_turo_regidrago_staging_override.inc"

  void choose_supporter_with_turo_staging_fallback() {
    if (supporter_allowed() && issue_3040_play_turo_regidrago_staging_route()) return;
    choose_supporter_issue1070_original();
  }

// Keep the issue-1070 selector as the single downstream owner and route only its
// final fallback through the confirmed Turo staging policy. The semantic wrapper
// name avoids carrying an issue number into later composition layers.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
// Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
// Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3040
#define choose_supporter_issue1070_original choose_supporter_with_turo_staging_fallback
#include "part_issue_1070_tate_after_vstar_search_original.inc"
#undef choose_supporter_issue1070_original
