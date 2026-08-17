  class ActiveVstarStevenProjectionPolicy final {
   public:
    [[nodiscard]] static constexpr int failing_topdecks(
        const int grass_after_steven, const int fire_in_deck) {
      // Preserve the established known-deck topdeck-risk model for Steven's
      // Resolve -> Crispin. After Steven banks one Grass, a lone remaining Grass
      // or lone Fire is counted as a failing next draw for the deterministic
      // continuation. This class owns only the pure arithmetic projection; Engine
      // keeps K1 visibility, route admission, lock checks, DCI/JIT, and actions.
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Route-risk provenance: https://github.com/FlareZ123/pokemon-sims/issues/1098
      // Cleanup architecture: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
      return static_cast<int>(grass_after_steven == 1) +
             static_cast<int>(fire_in_deck == 1);
    }

    [[nodiscard]] static constexpr bool continuation_survives_next_draw(
        const int post_steven_deck_size, const int grass_after_steven,
        const int fire_in_deck) {
      return post_steven_deck_size >
             failing_topdecks(grass_after_steven, fire_in_deck);
    }
  };

  bool issue_3203_active_vstar_steven_crispin_treasure_route_available() const {
    const bool next_turn_items_locked = item_locked_on_turn(state_.turn + 1);
    // Shared projected Item-lock timing: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc

    // Steven ends the current turn after reserving Crispin plus one Grass. On the
    // following turn Crispin supplies one Grass, the normal manual attachment supplies
    // the second, and held Mysterious Treasure discards the held Dragon on the exact
    // ready turn. StrictJit and MatchupFlexJit therefore share this continuation.
    // TurnTwoItem and Combined begin on T2 and remain active thereafter, so they block
    // every projected Treasure turn from T2 onward. Rule Box Ability suppression does
    // not disable these Trainer/attachment actions.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced Supporter, Item, discard, search, attachment, and turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Persistent T2 Item-lock specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // K1/JIT/lock/priority policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3203
    // Canonical payload membership: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/payload_hand_policy.inc
    // The current turn's manual attachment cannot consume the following turn's
    // attachment because begin_turn() resets manual_energy_used before that turn:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
    // https://github.com/FlareZ123/pokemon-sims/issues/4130
    if (!prizes_known() || !strict_payload_timing() || next_turn_items_locked ||
        state_.turn >= scenario_.max_turn || !supporter_allowed() ||
        !state_.active || state_.active->card != Card::RegidragoVstar ||
        state_.active->grass != 0 || state_.active->fire < 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::Grass) > 0 || hand_count(Card::Crispin) > 0 ||
        !need_energy() || !need_payload() ||
        !payload_zone_contains(state_.hand)) {
      return false;
    }

    const int grass_in_deck = deck_count_after_search_started(Card::Grass);
    const int fire_in_deck = deck_count_after_search_started(Card::Fire);
    const int treasure_targets =
        PayloadZonePolicy::count_matching(state_.deck, is_dragon_or_psychic);
    if (deck_count_after_search_started(Card::Crispin) == 0 ||
        grass_in_deck < 2 || fire_in_deck == 0 || treasure_targets == 0) {
      return false;
    }

    const int post_steven_deck_size = static_cast<int>(state_.deck.size()) - 2;
    const int grass_after_steven = grass_in_deck - 1;

    // Preserve #1098's known-deck topdeck-risk model while removing only historical
    // profile, seat, absolute-turn, and blanket no-lock coordinates from admission.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Route-risk provenance: https://github.com/FlareZ123/pokemon-sims/issues/1098
    // Semantic-admission correction: https://github.com/FlareZ123/pokemon-sims/issues/3203
    return ActiveVstarStevenProjectionPolicy::continuation_survives_next_draw(
        post_steven_deck_size, grass_after_steven, fire_in_deck);
  }
