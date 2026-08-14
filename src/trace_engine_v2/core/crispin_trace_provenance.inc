  void trace_issue_1437_crispin_provenance(
      const std::string_view action, const std::string_view rules,
      const std::string_view detail) {
    // The generic Pineco Energy branch can play a Crispin that was already held,
    // searched by another connector, or selected by Secret Box. Preserve the
    // real Secret Box search line and use neutral wording when provenance is not
    // represented by simulator state:
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Trace contract: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1437
    static constexpr std::string_view kSecretBoxSearchDetail =
        "Used the Secret Box searched Crispin before lower-impact Prize or "
        "future-route Supporters.";
    static constexpr std::string_view kHeldCrispinDetail =
        "Used held Crispin before lower-impact Prize or future-route Supporters.";
    if (detail == kSecretBoxSearchDetail) {
      trace(action, "R-CRISPIN-01; P-AXIS-01", kHeldCrispinDetail);
      return;
    }
    trace(action, rules, detail);
  }

  // Keep the complete #3152 Steven/Secret Box comparator under the canonical
  // route directory while preserving this exact class-body composition point.
  // C++ textual include semantics: https://eel.is/c++draft/cpp.include
  // Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3152
  #include "routes/issue_3152_steven_secret_box_policy.inc"

  // part_issue_1118_secret_box.inc defines the Pineco turn policy. Rename that
  // implementation so the issue-1439 post-include wrapper can compare its exact
  // complete route before the original Secret Box action sequence:
  // https://github.com/FlareZ123/pokemon-sims/issues/1439
  #define run_secret_box_turn run_secret_box_turn_issue1439_original
