  // Canonical Hisuian Heavy Ball policy owner:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/hisuian_heavy_ball_policy.inc
  #include "core/routes/hisuian_heavy_ball_policy.inc"

  // Complete search-connector helper members live in one named route owner.
  // This preserves the existing Engine member boundary while reducing numbered-fragment ownership:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
  #include "core/routes/search_connector_helpers.inc"

  Card fallback_quick_ball_target_after_search_started() const {
    for (const Card target : {Card::RegidragoV, Card::LatiasEx, Card::Oricorio, Card::TapuLeleGX,
                              Card::CrobatV, Card::MawileGX, Card::DialgaGX,
                              Card::Pineco}) {  // Quick Ball/Pineco: https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://github.com/FlareZ123/pokemon-sims/issues/1958
      if (deck_count_after_search_started(target) > 0) return target;
    }