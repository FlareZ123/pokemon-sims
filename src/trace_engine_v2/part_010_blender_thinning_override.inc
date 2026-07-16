  bool play_brilliant_blender() {
    // Brilliant Blender may search for and discard up to 5 deck cards:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 ||
        !can_play_payload_this_turn() || !payload_might_be_in_deck()) {
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
    const bool forest_seal_stone_is_live = !state_.vstar_power_used &&
        ((state_.active && live_fss_holder(*state_.active)) ||
         std::any_of(state_.bench.begin(), state_.bench.end(), live_fss_holder));

    if (strict_payload_timing() && need_energy()) {
      const int missing_grass = grass_needed();
      const int missing_fire = fire_needed();
      const int missing_energy = missing_grass + missing_fire;
      const auto energy_available = [this](const Card energy) {
        return deck_seen_ ? deck_count_after_search_started(energy) > 0
                          : might_be_unseen(energy);
      };
      const bool grass_available = energy_available(Card::Grass);
      const bool fire_available = energy_available(Card::Fire);
      const bool needed_energy_available =
          (missing_grass > 0 && grass_available) ||
          (missing_fire > 0 && fire_available);
      const bool held_manual_attachment = !state_.manual_energy_used &&
          missing_energy == 1 &&
          ((missing_grass == 1 && hand_count(Card::Grass) > 0) ||
           (missing_fire == 1 && hand_count(Card::Fire) > 0));

      const auto crispin_completes_energy_axis = [&]() {
        if (!supporter_allowed() || !crispin_can_advance_energy_axis()) return false;
        if (missing_grass == 1 && missing_fire == 0) {
          return grass_available && (fire_available || !state_.manual_energy_used);
        }
        if (missing_grass == 0 && missing_fire == 1) {
          return fire_available && (grass_available || !state_.manual_energy_used);
        }
        return missing_grass == 1 && missing_fire == 1 &&
            !state_.manual_energy_used && grass_available && fire_available;
      };
      const bool held_crispin_route = hand_count(Card::Crispin) > 0 &&
          crispin_completes_energy_axis();

      const bool held_oricorio_route = !state_.manual_energy_used &&
          missing_energy == 1 && needed_energy_available && bench_space() > 0 &&
          hand_count(Card::Oricorio) > 0 &&
          ability_available_for_pokemon(Card::Oricorio);

      const auto vessel_cost_survives_blender = [this]() {
        const std::vector<Card> original_hand = state_.hand;
        if (!remove_one(state_.hand, Card::BrilliantBlender)) return false;
        const bool payable = choose_discard(false, true, true,
                                             Card::EarthenVessel).has_value();
        state_.hand = original_hand;
        return payable;
      };
      const bool held_vessel_route = !state_.manual_energy_used &&
          missing_energy == 1 && needed_energy_available && !item_locked() &&
          hand_count(Card::EarthenVessel) > 0 && vessel_cost_survives_blender();

      const bool star_alchemy_energy_route = forest_seal_stone_is_live &&
          ((!state_.manual_energy_used && missing_energy == 1 &&
            needed_energy_available) ||
           (hand_count(Card::Crispin) == 0 && might_be_unseen(Card::Crispin) &&
            crispin_completes_energy_axis()));

      // Strict JIT recognizes only a Dragon payload discarded during the same turn
      // that the Active Regidrago VSTAR can pay Apex Dragon's GGF cost. Preserve the
      // one-shot ACE SPEC after the manual attachment window is lost unless a concrete
      // current-turn accelerator remains observable:
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://www.pokemon.com/us/pokemon-tcg/rules
      // https://api.pokemontcg.io/v2/cards/sv7-133
      // https://api.pokemontcg.io/v2/cards/sm2-55
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://api.pokemontcg.io/v2/cards/swsh12-156
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
      // https://github.com/FlareZ123/pokemon-sims/issues/737
      if (!held_manual_attachment && !held_crispin_route &&
          !held_oricorio_route && !held_vessel_route &&
          !star_alchemy_energy_route) {
        return false;
      }
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

    const bool fss_tate_incomplete_promotion = strict_payload_timing() &&
        forest_seal_stone_is_live && supporter_allowed() && need_active_vstar() &&
        !need_energy() && !benched_vstar_promotion_ready() &&
        !can_free_retreat_with_latias() && hand_count(Card::TateLiza) == 0 &&
        might_be_unseen(Card::TateLiza);
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
