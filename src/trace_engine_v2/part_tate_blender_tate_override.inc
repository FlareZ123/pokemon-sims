  bool has_live_one_discard_hand_payload_line() const {
    // Each modeled one-discard outlet must begin a deck search after paying its cost.
    // An empty deck makes that promised post-Tate payload route unavailable:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    if (!deck_search_available() || item_locked() || !can_play_payload_this_turn() ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // These Items discard one other hand card before resolving their searches. When
    // the policy-selected cost is a Dragon payload, the same play establishes the
    // strict current-turn Apex Dragon payload axis after Tate & Liza switches Active:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    for (const Card item : {Card::MysteriousTreasure, Card::QuickBall, Card::EarthenVessel}) {
      if (hand_count(item) == 0) continue;
      // A K1 Mysterious Treasure route is live only when the inspected deck still
      // contains a Psychic or Dragon Pokémon. The payload discard is a play cost and
      // cannot justify a Trainer that is known to have no effect:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
      const bool known_dead_mysterious = item == Card::MysteriousTreasure && deck_seen_ &&
          std::none_of(state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic);
      if (known_dead_mysterious) continue;
      // The same target-aware legality applies to Quick Ball's Basic Pokémon search:
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
      const bool known_dead_quick = item == Card::QuickBall && deck_seen_ &&
          std::none_of(state_.deck.begin(), state_.deck.end(), is_basic);
      if (known_dead_quick) continue;
      // Earthen Vessel is live only while K0 keeps Basic Energy plausible or K1
      // proves at least one modeled Basic Energy remains in deck:
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
      if (item == Card::EarthenVessel && !earthen_vessel_has_legal_target()) continue;
      const auto cost = choose_discard(true, true, true, item);
      if (cost && is_payload(*cost)) return true;
    }
    return false;
  }

  bool preserve_incomplete_tate_blender_line() const {
    const int supporters_in_hand = static_cast<int>(
        std::count_if(state_.hand.begin(), state_.hand.end(), is_supporter));
    const bool only_tate_supporter_held =
        supporters_in_hand == hand_count(Card::TateLiza);

    // Tate & Liza can promote the selected Benched VSTAR and Brilliant Blender can
    // supply its current-turn payload, but Apex Dragon still requires GGF. Inspect
    // the actual promotion target instead of generic Energy routing to another line:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    return supporter_allowed() && strict_payload_timing() && only_tate_supporter_held &&
           hand_count(Card::TateLiza) > 0 && need_active_vstar() &&
           !benched_vstar_promotion_ready() && has_live_blender_payload_line();
  }

  bool preserve_tate_hand_payload_switch_line() const {
    // Tate & Liza switch mode may promote a complete Benched Regidrago VSTAR before
    // a payable one-discard Item puts the held Dragon into discard. Preserve that
    // complete route instead of shuffling all three cards through Tate's draw mode:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    return supporter_allowed() && hand_count(Card::TateLiza) > 0 &&
           need_active_vstar() && benched_vstar_promotion_ready() && need_payload() &&
           !can_free_retreat_with_latias() && has_live_one_discard_hand_payload_line();
  }

  bool play_tate_switch() {
    if (preserve_incomplete_tate_blender_line()) {
      trace("HOLD SUPPORTER", "R-TATE-01; R-BLENDER-01; R-RVS-01; P-DCI-01",
            "Retained Tate & Liza because the promoted Regidrago VSTAR still lacks part of GGF.");
      return false;
    }
    return play_tate_switch_tate_blender_original();
  }

  bool play_tate_draw() {
    if (preserve_incomplete_tate_blender_line()) {
      trace("HOLD SUPPORTER", "R-TATE-01; R-BLENDER-01; R-RVS-01; P-DCI-01",
            "Retained Tate & Liza and Brilliant Blender instead of shuffling the incomplete promotion route.");
      return false;
    }
    if (preserve_tate_hand_payload_switch_line()) {
      trace("HOLD SUPPORTER", "R-TATE-01; R-RVS-01; P-DCI-01",
            "Retained Tate & Liza switch mode and the held one-discard Dragon payload route.");
      return false;
    }
    return play_tate_draw_tate_blender_original();
  }
