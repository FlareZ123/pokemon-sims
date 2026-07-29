  bool held_blender_only_blocks_combined_legacy_star_route() const {
    if (hand_count(Card::BrilliantBlender) == 0 || !active_is_vstar() ||
        state_.manual_energy_used || supporter_allowed() || !need_energy() ||
        !need_payload() || !can_play_payload_this_turn() ||
        !payload_might_be_in_deck() || state_.deck.empty()) {
      return false;
    }

    const int missing_grass = std::max(0, 2 - state_.active->grass);
    const int missing_fire = std::max(0, 1 - state_.active->fire);
    if (missing_grass + missing_fire != 1) return false;

    const Card needed_energy = missing_grass == 1 ? Card::Grass : Card::Fire;
    return hand_count(needed_energy) == 0 && might_be_unseen(needed_energy);
  }

  bool use_legacy_star() {
    const bool issue_1765_hold_for_t3_payload_window =
        scenario_.dci == DciProfile::StrictJit && scenario_.locks == LockMode::None &&
        !scenario_.going_first && state_.turn == 2 && scenario_.max_turn >= 3 &&
        deck_seen_ && state_.manual_energy_used && active_is_vstar() &&
        grass_needed() == 0 && fire_needed() == 1 && need_payload() &&
        hand_count(Card::Fire) > 0 && hand_count(Card::EarthenVessel) > 0 &&
        count_of(state_.discard, Card::MysteriousTreasure) > 0;
    if (issue_1765_hold_for_t3_payload_window) {
      // The surplus-Grass Treasure route completes the VSTAR and Grass axes on T2,
      // while the held Fire completes GGF on T3. Strict JIT cannot credit a T2
      // Legacy Star payload on T3, so preserve the one-use VSTAR Power for the first
      // turn that can complete every setup axis:
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // VSTAR Power and manual-attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Strict-JIT and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed route and implementation: https://github.com/FlareZ123/pokemon-sims/issues/1765 https://github.com/FlareZ123/pokemon-sims/pull/1770
      return false;
    }

    if (!held_blender_only_blocks_combined_legacy_star_route()) {
      return use_legacy_star_issue1069_original();
    }

    const int held_blenders = hand_count(Card::BrilliantBlender);
    std::erase(state_.hand, Card::BrilliantBlender);
    const bool used = use_legacy_star_issue1069_original();
    state_.hand.insert(state_.hand.end(), static_cast<std::size_t>(held_blenders),
                       Card::BrilliantBlender);

    // Brilliant Blender can solve only the Dragon-payload axis. When no matching
    // Basic Energy is held and the Supporter play is already spent, temporarily
    // excluding Blender from route admission lets one Legacy Star resolution discard
    // the current-turn payload and recover the missing manual-attachment Energy. The
    // physical Blender remains in hand throughout and is restored after evaluation:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR and Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Manual Energy attachment and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1069
    return used;
  }
