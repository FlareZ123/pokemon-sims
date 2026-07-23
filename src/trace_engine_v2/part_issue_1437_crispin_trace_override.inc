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
    if (detail ==
        "Used the Secret Box searched Crispin before lower-impact Prize or "
        "future-route Supporters.") {
      trace(action, "R-CRISPIN-01; P-AXIS-01",
            "Used held Crispin before lower-impact Prize or future-route "
            "Supporters.");
      return;
    }
    trace(action, rules, detail);
  }
