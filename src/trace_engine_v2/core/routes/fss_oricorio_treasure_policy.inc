  bool fss_latias_over_redundant_burnet_route_available() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2087

    const bool latias_ability_available =
        scenario_.locks != LockMode::FullRuleBoxAbility &&
        scenario_.locks != LockMode::FullCombined;
    if (!prizes_known() || scenario_.dci != DciProfile::StrictJit ||
        state_.turn != 2 || state_.turn >= scenario_.max_turn ||
        !supporter_allowed() ||
        state_.manual_energy_used || state_.retreat_used || !state_.active ||
        state_.active->card != Card::Oricorio || bench_space() <= 0 ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        !latias_ability_available || hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        hand_count(Card::Crispin) == 0 || !crispin_can_advance_energy_axis() ||
        !in_play(Card::TapuLeleGX) ||
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        !payload_might_be_in_deck()) {
      return false;
    }

    const Pokemon* evolving_regi = nullptr;
    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.card == Card::RegidragoV &&
          pokemon.entered_turn < state_.turn) {
        evolving_regi = &pokemon;
        break;
      }
    }
    if (evolving_regi == nullptr || pays_apex_energy_cost(*evolving_regi)) {
      return false;
    }

    const int deck_grass = deck_count_after_search_started(Card::Grass);
    const int deck_fire = deck_count_after_search_started(Card::Fire);
    if (deck_grass == 0 || deck_fire == 0) return false;

    bool current_turn_energy_completes = false;
    bool next_turn_energy_continues = false;
    for (const Card crispin_attachment : {Card::Grass, Card::Fire}) {
      Pokemon after_crispin = *evolving_regi;
      int held_grass = hand_count(Card::Grass);
      int held_fire = hand_count(Card::Fire);
      const int remaining_grass = deck_grass - 1;
      const int remaining_fire = deck_fire - 1;
      if (!attach_energy_card(after_crispin, crispin_attachment)) continue;

      // Crispin attaches one searched Basic and places the other searched type in
      // hand. DDE makes those typed cards alternative completion choices rather
      // than additive G/F components: https://github.com/FlareZ123/pokemon-sims/issues/2427
      if (crispin_attachment == Card::Grass) ++held_fire;
      else ++held_grass;

      for (const Card manual_attachment : {Card::Grass, Card::Fire}) {
        if ((manual_attachment == Card::Grass && held_grass == 0) ||
            (manual_attachment == Card::Fire && held_fire == 0)) {
          continue;
        }
        Pokemon after_manual = after_crispin;
        if (!attach_energy_card(after_manual, manual_attachment)) continue;
        if (pays_apex_energy_cost(after_manual)) {
          current_turn_energy_completes = true;
          continue;
        }
        next_turn_energy_continues = next_turn_energy_continues ||
            completing_basic_energy_for(after_manual, [&](const Card card) {
              return card == Card::Grass ? remaining_grass > 0 :
                     card == Card::Fire ? remaining_fire > 0 : false;
            }).has_value();
      }
    }
    if (current_turn_energy_completes || !next_turn_energy_continues) return false;

    // The prior-turn Benched Regidrago V can evolve after Crispin and the manual
    // attachment. Latias ex then resolves the different Active-position axis,
    // while held Professor Burnet preserves the deck-to-discard payload axis for
    // the next ready turn. Brilliant Blender would duplicate Burnet and leave the
    // Basic Active stranded:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Evolution, Supporter, attachment, Ability, Bench, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Hidden-order boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1137
    return true;
  }

  bool fss_latias_post_crispin_burnet_route_available() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2087

    const bool latias_ability_available =
        scenario_.locks != LockMode::FullRuleBoxAbility &&
        scenario_.locks != LockMode::FullCombined;
    if (!prizes_known() || scenario_.dci != DciProfile::StrictJit ||
        state_.turn != 3 || !supporter_allowed() || state_.retreat_used ||
        !state_.active || state_.active->card != Card::Oricorio ||
        bench_space() <= 0 || in_play(Card::LatiasEx) ||
        hand_count(Card::LatiasEx) > 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        !latias_ability_available || hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        !payload_might_be_in_deck()) {
      return false;
    }

    bool eligible_regi = false;
    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.card != Card::RegidragoV ||
          pokemon.entered_turn >= state_.turn) {
        continue;
      }

      const bool energy_complete_now = pays_apex_energy_cost(pokemon);
      const bool unused_manual_completes = !state_.manual_energy_used &&
          completing_basic_energy_for(pokemon, [this](const Card card) {
            return hand_count(card) > 0;
          }).has_value();
      if (energy_complete_now || unused_manual_completes) {
        eligible_regi = true;
        break;
      }
    }
    if (!eligible_regi) return false;

    // Professor Burnet already owns the current-turn deck-to-discard payload axis.
    // Star Alchemy should search Latias ex for the unresolved Active-position axis;
    // the held VSTAR evolves the prior-turn Regidrago V, the current or projected
    // manual attachment establishes GGF, Skyliner gives Oricorio free Retreat, and
    // Burnet supplies the strict-JIT Dragon payload in the same turn:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Latias ex and Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Tool, search, attachment, evolution, Bench, Ability, Supporter, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Complementary-axis priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Original turn-two projection: https://github.com/FlareZ123/pokemon-sims/issues/1137
    // Preceding Steven correction: https://github.com/FlareZ123/pokemon-sims/issues/1378
    // Confirmed post-Crispin bug: https://github.com/FlareZ123/pokemon-sims/issues/1379
    return true;
  }

  bool fss_latias_vessel_promotion_route_available() const {
    const bool latias_ability_available =
        scenario_.locks != LockMode::FullRuleBoxAbility &&
        scenario_.locks != LockMode::FullCombined;
    if (!deck_seen_ || scenario_.dci != DciProfile::StrictJit || item_locked() ||
        state_.manual_energy_used || state_.retreat_used || !state_.active ||
        !is_basic(state_.active->card) || bench_space() <= 0 ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        !latias_ability_available || hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::EarthenVessel) == 0 ||
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    for (const Pokemon& pokemon : state_.bench) {
      if (pokemon.card != Card::RegidragoV ||
          pokemon.entered_turn >= state_.turn) {
        continue;
      }

      const auto needed_energy = completing_basic_energy_for(
          pokemon, [this](const Card card) {
            return deck_count_after_search_started(card) > 0;
          });
      if (!needed_energy.has_value()) continue;

      // Star Alchemy must search Latias ex for the unresolved Active-position axis.
      // The held prior-turn V evolves, Earthen Vessel discards the held Dragon and
      // searches the final Basic Energy, the manual attachment completes GGF, and
      // Skyliner gives the Basic Active free Retreat during the same turn:
      // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
      // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Trainer, evolution, attachment, Ability, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1068
      return true;
    }
    return false;
  }

  bool fss_oricorio_with_held_treasure_vstar_route_available() const {
    if (!deck_seen_ || item_locked() || state_.manual_energy_used ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.turn >= scenario_.max_turn || bench_space() <= 0 ||
        in_play(Card::Oricorio) || hand_count(Card::Oricorio) > 0 ||
        deck_count_after_search_started(Card::Oricorio) == 0 ||
        !ability_available_for_pokemon(Card::Oricorio) ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        !payload_might_be_in_deck()) {
      return false;
    }

    const int redundant_payloads = static_cast<int>(std::count_if(
        state_.hand.begin(), state_.hand.end(), is_payload));
    if (redundant_payloads < 2) return false;

    const int held_grass = hand_count(Card::Grass);
    const int held_fire = hand_count(Card::Fire);
    const int deck_grass = deck_count_after_search_started(Card::Grass);
    const int deck_fire = deck_count_after_search_started(Card::Fire);
    int deck_attachments_needed = 4;
    for (int used_grass = 0; used_grass <= std::min(3, held_grass); ++used_grass) {
      for (int used_fire = 0; used_fire <= std::min(3, held_fire); ++used_fire) {
        Pokemon projected = *state_.active;
        for (int i = 0; i < used_grass; ++i) attach_energy_card(projected, Card::Grass);
        for (int i = 0; i < used_fire; ++i) attach_energy_card(projected, Card::Fire);
        deck_attachments_needed = std::min(
            deck_attachments_needed,
            minimum_basic_attachments_to_apex(projected, deck_grass, deck_fire));
      }
    }
    if (deck_attachments_needed == 0 || deck_attachments_needed > 2) return false;

    // Delegate the VSTAR axis to the held Mysterious Treasure, whose printed cost can
    // use one of two redundant held Dragon payloads. Star Alchemy is then free to find
    // Oricorio, Vital Dance searches only the Basic Energy not already held, and the
    // manual attachments across the available turns establish GGF. Held Professor
    // Burnet remains available for a deck-resident strict-JIT payload:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Oricorio and Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Trainer, Bench, Ability, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest connector decomposition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Redundant Dragon cost precedent: https://github.com/FlareZ123/pokemon-sims/issues/925
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1071
    return true;
  }

  bool fss_crispin_scheduled_turn_two_route_available() const {
    if (!deck_seen_ || scenario_.dci == DciProfile::NoDiscardControl ||
        scenario_.locks != LockMode::TurnTwoItem || state_.turn != 1 ||
        scenario_.going_first || scenario_.max_turn < 2 ||
        !supporter_allowed() || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0 ||
        !payload_might_be_in_deck()) {
      return false;
    }

    for (const Card crispin_attachment : {Card::Grass, Card::Fire}) {
      Pokemon projected = *state_.active;
      int held_grass = hand_count(Card::Grass);
      int held_fire = hand_count(Card::Fire);
      if (!attach_energy_card(projected, crispin_attachment)) continue;

      if (crispin_attachment == Card::Grass) ++held_fire;
      else ++held_grass;

      const int remaining_attachments = minimum_basic_attachments_to_apex(
          projected, held_grass, held_fire);
      if (remaining_attachments <= 2) {
        // Crispin searches different Basic Energy types, attaches one, and puts the
        // other into hand. The current and next legal manual attachments then finish
        // GGF before the scheduled T2 Item lock, while the held Burnet remains free to
        // establish the strict-JIT payload on the ready turn. This deterministic route
        // outranks Oricorio plus Celestial Roar's unknown top-three result:
        // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
        // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
        // Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
        // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
        // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Supporter, attachment, attack, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Future-card oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1085
        return true;
      }
    }
    return false;
  }

  Card fss_target_after_search_started() const {
    if (fss_latias_over_redundant_burnet_route_available() ||
        fss_latias_post_crispin_burnet_route_available()) {
      return Card::LatiasEx;
    }

    if (fss_latias_vessel_promotion_route_available()) {
      // The constrained Latias route solves Active position, Energy, Evolution, and
      // ready-turn payload together, so it outranks any delegated single-axis target:
      // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
      // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1068
      return Card::LatiasEx;
    }

    if (fss_crispin_scheduled_turn_two_route_available()) return Card::Crispin;

    const Card original = fss_target_after_search_started_issue1071_original();
    if (original == Card::RegidragoVstar &&
        fss_oricorio_with_held_treasure_vstar_route_available()) {
      return Card::Oricorio;
    }
    return original;
  }
