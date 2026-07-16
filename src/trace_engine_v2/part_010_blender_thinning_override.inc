  bool blender_energy_axis_can_finish_this_turn() {
    if (!strict_payload_timing() || !state_.active ||
        state_.active->card != Card::RegidragoVstar) {
      return true;
    }

    const int grass_missing = std::max(0, 2 - state_.active->grass);
    const int fire_missing = std::max(0, 1 - state_.active->fire);
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
    const Pokemon* target = target_regi();
    if (target != &*state_.active) return false;

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

  bool strict_jit_blender_active_axis_can_finish_this_turn() {
    if (!strict_payload_timing() || !need_active_vstar() ||
        !benched_vstar_promotion_ready()) {
      return true;
    }

    const bool held_latias_route = state_.active &&
        is_basic(state_.active->card) && hand_count(Card::LatiasEx) > 0 &&
        !in_play(Card::LatiasEx) && bench_space() > 0 &&
        ability_available_for_pokemon(Card::LatiasEx) && !state_.retreat_used;
    const bool held_tate_route = supporter_allowed() &&
        hand_count(Card::TateLiza) > 0;
    const bool held_guzma_route =
        scenario_.opponent_bench == OpponentBenchState::Available &&
        supporter_allowed() && hand_count(Card::Guzma) > 0;
    const bool active_regi_can_evolve_ready = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass >= 2 && state_.active->fire >= 1 &&
        (hand_count(Card::RegidragoVstar) > 0 ||
         held_item_can_find_pending_vstar());

    const auto live_fss_holder = [](const Pokemon& pokemon) {
      return pokemon.tool == Tool::ForestSealStone &&
             is_pokemon_v(pokemon.card);
    };
    const bool forest_seal_stone_is_live =
        !state_.vstar_power_used && supporter_allowed() &&
        ((state_.active && live_fss_holder(*state_.active)) ||
         std::any_of(state_.bench.begin(), state_.bench.end(),
                     live_fss_holder));
    const bool fss_latias_route = forest_seal_stone_is_live && state_.active &&
        is_basic(state_.active->card) && !state_.retreat_used && bench_space() > 0 &&
        !in_play(Card::LatiasEx) && ability_available_for_pokemon(Card::LatiasEx) &&
        might_be_unseen(Card::LatiasEx);
    const bool fss_tate_route = forest_seal_stone_is_live &&
        might_be_unseen(Card::TateLiza);

    // Strict JIT counts Blender's Dragon only during this turn. Preserve the
    // one-copy ACE SPEC while a powered Benched Regidrago VSTAR has no observable
    // same-turn route to the Active Spot. Guzma counts only when the scenario
    // explicitly guarantees the opposing Benched target required by its text:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sm3-115
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#guzma-opponent-bench-prerequisite
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/725
    // https://github.com/FlareZ123/pokemon-sims/issues/741
    return can_free_retreat_with_latias() || held_latias_route ||
           held_tate_route || held_guzma_route ||
           active_regi_can_evolve_ready || fss_latias_route ||
           fss_tate_route;
  }

  bool play_brilliant_blender() {
    // Brilliant Blender may search for and discard up to 5 deck cards:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 ||
        !can_play_payload_this_turn() || !payload_might_be_in_deck() ||
        !blender_energy_axis_can_finish_this_turn() ||
        !strict_jit_blender_active_axis_can_finish_this_turn()) {
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
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::GoodraVstar, Card::DialgaGX}) {
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
      for (const Card card : {Card::PathToPeak, Card::MawileGX, Card::Guzma,
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
