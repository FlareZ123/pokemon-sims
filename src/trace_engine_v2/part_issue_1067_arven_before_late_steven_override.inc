#undef play_steven
#if defined(TRACE_ENGINE_FINAL_PLAY_STEVEN_ALIAS)
// The final Steven dispatcher is defined by the issue-2622 overlay included below.
// Let composition layers rename that one public method without interfering with
// the issue-1821 -> issue-1030 -> issue-2622 internal wrapper chain.
// C++ macro replacement: https://eel.is/c++draft/cpp.replace
#define play_steven TRACE_ENGINE_FINAL_PLAY_STEVEN_ALIAS
#endif

// Include the canonical late-Steven/Burnet route directly at its proven member boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Canonical route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/late_steven_burnet_route_policy.inc
#include "core/routes/late_steven_burnet_route_policy.inc"

  bool known_hand_or_deck_card_available(const Card card) const {
    // Once the deck is known, route package checks should express card access once
    // rather than duplicate hand-or-known-deck predicates at every call site.
    // K1 knowledge-state policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    return hand_count(card) > 0 || deck_count_after_search_started(card) > 0;
  }

  bool held_arven_fss_blender_finish_available() const {
    const bool forest_seal_ability_available_this_turn =
        ability_available_for_pokemon(Card::RegidragoV) ||
        hand_count(Card::ChaoticSwell) > 0;
    const bool payload_in_known_deck = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_payload);

    if (!deck_seen_ || !supporter_allowed() || item_locked() ||
        scenario_.dci != DciProfile::StrictJit || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        !pays_apex_energy_cost(*state_.active) ||
        state_.active->tool != Tool::None || state_.vstar_power_used ||
        hand_count(Card::Arven) == 0 || !need_vstar() || !need_payload() ||
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        !forest_seal_ability_available_this_turn || !payload_in_known_deck) {
      return false;
    }

    const bool forest_seal_stone_available =
        known_hand_or_deck_card_available(Card::ForestSealStone);
    const bool brilliant_blender_available =
        known_hand_or_deck_card_available(Card::BrilliantBlender);

    // DDE + either Basic already pays Apex Dragon on the Dragon Regidrago V that
    // will evolve, so the held-Arven fast route must use semantic Energy readiness:
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2426
    // Arven can search one Item and one Pokémon Tool. Forest Seal Stone then searches
    // the missing Regidrago VSTAR, the prior-turn Basic may evolve, and Brilliant
    // Blender supplies the strict-JIT Dragon payload during the same turn. Steven's
    // Resolve would end that turn and delay an already complete legal route:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Chaotic Swell can replace the modeled Path before Star Alchemy: https://api.pokemontcg.io/v2/cards/sm12-187
    // Supporter, Stadium, Tool, Ability, evolution, and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 deck inspection and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1067
    return forest_seal_stone_available && brilliant_blender_available;
  }

  // Canonical semantic Steven/Blender route owners:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/steven_blender_semantic_policy.inc
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/k0_steven_blender_semantic_policy.inc
  #include "core/routes/steven_blender_semantic_policy.inc"
  #include "core/routes/k0_steven_blender_semantic_policy.inc"
  // Canonical issue-3203 route owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/active_vstar_steven_route_policy.inc
  #include "core/routes/active_vstar_steven_route_policy.inc"

  bool late_steven_package_available() const {
    // Centralize semantic late-Steven package checks so the dispatcher only chooses
    // between a current-turn Arven finish and the best legal Steven package.
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Latias/Blender package: https://github.com/FlareZ123/pokemon-sims/issues/2622
    // Burnet package: https://github.com/FlareZ123/pokemon-sims/issues/3223
    // K1 semantic Blender fallback: https://github.com/FlareZ123/pokemon-sims/issues/3222
    // K0 semantic Blender fallback: https://github.com/FlareZ123/pokemon-sims/issues/3221
    // Active-VSTAR Crispin/Treasure package: https://github.com/FlareZ123/pokemon-sims/issues/3203
    return issue_2622_steven_latias_blender_package_available() ||
           late_steven_burnet_route_available() ||
           issue_3222_steven_blender_fallback_available() ||
           issue_3221_k0_steven_blender_route_available() ||
           issue_3203_active_vstar_steven_crispin_treasure_route_available();
  }

  bool should_play_steven() const {
    if (held_arven_fss_blender_finish_available()) return false;
    // When Arven cannot complete the current turn, prefer a K1-proven Steven package
    // that guarantees the next-turn setup represented by the semantic package checks.
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Earliest deterministic route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (late_steven_package_available()) return true;
    return should_play_steven_issue1067_original();
  }

  // Keep exact source-bounded Supporter packages together before the final dispatcher.
  // Canonical Steven package owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/steven_package_policy.inc
  #include "core/routes/steven_package_policy.inc"