  bool wonder_tag_is_redundant_with_held_crispin_legacy_star_route() const {
    if (deck_seen_ || !supporter_allowed() || state_.manual_energy_used ||
        state_.vstar_power_used || !need_energy() || has_vstar() ||
        !need_payload() || need_regi() || hand_count(Card::TapuLeleGX) == 0 ||
        hand_count(Card::Crispin) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        !ability_available_for_pokemon(Card::RegidragoVstar) ||
        !payload_might_be_in_deck()) {
      return false;
    }

    const bool grass_available = might_be_unseen(Card::Grass);
    const bool fire_available = might_be_unseen(Card::Fire);
    int grass_remaining = grass_needed();
    int fire_remaining = fire_needed();
    int grass_in_hand = hand_count(Card::Grass);
    int fire_in_hand = hand_count(Card::Fire);

    if (grass_available && fire_available) {
      // The modeled Crispin route attaches a currently needed type, preferring
      // Grass while Grass is still required, and puts the other type into hand:
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

    // At K0, held Crispin plus the unused manual attachment can complete GGF.
    // The prior-turn Active Regidrago V can then evolve into the held VSTAR and
    // use Legacy Star for the strict-JIT payload axis. Wonder Tag is optional,
    // so searching a duplicate Crispin would spend Tapu Lele-GX, its Bench slot,
    // and its one-shot Ability without advancing the shortest observable route:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR and Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1016
    return crispin_and_manual_complete_ggf;
  }

  bool wonder_tag_crispin_is_redundant_with_held_complete_route() const {
    if (wonder_tag_crispin_is_redundant_with_held_complete_route_issue980_original()) {
      return true;
    }
    if (!deck_seen_ || state_.manual_energy_used || !need_energy() || need_regi()) {
      return false;
    }

    const bool held_manual_completes_energy =
        (grass_needed() == 1 && fire_needed() == 0 &&
         hand_count(Card::Grass) > 0) ||
        (grass_needed() == 0 && fire_needed() == 1 &&
         hand_count(Card::Fire) > 0);
    if (!held_manual_completes_energy) return false;

    const bool vstar_card_held = hand_count(Card::RegidragoVstar) > 0;
    const bool active_regi_can_evolve = vstar_card_held && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn;
    const bool benched_vstar_exists = std::any_of(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoVstar;
        });
    const bool benched_regi_can_evolve = vstar_card_held && std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn;
        });
    const bool vstar_exists_or_evolvable =
        has_vstar() || active_regi_can_evolve || benched_regi_can_evolve;
    if (!vstar_exists_or_evolvable) return false;

    const bool benched_vstar_can_exist =
        benched_vstar_exists || benched_regi_can_evolve;
    const bool active_axis_without_new_bench = active_is_vstar() ||
        active_regi_can_evolve ||
        (benched_vstar_can_exist && can_free_retreat_with_latias());
    const bool held_latias_route = benched_vstar_can_exist && state_.active &&
        is_basic(state_.active->card) && hand_count(Card::LatiasEx) > 0 &&
        bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
        !state_.retreat_used;
    if (!active_axis_without_new_bench && !held_latias_route) return false;

    const bool held_burnet_route = supporter_allowed() &&
        hand_count(Card::ProfessorBurnet) > 0 &&
        can_play_payload_this_turn() && payload_might_be_in_deck();
    const bool held_payload_route = payload_ready() || held_burnet_route ||
        has_live_blender_payload_line() || has_live_serena_hand_payload_line() ||
        has_live_one_discard_hand_payload_line() ||
        has_live_ultra_ball_hand_payload_line();

    // A held Basic Energy plus the unused manual attachment completes Apex Dragon's
    // GGF cost. When held evolution, payload, and Active-position resources complete
    // every other current-turn axis, Wonder Tag into Crispin spends Tapu Lele-GX,
    // a Bench slot, and its one-shot Ability without advancing setup:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/989
    return held_payload_route;
  }

  bool needs_tapu_connector() {
    // A full-horizon Supporter lock makes every Wonder Tag target unplayable.
    // Spending a search Item, Tapu Lele-GX, and a Bench slot cannot advance a
    // live Supporter axis under that scenario, so preserve those resources:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Full-Supporter-lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-semantics
    // Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1443
    if (scenario_.locks == LockMode::FullSupporter) return false;

    // Preserve the existing Oricorio last-slot protection before the broader
    // complete-route suppression:
    // https://github.com/FlareZ123/pokemon-sims/issues/180
    // https://github.com/FlareZ123/pokemon-sims/issues/616
    if (bench_space() == 1 && hand_count(Card::Oricorio) > 0 &&
        !in_play(Card::Oricorio) && oricorio_preserves_burnet_with_current_hand()) {
      return false;
    }

    // This shared connector gate controls both a held Tapu Lele-GX and search Items
    // considering Tapu as their Basic target. Rejecting the redundant route here
    // prevents the needless Wonder Tag and the needless Quick Ball cost/search:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1016
    if (wonder_tag_is_redundant_with_held_crispin_legacy_star_route()) {
      return false;
    }

    if (wonder_tag_crispin_is_redundant_with_held_complete_route()) {
      return false;
    }

    if (needs_tapu_connector_original() && original_wonder_tag_route_has_live_target()) {
      return true;
    }
    if (future_turn_wonder_tag_route_has_live_target()) return true;

    // Wonder Tag into Tate & Liza remains live only when the promoted Benched VSTAR
    // already pays GGF and the held payload Item retains a legal post-search target:
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    const bool payload_axis_completable = !need_payload() ||
        (state_.deck.size() > 1U && has_live_one_discard_hand_payload_line());
    const bool direct_communication_latias_route = need_active_vstar() &&
        pokemon_communication_has_direct_target_route(Card::LatiasEx);
    const bool tate_completes_active_axis = need_active_vstar() &&
        benched_vstar_promotion_ready() && payload_axis_completable &&
        !can_free_retreat_with_latias() && !direct_communication_latias_route &&
        hand_count(Card::TateLiza) == 0 && might_be_unseen(Card::TateLiza);
    return supporter_allowed() && ability_available_for_pokemon(Card::TapuLeleGX) &&
           tate_completes_active_axis;
  }
