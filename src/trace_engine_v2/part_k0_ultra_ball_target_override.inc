  bool ultra_ball_has_legal_target() const {
    // Ultra Ball searches for any Pokémon. At K0, a target remains plausible only
    // when the fixed list has a Pokémon copy not accounted for in public zones. At
    // K1, might_be_unseen() checks the inspected deck exactly:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // https://github.com/FlareZ123/pokemon-sims/issues/690
    return std::any_of(recipe_.begin(), recipe_.end(), [this](const auto& entry) {
      return is_pokemon(entry.first) && might_be_unseen(entry.first);
    });
  }

  bool play_ultra_ball(const bool permit_payload) {
    // Reject the publicly proved zero-target K0 state before the wrapped policy can
    // pay Ultra Ball's two-card discard requirement and establish deck knowledge:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/issues/690
    return ultra_ball_has_legal_target() &&
           play_ultra_ball_k0_target_original(permit_payload);
  }
