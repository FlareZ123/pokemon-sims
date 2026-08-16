  // Canonical Hisuian Heavy Ball policy owner:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/hisuian_heavy_ball_policy.inc
  #include "core/routes/hisuian_heavy_ball_policy.inc"

  // Complete search-connector helper members live in one named route owner.
  // This preserves the existing Engine member boundary while reducing numbered-fragment ownership:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
  #include "core/routes/search_connector_helpers.inc"
