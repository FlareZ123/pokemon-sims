// Canonical shared owner for the K0 and K1 Steven / Brilliant Blender semantic
// route predicates. Section selection preserves each historical textual include
// boundary and its surrounding macro lifetime.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#if defined(SIM_TRACE_STEVEN_BLENDER_K0_SECTION) && \
    defined(SIM_TRACE_STEVEN_BLENDER_K1_SECTION)
#error "Steven/Blender semantic route composition selected multiple sections"
#endif

#if defined(SIM_TRACE_STEVEN_BLENDER_K0_SECTION)
  bool issue_3221_k0_steven_blender_route_available() const {
    // Brilliant Blender is an Item, and the registered TurnTwoItem scenario locks
    // Items from the player's second turn onward. Use the central projected-turn
    // predicate so this route cannot drift from the engine's lock contract.
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Persistent Item lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Shared implementation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
    const bool next_turn_items_locked = item_locked_on_turn(state_.turn + 1);

    // This is specifically the K0 variant. Any legal K1 source, including a complete
    // Prize inspection, leaves this helper and is handled by the known-composition route.
    // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // At K0, Steven's Resolve may establish the first physical deck inspection and
    // reserve the plausible VSTAR plus final Grass. Brilliant Blender is held for
    // the following same-ready-turn payload discard. Rule Box Ability lock does not
    // affect either Trainer; Item locks matter on the projected Blender turn.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced Supporter, Item, search, evolution, and attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Same-ready-turn JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Lock semantics: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3221
    if (prizes_known() || !strict_payload_timing() || next_turn_items_locked ||
        state_.turn >= scenario_.max_turn || !supporter_allowed() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 1 || state_.active->fire < 1 ||
        hand_count(Card::StevensResolve) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::RegidragoVstar) > 0 || hand_count(Card::Grass) > 0 ||
        !need_vstar() || !need_energy() || !need_payload() ||
        !payload_might_be_in_deck()) {
      return false;
    }

    // The active prior-turn Regidrago V is one Grass short of GGF. Steven can search
    // the plausible VSTAR and Grass, end the turn, then the held Blender supplies the
    // Dragon payload after the searched attachment and legal evolution next turn.
    // The route depends on state semantics rather than an absolute turn/profile/lock
    // witness; hidden targets remain conditional until Steven resolves the K0 search.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3221
    return grass_needed() == 1 && fire_needed() == 0 &&
        might_be_unseen(Card::RegidragoVstar) && might_be_unseen(Card::Grass);
  }
#elif defined(SIM_TRACE_STEVEN_BLENDER_K1_SECTION)
  bool issue_3222_steven_blender_fallback_available() const {
    // Brilliant Blender is an Item, and the registered TurnTwoItem scenario locks
    // Items from the player's second turn onward. Use the central projected-turn
    // predicate so this route cannot drift from the engine's lock contract.
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Persistent Item lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Shared implementation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_003.inc
    const bool next_turn_items_locked = item_locked_on_turn(state_.turn + 1);

    // This route consumes fixed-list deck knowledge, so both legal K1 sources are
    // accepted: a deck inspection or a complete Prize inspection. True K0 is rejected.
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Steven's Resolve is governed by current Supporter legality. Brilliant Blender
    // is an Item used on the following ready turn. Rule Box Ability lock does not
    // affect either Trainer, while a full/combined Item lock or a scheduled T2 Item
    // lock that lands on the projected Blender turn removes the route.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced Supporter, Item, evolution, and attachment procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Same-ready-turn JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Lock semantics: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Earliest-route objective: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3222
    if (!prizes_known() || !strict_payload_timing() || next_turn_items_locked ||
        state_.turn >= scenario_.max_turn || !supporter_allowed() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        hand_count(Card::StevensResolve) == 0 || !need_vstar() ||
        !need_payload() || !payload_might_be_in_deck() ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::BrilliantBlender) == 0) {
      return false;
    }

    // Exactly one Basic Energy remains missing and is already held. Steven can bank
    // the VSTAR and Blender, end the current turn, then the prior-turn Regidrago V
    // may receive the held completing attachment, evolve, and use Blender to put the
    // Dragon payload into discard on that same JIT-ready turn. Printed card text does
    // not impose an absolute turn, seat, or JIT-profile identity on this sequence.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3222
    const int missing_grass = grass_needed();
    const int missing_fire = fire_needed();
    const bool held_grass_completion =
        missing_grass == 1 && missing_fire == 0 && hand_count(Card::Grass) > 0;
    const bool held_fire_completion =
        missing_grass == 0 && missing_fire == 1 && hand_count(Card::Fire) > 0;
    return held_grass_completion || held_fire_completion;
  }
#else
#error "Steven/Blender semantic route owner included without a section selector"
#endif
