  bool blender_energy_axis_can_finish_this_turn() const {
    if (!strict_payload_timing() || !need_energy()) return true;

    int grass_missing = grass_needed();
    int fire_missing = fire_needed();
    int grass_in_hand = hand_count(Card::Grass);
    int fire_in_hand = hand_count(Card::Fire);

    // A held Crispin may still resolve after a late Item search and before the final
    // manual attachment. Simulate only the exact public/K1 route implemented by
    // play_crispin(), rather than assuming a future draw or Legacy Star recovery:
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // https://github.com/FlareZ123/pokemon-sims/issues/737
    if (supporter_allowed() && hand_count(Card::Crispin) > 0) {
      const bool grass_available = prizes_known()
          ? deck_count_after_search_started(Card::Grass) > 0
          : might_be_unseen(Card::Grass);
      const bool fire_available = prizes_known()
          ? deck_count_after_search_started(Card::Fire) > 0
          : might_be_unseen(Card::Fire);

      if (grass_available && fire_available) {
        if (grass_missing > 0) {
          --grass_missing;
          ++fire_in_hand;
        } else if (fire_missing > 0) {
          --fire_missing;
          ++grass_in_hand;
        }
      } else if (grass_available) {
        ++grass_in_hand;
      } else if (fire_available) {
        ++fire_in_hand;
      }
    }

    // The final manual attachment remains a legal same-turn route only while it has
    // not been used and a needed Basic Energy is already observable in hand. Vital
    // Dance and Earthen Vessel routes resolve before Blender and therefore appear
    // here as that held Energy:
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (!state_.manual_energy_used) {
      if (grass_missing > 0 && grass_in_hand > 0) {
        --grass_missing;
      } else if (fire_missing > 0 && fire_in_hand > 0) {
        --fire_missing;
      }
    }

    return grass_missing == 0 && fire_missing == 0;
  }

  bool play_brilliant_blender() {
    // Brilliant Blender may search for and discard up to 5 deck cards:
    // https://api.pokemontcg.io/v2/cards/sv8-164
    if (item_locked() || hand_count(Card::BrilliantBlender) == 0 ||
        !can_play_payload_this_turn() || !payload_might_be_in_deck() ||
        !blender_energy_axis_can_finish_this_turn()) {
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
