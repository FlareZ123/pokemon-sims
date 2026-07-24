  std::optional<Card> choose_discard(
      const bool permit_payload, const bool flex_fodder,
      const bool allow_heavy_ball = true,
      const std::optional<Card> excluded_from_cost = std::nullopt,
      const bool allow_redundant_payload = false) const {
    const auto ordinary = choose_discard_issue1436_original(
        permit_payload, flex_fodder, allow_heavy_ball, excluded_from_cost,
        allow_redundant_payload);
    if (ordinary) return ordinary;

    const auto publicly_possible_count = [this](const Card card) {
      if (deck_seen_ || prizes_revealed_) {
        return deck_count_after_search_started(card);
      }
      const int known = hand_count(card) + count_of(state_.discard, card) +
          in_play_count(card) + attached_public_count(card);
      return std::max(0, total_copies(card) - known);
    };
    const int missing_grass = grass_needed();
    const int missing_fire = fire_needed();
    const int missing_energy = missing_grass + missing_fire;
    const int future_turns = std::max(0, scenario_.max_turn - state_.turn);
    const int attachment_windows = future_turns +
        (state_.manual_energy_used ? 0 : 1);
    int possible_payloads = 0;
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      possible_payloads += publicly_possible_count(card);
    }

    const bool quick_ball_oricorio_latias_route =
        excluded_from_cost && *excluded_from_cost == Card::QuickBall &&
        scenario_.dci == DciProfile::StrictJit &&
        scenario_.locks == LockMode::None && active_is_vstar() &&
        hand_count(Card::QuickBall) > 0 && hand_count(Card::LatiasEx) > 0 &&
        hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) &&
        bench_space() > 0 && ability_available_for_pokemon(Card::Oricorio) &&
        might_be_unseen(Card::Oricorio) && missing_energy > 0 &&
        missing_energy <= 2 && attachment_windows >= missing_energy &&
        publicly_possible_count(Card::Grass) >= missing_grass &&
        publicly_possible_count(Card::Fire) >= missing_fire &&
        hand_count(Card::BrilliantBlender) > 0 && !payload_ready() &&
        possible_payloads > future_turns;
    if (quick_ball_oricorio_latias_route) {
      // Once Regidrago VSTAR is already Active, Latias ex no longer advances the
      // modeled Active-position axis. Quick Ball may spend that dynamically inert
      // singleton only when Oricorio can supply every missing Basic Energy, enough
      // future manual attachments remain, and Brilliant Blender is guaranteed a
      // surviving same-turn Dragon payload after all intervening draws:
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
      // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
      // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Core Item, Bench, Ability, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
      // Earliest deterministic route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed stale-claim bug: https://github.com/FlareZ123/pokemon-sims/issues/1419
      return Card::LatiasEx;
    }

    if (scenario_.locks != LockMode::FullSupporter) return std::nullopt;

    // Full Supporter lock disables every Supporter effect for the complete
    // modeled horizon. A held Supporter therefore has zero Supporter AMR and may
    // pay an Item's printed discard cost after every ordinary lower-DCI option
    // has been exhausted. The card being played remains excluded from its own
    // cost, and every non-Supporter singleton keeps its existing protection:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1436
    for (const Card card : state_.hand) {
      if (!is_supporter(card)) continue;
      const int excluded = excluded_from_cost && *excluded_from_cost == card ? 1 : 0;
      if (hand_count(card) > excluded) return card;
    }
    return std::nullopt;
  }
