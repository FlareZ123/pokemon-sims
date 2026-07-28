  int issue_1724_stadium_attempt_turn_{-1};
  int issue_1724_stadium_attempt_count_{0};

  bool play_chaotic_swell_issue_1724() {
    if (play_chaotic_swell()) return true;

    if (issue_1724_stadium_attempt_turn_ != state_.turn) {
      issue_1724_stadium_attempt_turn_ = state_.turn;
      issue_1724_stadium_attempt_count_ = 0;
    }
    ++issue_1724_stadium_attempt_count_;

    // Reserve this compression retry for the third Stadium decision point in the
    // active turn policy, after two Item fixed points and the Supporter selector have
    // already had priority. The action is based only on the visible hand, board, K1
    // card counts, and current legality. It never reads the next card's identity:
    // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
    // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
    // Official Stadium, Bench, Ability, and action-order procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Deterministic-route priority and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Future-card-oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1724
    if (issue_1724_stadium_attempt_count_ < 3 ||
        scenario_.locks != LockMode::None || state_.turn_ended ||
        !setup_axis_missing() || state_.stadium_used ||
        state_.stadium != Stadium::None ||
        hand_count(Card::ChaoticSwell) == 0 ||
        hand_count(Card::CrobatV) == 0 || state_.hand.size() != 7U ||
        state_.dark_asset_used || bench_space() <= 0 || state_.deck.empty() ||
        !ability_available_for_pokemon(Card::CrobatV) ||
        forretress_combo_enabled() ||
        (hand_count(Card::Oricorio) > 0 && needs_oricorio_connector()) ||
        (hand_count(Card::TapuLeleGX) > 0 && needs_tapu_connector())) {
      return false;
    }

    remove_one(state_.hand, Card::ChaoticSwell);
    state_.stadium_used = true;
    state_.stadium = Stadium::ChaoticSwell;
    state_.path_lock_removed = true;
    trace("PLAY STADIUM", "R-CHAOTIC-SWELL-01; R-CROBAT-01; P-COMPRESS-01",
          "Played Chaotic Swell into the empty Stadium zone to make the legal Crobat V Dark Asset draw live.");
    return true;
  }
