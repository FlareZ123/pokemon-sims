  bool blender_energy_axis_can_finish_this_turn() {
    if (!strict_payload_timing()) return true;

    const Pokemon* target = target_regi();
    if (target == nullptr || target->card != Card::RegidragoVstar) return true;

    // Evaluate the intended attacker even while a Basic support Pokémon is Active.
    // A Latias ex promotion route solves only the Retreat axis; the selected Benched
    // Regidrago VSTAR must still be able to reach Apex Dragon's GGF cost before a
    // strict-JIT Brilliant Blender payload expires:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/issues/1031
    const int grass_missing = std::max(0, 2 - target->grass);
    const int fire_missing = std::max(0, 1 - target->fire);
    const int energy_missing = grass_missing + fire_missing;
    if (energy_missing == 0) return true;

    // Strict JIT recognizes only a Dragon discarded during the same turn that the
    // Active Regidrago VSTAR reaches Apex Dragon's GGF cost. Brilliant Blender is
    // a one-use ACE SPEC, so preserve it after every observable same-turn Energy
    // route has closed:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/737
    const bool manual_attachment_available = !state_.manual_energy_used;
    const bool one_energy_missing = energy_missing == 1;
    const Card final_energy = grass_missing > 0 ? Card::Grass : Card::Fire;
    if (manual_attachment_available && one_energy_missing &&
        hand_count(final_energy) > 0) {
      return true;
    }

    const bool final_energy_might_be_in_deck = might_be_unseen(final_energy);
    const bool held_oricorio_route = manual_attachment_available &&
        one_energy_missing && final_energy_might_be_in_deck &&
        hand_count(Card::Oricorio) > 0 && !in_play(Card::Oricorio) &&
        bench_space() > 0 && ability_available_for_pokemon(Card::Oricorio);
    if (held_oricorio_route) {
      // Vital Dance searches Basic Energy into hand, after which the unused manual
      // attachment can supply the final GGF component:
      // https://api.pokemontcg.io/v2/cards/sm2-55
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/issues/737
      return true;
    }

    bool held_vessel_route = false;
    if (manual_attachment_available && one_energy_missing &&
        final_energy_might_be_in_deck && !item_locked() &&
        hand_count(Card::EarthenVessel) > 0) {
      const std::vector<Card> original_hand = state_.hand;
      remove_one(state_.hand, Card::BrilliantBlender);
      held_vessel_route =
          choose_discard(false, true, true, Card::EarthenVessel).has_value();
      state_.hand = original_hand;
    }
    if (held_vessel_route) {
      // Earthen Vessel may search the final Basic Energy only when another card can
      // pay its discard cost while the held Blender remains protected:
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://github.com/FlareZ123/pokemon-sims/issues/737
      return true;
    }

    if (!supporter_allowed() || hand_count(Card::Crispin) == 0) return false;

    const bool grass_available = prizes_known()
        ? deck_count_after_search_started(Card::Grass) > 0
        : might_be_unseen(Card::Grass);
    const bool fire_available = prizes_known()
        ? deck_count_after_search_started(Card::Fire) > 0
        : might_be_unseen(Card::Fire);
    if (!grass_available && !fire_available) return false;

    int remaining_grass = grass_missing;
    int remaining_fire = fire_missing;
    int grass_in_hand = hand_count(Card::Grass);
    int fire_in_hand = hand_count(Card::Fire);

    // Crispin attaches one searched type only when both Grass and Fire are present;
    // the other enters hand. With one searchable type, it attaches none and puts that
    // Energy into hand, matching the active resolver:
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_011.inc#L72-L108
    // https://github.com/FlareZ123/pokemon-sims/issues/737
    if (grass_available && fire_available) {
      if (remaining_grass > 0) {
        --remaining_grass;
        ++fire_in_hand;
      } else if (remaining_fire > 0) {
        --remaining_fire;
        ++grass_in_hand;
      }
    } else if (grass_available) {
      ++grass_in_hand;
    } else {
      ++fire_in_hand;
    }

    if (remaining_grass + remaining_fire == 0) return true;
    if (!manual_attachment_available ||
        remaining_grass + remaining_fire != 1) {
      return false;
    }
    return (remaining_grass == 1 && grass_in_hand > 0) ||
           (remaining_fire == 1 && fire_in_hand > 0);
  }

  bool blender_active_axis_can_finish_this_turn() const {
    if (!strict_payload_timing() || !need_active_vstar()) return true;

    // Issue #725 is specifically the loss of the one-shot ACE SPEC while a complete
    // Regidrago VSTAR is stranded on the Bench. Preserve the repository's existing
    // policy for Energy-incomplete targets, which are handled by the narrower Tate and
    // Forest Seal Stone guards below:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/725
    if (!benched_vstar_promotion_ready()) return true;
    if (can_free_retreat_with_latias()) return true;

    // Tate & Liza switches unconditionally. Professor Turo's Scenario may remove a
    // Basic Active. Guzma completes the same promotion only when the scenario proves
    // an eligible opposing Benched Pokémon for its printed first switch:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sv4-171
    // https://api.pokemontcg.io/v2/cards/sm3-115
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#guzma-opponent-bench-prerequisite
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool supporter_promotion_route = supporter_allowed() &&
        (hand_count(Card::TateLiza) > 0 ||
         (hand_count(Card::ProfessorTuro) > 0 && state_.active &&
          is_basic(state_.active->card)) ||
         (hand_count(Card::Guzma) > 0 &&
          scenario_.opponent_bench == OpponentBenchState::Available));
    if (supporter_promotion_route) return true;

    // Star Alchemy may search Tate & Liza through a Rule Box Ability lock because the
    // Ability belongs to Forest Seal Stone. A live attached Tool therefore completes
    // the same-turn promotion route when Tate remains a legal unseen search target:
    // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    const auto live_fss_holder = [](const Pokemon& pokemon) {
      return pokemon.tool == Tool::ForestSealStone && is_pokemon_v(pokemon.card);
    };
    const bool star_alchemy_to_tate =
        !state_.vstar_power_used && supporter_allowed() &&
        hand_count(Card::TateLiza) == 0 && might_be_unseen(Card::TateLiza) &&
        ((state_.active && live_fss_holder(*state_.active)) ||
         std::any_of(state_.bench.begin(), state_.bench.end(), live_fss_holder));
    if (star_alchemy_to_tate) return true;

    // A held Latias ex can be played to the open Bench before the final retreat step.
    // Skyliner then removes the Basic Active's Retreat Cost. Bench capacity, Ability
    // availability, and the once-per-turn retreat state are all observable now:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    return state_.active && is_basic(state_.active->card) &&
        hand_count(Card::LatiasEx) > 0 && !in_play(Card::LatiasEx) &&
        bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
        !state_.retreat_used;
  }

  bool play_brilliant_blender() {
    // Brilliant Blender may search for and discard up to 5 deck cards:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 ||
        !can_play_payload_this_turn() || !payload_might_be_in_deck() ||
        !blender_energy_axis_can_finish_this_turn() ||
        !blender_active_axis_can_finish_this_turn()) {
      return false;
    }

    // A public Dragon payload plus a legal hand-discard outlet already completes
    // the current-turn payload axis. Preserve the one-copy ACE SPEC and the deck
    // payloads instead of duplicating that role:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/976
    // https://github.com/FlareZ123/pokemon-sims/issues/1339
    const bool payload_already_held =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    bool live_one_discard_payload_outlet = false;
    if (payload_already_held && deck_search_available()) {
      for (const Card item : {Card::MysteriousTreasure, Card::QuickBall,
                              Card::EarthenVessel}) {
        if (hand_count(item) == 0) continue;
        const bool known_dead_mysterious = item == Card::MysteriousTreasure &&
            deck_seen_ && std::none_of(state_.deck.begin(), state_.deck.end(),
                                      is_dragon_or_psychic);
        const bool known_dead_quick = item == Card::QuickBall && deck_seen_ &&
            std::none_of(state_.deck.begin(), state_.deck.end(), is_basic);
        if (known_dead_mysterious || known_dead_quick ||
            (item == Card::EarthenVessel && !earthen_vessel_has_legal_target())) {
          continue;
        }
        const auto cost = choose_discard(true, true, true, item);
        if (cost && is_payload(*cost)) {
          live_one_discard_payload_outlet = true;
          break;
        }
      }
    }
    bool live_ultra_ball_payload_outlet = false;
    if (payload_already_held && hand_count(Card::UltraBall) > 0 &&
        ultra_ball_has_legal_target()) {
      Engine simulated = *this;
      const auto payload_cost =
          simulated.choose_discard(true, true, true, Card::UltraBall);
      if (payload_cost && is_payload(*payload_cost) &&
          remove_one(simulated.state_.hand, *payload_cost)) {
        live_ultra_ball_payload_outlet =
            simulated.choose_discard(false, true, true, Card::UltraBall).has_value();
      }
    }
    const bool live_serena_payload_outlet = payload_already_held &&
        supporter_allowed() && hand_count(Card::Serena) > 0;
    if (live_one_discard_payload_outlet || live_ultra_ball_payload_outlet ||
        live_serena_payload_outlet) {
      return false;
    }

    const int supporters_in_hand = static_cast<int>(
        std::count_if(state_.hand.begin(), state_.hand.end(), is_supporter));
    const bool only_tate_supporter_held =
        supporters_in_hand == hand_count(Card::TateLiza);
    // Brilliant Blender is a one-shot deck-to-discard payload outlet. Under strict
    // current-turn timing, retain it while Tate & Liza would promote a VSTAR that
    // still lacks part of Apex Dragon's GGF cost. Inspect the actual promotion target
    // rather than generic Energy routing to another Regidrago line:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    if (strict_payload_timing() && only_tate_supporter_held &&
        hand_count(Card::TateLiza) > 0 && need_active_vstar() &&
        !benched_vstar_promotion_ready()) {
      return false;
    }

    // Path to the Peak suppresses Abilities on Rule Box Pokémon, while Forest Seal
    // Stone remains usable because Star Alchemy is the Tool's Ability. Treat the
    // attached Tool as live independently of the holder's own Ability availability:
    // https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    const auto live_fss_holder = [](const Pokemon& pokemon) {
      return pokemon.tool == Tool::ForestSealStone && is_pokemon_v(pokemon.card);
    };
    const bool forest_seal_stone_is_live =
        (state_.active && live_fss_holder(*state_.active)) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), live_fss_holder);
    const bool fss_tate_incomplete_promotion = strict_payload_timing() &&
        !state_.vstar_power_used && supporter_allowed() && need_active_vstar() &&
        !need_energy() && !benched_vstar_promotion_ready() &&
        !can_free_retreat_with_latias() && hand_count(Card::TateLiza) == 0 &&
        might_be_unseen(Card::TateLiza) && forest_seal_stone_is_live;
    if (fss_tate_incomplete_promotion) {
      // Star Alchemy can search Tate & Liza, but the actual Benched Regidrago VSTAR
      // must already pay Apex Dragon's GGF cost before the one-shot Blender payload
      // is useful this turn. Preserve Blender in the same incomplete connector state
      // where the Forest Seal Stone selector preserves the VSTAR Power:
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://api.pokemontcg.io/v2/cards/sm7-148
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_011_fss_latias_override.inc#L4-L24
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      return false;
    }

    remove_one(state_.hand, Card::BrilliantBlender);
    state_.discard.push_back(Card::BrilliantBlender);
    std::vector<Card> discarded;
    // Apex Dragon copies attacks from Dragon Pokémon in discard. Prioritize every
    // modeled A/S Dragon payload before Dipplin or generic deck compression:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv6-130
    // https://api.pokemontcg.io/v2/cards/swsh11-136
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // Appletun is a Dragon Pokémon and a modeled Apex Dragon payload:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1256
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::GoodraVstar, Card::DialgaGX,
                               Card::Appletun}) {
      if (discarded.size() >= 5U) break;
      if (move_deck_to_discard(payload)) discarded.push_back(payload);
    }
    if (discarded.size() < 5U && move_deck_to_discard(Card::Dipplin)) {
      discarded.push_back(Card::Dipplin);
    }

    // In no-discard-control, the engine protects only resources needed immediately.
    // Before a legal same-turn draw refresh, use Blender's remaining selections on
    // cards that cannot advance this no-lock setup state. This improves the refresh
    // without spending recovery or live connector cards:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    const bool tate_refresh_available = hand_count(Card::TateLiza) > 0;
    const bool serena_refresh_available =
        hand_count(Card::Serena) > 0 && choose_discard(false, true).has_value();
    const bool thin_before_refresh =
        scenario_.dci == DciProfile::NoDiscardControl &&
        scenario_.locks == LockMode::None && supporter_allowed() &&
        setup_axis_missing() &&
        (tate_refresh_available || serena_refresh_available);
    if (thin_before_refresh) {
      // Erika's Invitation replaced retired Mawile-GX in the canonical recipe and is
      // opponent-dependent in this goldfish model. Use spare legal Blender capacity
      // to remove it before the same-turn refresh:
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://api.pokemontcg.io/v2/cards/sv3pt5-160
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
      // https://github.com/FlareZ123/pokemon-sims/issues/881
      for (const Card card : {Card::PathToPeak, Card::ErikasInvitation, Card::Guzma,
                              Card::Channeler, Card::FieldBlower,
                              Card::ChaoticSwell}) {
        if (discarded.size() >= 5U) break;
        if (move_deck_to_discard(card)) discarded.push_back(card);
      }
    }

    shuffle(state_.deck);
    outcome_.used_blender = true;
    trace("PLAY ITEM", "R-BLENDER-01; R-GAME-ITEM; P-DCI-01",
          "Searched and discarded: " + join_cards(discarded) + ".");
    return true;
  }
