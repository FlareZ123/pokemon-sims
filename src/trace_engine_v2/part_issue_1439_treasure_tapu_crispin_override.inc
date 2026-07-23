  bool issue_1439_treasure_tapu_crispin_available() const {
    const auto payload = std::find_if(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const bool active_regi_can_evolve = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn;

    // A complete current-turn route outranks a same-deadline Secret Box and
    // Forretress ex route when it preserves the ACE SPEC, Bench capacity, and
    // self-Knock-Out. Mysterious Treasure may discard the held Dragon payload and
    // search Psychic Tapu Lele-GX; Wonder Tag searches Crispin; Crispin supplies
    // the missing Grass, with the manual attachment completing the one-type case:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Core Item, Bench, Ability, Supporter, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Same-deadline resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Pineco route comparison: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#secret-box-and-pineco-route-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1439
    return secret_box_combo_enabled() && deck_seen_ &&
        scenario_.dci == DciProfile::StrictJit && state_.turn > 1 &&
        !item_locked() && !rule_box_abilities_locked() &&
        supporter_allowed() && !state_.manual_energy_used &&
        active_regi_can_evolve && grass_needed() == 1 && fire_needed() == 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::SecretBox) > 0 && payload != state_.hand.end() &&
        hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
        hand_count(Card::Crispin) == 0 && bench_space() > 0 &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 && need_payload() &&
        secret_box_full_combo_axes_visible() && secret_box_cost_plan().has_value();
  }

  bool play_issue_1439_treasure_tapu_crispin_completion() {
    if (!issue_1439_treasure_tapu_crispin_available()) return false;
    const auto payload = std::find_if(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const Card discarded_payload = *payload;

    if (!evolve_best_regi() || !active_is_vstar()) {
      throw std::logic_error("Issue 1439 route lost its legal Regidrago VSTAR evolution");
    }
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(discarded_payload, "Mysterious Treasure cost",
                           "R-MT-01; P-JIT-01")) {
      throw std::logic_error("Issue 1439 Treasure payload cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error("Issue 1439 Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-JIT-01; P-COMPRESS-01",
          "Discarded the current-turn Dragon payload and searched Tapu Lele-GX "
          "for the resource-preserving Crispin route.");

    if (!bench_from_hand(Card::TapuLeleGX, false)) {
      throw std::logic_error("Issue 1439 Tapu Lele-GX Bench action failed");
    }
    record_deck_search_knowledge("Wonder Tag");
    if (!move_deck_to_hand(Card::Crispin)) {
      throw std::logic_error("Issue 1439 Crispin target disappeared");
    }
    shuffle(state_.deck);
    trace("WONDER TAG", "R-TAPU-01; R-CRISPIN-01; P-COMPRESS-01",
          "Searched and revealed Crispin for the missing Grass Energy axis.");

    if (!play_crispin()) {
      throw std::logic_error("Issue 1439 Crispin completion diverged");
    }
    if (need_energy() && !attach_manual()) {
      throw std::logic_error("Issue 1439 manual Grass attachment diverged");
    }
    if (!active_is_vstar() || need_energy() || !payload_ready()) {
      throw std::logic_error("Issue 1439 route failed its proven ready state");
    }
    trace("POLICY", "R-MT-01; R-TAPU-01; R-CRISPIN-01; P-AXIS-01",
          "Completed the lower-resource Mysterious Treasure to Tapu Lele-GX to "
          "Crispin route before spending Secret Box or the Forretress ex line: " +
              state_line());
    return true;
  }

  void run_secret_box_turn() {
    if (!issue_1439_treasure_tapu_crispin_available()) {
      run_secret_box_turn_issue1439_original();
      return;
    }

    // Preserve the original turn-start telemetry and trace contract before the
    // exact route comparison, then stop before Secret Box's three-card cost:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
    // https://github.com/FlareZ123/pokemon-sims/issues/1439
    if (state_.turn == 2) {
      const auto eligible_regi = [this](const Pokemon& pokemon) {
        return pokemon.card == Card::RegidragoV &&
            pokemon.entered_turn < state_.turn;
      };
      outcome_.t2_eligible_regi_start =
          (state_.active && eligible_regi(*state_.active)) ||
          std::any_of(state_.bench.begin(), state_.bench.end(), eligible_regi);
      outcome_.t2_active_regi_start = state_.active &&
          (state_.active->card == Card::RegidragoV ||
           state_.active->card == Card::RegidragoVstar);
      outcome_.t2_pineco_line_start = in_play_count(Card::Pineco) > 0 ||
          in_play_count(Card::ForretressEx) > 0;
      outcome_.t2_vstar_held_start = hand_count(Card::RegidragoVstar) > 0;
    }
    trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01",
          "Start Pineco policy: " + state_line());
    if (!play_issue_1439_treasure_tapu_crispin_completion()) {
      throw std::logic_error("Issue 1439 projected route diverged before resolution");
    }
  }
