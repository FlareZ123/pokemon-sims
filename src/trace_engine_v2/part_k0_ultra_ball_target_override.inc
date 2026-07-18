  bool ultra_ball_has_legal_target() const {
    // Ultra Ball searches for any Pokémon. Once K1 has inspected the physical deck,
    // target legality comes directly from state_.deck, including supported reduced
    // fixtures whose modeled Pokémon is absent from the canonical recipe:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc#L77-L92
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // https://github.com/FlareZ123/pokemon-sims/issues/866
    if (deck_seen_) {
      return std::any_of(state_.deck.begin(), state_.deck.end(), is_pokemon);
    }

    // At K0, a target remains plausible only when the fixed list has a Pokémon copy
    // that public zones have not exhausted:
    // https://github.com/FlareZ123/pokemon-sims/issues/690
    return std::any_of(recipe_.begin(), recipe_.end(), [this](const auto& entry) {
      return is_pokemon(entry.first) && might_be_unseen(entry.first);
    });
  }

  bool play_ultra_ball(const bool permit_payload) {
    // Reject the publicly proved zero-target state before the wrapped policy can pay
    // Ultra Ball's two-card discard requirement and establish deck knowledge:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/issues/690
    // https://github.com/FlareZ123/pokemon-sims/issues/866
    return ultra_ball_has_legal_target() &&
           play_ultra_ball_k0_target_original(permit_payload);
  }
