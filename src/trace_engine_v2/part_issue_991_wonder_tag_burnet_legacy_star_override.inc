  bool wonder_tag_burnet_is_redundant_with_legacy_star_route() const {
    if (!deck_seen_ || !supporter_allowed() || state_.manual_energy_used ||
        state_.vstar_power_used || !need_energy() || has_vstar() ||
        !need_payload() || need_regi() || hand_count(Card::TapuLeleGX) == 0 ||
        hand_count(Card::Crispin) == 0 ||
        hand_count(Card::ProfessorBurnet) > 0 ||
        hand_count(Card::RegidragoVstar) == 0 || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        !ability_available_for_pokemon(Card::RegidragoVstar) ||
        !payload_might_be_in_deck()) {
      return false;
    }

    const bool grass_available = deck_count_after_search_started(Card::Grass) > 0;
    const bool fire_available = deck_count_after_search_started(Card::Fire) > 0;
    int grass_remaining = grass_needed();
    int fire_remaining = fire_needed();
    int grass_in_hand = hand_count(Card::Grass);
    int fire_in_hand = hand_count(Card::Fire);

    if (grass_available && fire_available) {
      // The simulator's legal Crispin policy attaches the currently needed type,
      // preferring Grass while any Grass requirement remains:
      // https://api.pokemontcg.io/v2/cards/sv7-133
      const Card attached = grass_remaining > 0 ? Card::Grass : Card::Fire;
      if (attached == Card::Grass) {
        grass_remaining = std::max(0, grass_remaining - 1);
        ++fire_in_hand;
      } else {
        fire_remaining = std::max(0, fire_remaining - 1);
        ++grass_in_hand;
      }
    } else if (grass_available) {
      // With one searchable type, the modeled ruling puts it into hand and attaches
      // none, leaving the unused manual attachment to advance GGF:
      // https://api.pokemontcg.io/v2/cards/sv7-133
      ++grass_in_hand;
    } else if (fire_available) {
      ++fire_in_hand;
    } else {
      return false;
    }

    const bool crispin_and_manual_complete_ggf =
        (grass_remaining == 0 && fire_remaining == 0) ||
        (grass_remaining == 1 && fire_remaining == 0 && grass_in_hand > 0) ||
        (grass_remaining == 0 && fire_remaining == 1 && fire_in_hand > 0);

    // Held Crispin plus the unused manual attachment can complete GGF. The
    // prior-turn Active Regidrago V may then evolve and use Legacy Star for the
    // strict-JIT payload axis. Benching the searched second Tapu Lele-GX and using
    // Wonder Tag for Professor Burnet spends a card, Bench slot, and one-shot
    // Ability while that Supporter remains unused:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR and Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/991
    return crispin_and_manual_complete_ggf;
  }

  bool wonder_tag_gladion_is_redundant_with_held_manual_energy() const {
    if (!deck_seen_ || !supporter_allowed() || state_.manual_energy_used ||
        hand_count(Card::TapuLeleGX) == 0 || hand_count(Card::Gladion) > 0 ||
        !might_be_unseen(Card::Gladion)) {
      return false;
    }

    const std::optional<Card> prized_energy = final_prized_energy_for_gladion();

    // Gladion exchanges itself for a Prize card. When the exact Prize-target type
    // is already held and the unused manual attachment completes GGF, another Tapu
    // Lele-GX and Wonder Tag consume a card, Bench slot, and one-shot Ability without
    // advancing any setup axis:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Turn procedure and manual attachment: https://www.pokemon.com/us/pokemon-tcg/rules
    // Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/992
    return prized_energy && hand_count(*prized_energy) > 0;
  }

  bool bench_tapu_if_useful() {
    if (wonder_tag_burnet_is_redundant_with_legacy_star_route()) {
      // Keep the second Tapu Lele-GX in hand after Quick Ball has legally searched
      // and shuffled. Every setup axis already has a shorter observable route:
      // https://api.pokemontcg.io/v2/cards/cel25c-60_A
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://github.com/FlareZ123/pokemon-sims/issues/991
      trace("HOLD TAPU LELE-GX", "R-TAPU-01; P-DCI-01; P-KNOWLEDGE-01",
            "Retained the second Tapu because held Crispin, evolution, manual attachment, and Legacy Star already complete setup.");
      return false;
    }
    if (wonder_tag_gladion_is_redundant_with_held_manual_energy()) {
      // Preserve Tapu Lele-GX and Wonder Tag because the held Prize-target Energy
      // already completes the final axis through the legal manual attachment:
      // https://api.pokemontcg.io/v2/cards/cel25c-60_A
      // https://api.pokemontcg.io/v2/cards/sm4-95
      // https://github.com/FlareZ123/pokemon-sims/issues/992
      trace("HOLD TAPU LELE-GX", "R-TAPU-01; P-KNOWLEDGE-01; P-DCI-01",
            "Retained Tapu because the held needed Energy and manual attachment already complete setup.");
      return false;
    }
    return bench_tapu_if_useful_issue_991_original();
  }
