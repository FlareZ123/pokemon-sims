  bool hand_covers_missing_basic_energy() const {
    const int missing_grass = grass_needed();
    const int missing_fire = fire_needed();
    // Keep the visible manual-attachment inventory check in one predicate so route
    // selectors do not reimplement the GGF hand-coverage arithmetic.
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Manual Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    return hand_count(Card::Grass) >= missing_grass &&
           hand_count(Card::Fire) >= missing_fire;
  }

  bool complete_two_turn_manual_energy_schedule() const {
    const int remaining_grass = grass_needed();
    const int remaining_fire = fire_needed();
    // This route consumes one manual attachment on each of the next two turns.
    // Keep both the attachment count and visible hand coverage in one reusable
    // predicate while preserving the repository's DDE static-contract source shape.
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Double Dragon Energy / semantic Apex payment: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // DDE migration contract: https://github.com/FlareZ123/pokemon-sims/issues/2368
    // One manual Energy attachment per turn: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    return remaining_grass + remaining_fire == 2 &&
           hand_covers_missing_basic_energy();
  }

  bool fss_blender_bank_with_complete_manual_schedule_available() const {
    // A resolved deck search or complete Prize inspection establishes the same
    // fixed-list K1. This route reads exact remaining-deck inventory, so it must
    // use the repository-wide knowledge predicate rather than one provenance:
    // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2067
    if (state_.turn != 1 || scenario_.going_first ||
        scenario_.dci != DciProfile::NoDiscardControl ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 3 ||
        !prizes_known() || supporter_allowed() || !state_.manual_energy_used ||
        item_locked() || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::RegidragoVstar) == 0 ||
        !need_payload() || !can_play_payload_this_turn() ||
        !payload_might_be_in_deck() ||
        deck_count_after_search_started(Card::BrilliantBlender) == 0) {
      return false;
    }

    // No-discard-control permits Brilliant Blender to bank the Dragon payload now.
    // A held Regidrago VSTAR plus the two visible future manual attachments then
    // guarantees readiness by T3, while Steven's Resolve would end a later turn
    // without advancing any unresolved axis in the current public state:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // One manual Energy attachment per turn and evolution timing:
    // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // No-discard-control and earliest-route policy:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1735
    return complete_two_turn_manual_energy_schedule();
  }

  Card fss_target_after_search_started() const {
    const Card original = fss_target_after_search_started_issue1735_original();
    if (original == Card::StevensResolve &&
        fss_blender_bank_with_complete_manual_schedule_available()) {
      return Card::BrilliantBlender;
    }
    return original;
  }

  bool issue_1749_existing_regi_ready_route_without_backup() const {
    // A second Regidrago V is optional Bench placement and has no setup effect
    // when the existing Active Regidrago V already has a complete current-turn
    // route through Star Alchemy, the held final Energy, evolution, and Blender:
    // Regidrago V / Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Hisuian Heavy Ball / full Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Bench and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original route bug / confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/1749 https://github.com/FlareZ123/pokemon-sims/issues/2068
    if (!prizes_known() || state_.turn_ended || item_locked() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        hand_count(Card::RegidragoVstar) == 0) {
      return false;
    }

    const bool energy_route_live = pays_apex_energy_cost(*state_.active) ||
        (!state_.manual_energy_used &&
         completing_basic_energy_for(*state_.active, [this](const Card card) {
           return hand_count(card) > 0;
         }).has_value());
    // DDE-aware physical attachment distance:
    // https://github.com/FlareZ123/pokemon-sims/issues/2368
    if (!energy_route_live) return false;

    const auto live_fss_holder = [this](const Pokemon& pokemon) {
      return pokemon.tool == Tool::ForestSealStone &&
          is_pokemon_v(pokemon.card) &&
          ability_available_for_pokemon(pokemon.card);
    };
    const bool live_star_alchemy =
        (state_.active && live_fss_holder(*state_.active)) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), live_fss_holder);
    const bool blender_route_live = hand_count(Card::BrilliantBlender) > 0 ||
        (!state_.vstar_power_used && live_star_alchemy &&
         deck_count_after_search_started(Card::BrilliantBlender) > 0);

    return blender_route_live && can_play_payload_this_turn() &&
        payload_might_be_in_deck();
  }

  int in_play_count_issue1749(const Card card) const {
    if (card == Card::RegidragoV &&
        issue_1749_existing_regi_ready_route_without_backup()) {
      // The generic branch benches only while fewer than two Regidrago V are in
      // play. Returning its threshold here suppresses that one dominated action
      // while every incomplete or K0 state delegates to the physical count:
      // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_013.inc
      // https://github.com/FlareZ123/pokemon-sims/issues/1749
      return 2;
    }
    return in_play_count(card);
  }

#define in_play_count in_play_count_issue1749
