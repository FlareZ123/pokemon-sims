  bool issue_2270_wonder_tag_arven_latias_route_available() const {
    // Wonder Tag may find Arven; Arven may find Quick Ball; Quick Ball can discard
    // the held Dragon while searching Latias ex. Skyliner then gives the Basic
    // Active no Retreat Cost into the already powered Benched VSTAR.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Core Bench, Ability, Supporter, Item, discard, search, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and earliest-complete-route contracts: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // DDE semantic-energy contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2270
    if (!prizes_known() || !supporter_allowed() || item_locked() ||
        !can_play_payload_this_turn() || !need_payload() || need_regi() ||
        need_vstar() || need_energy() || !need_active_vstar() ||
        hand_count(Card::Arven) > 0 ||
        deck_count_after_search_started(Card::Arven) == 0 ||
        std::none_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        !state_.active || !is_basic(state_.active->card) || state_.retreat_used ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        !ability_available_for_pokemon(Card::LatiasEx) || in_play(Card::LatiasEx) ||
        deck_count_after_search_started(Card::QuickBall) == 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0) {
      return false;
    }

    const bool benched_powered_vstar = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          // Use the semantic Apex payment helper so one DDE plus one matching Basic
          // is accepted exactly like three matching Basic Energy.
          // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
          // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
          // Semantic helper specification: https://github.com/FlareZ123/pokemon-sims/issues/2238
          return pokemon.card == Card::RegidragoVstar &&
                 pays_apex_energy_cost(pokemon);
        });
    if (!benched_powered_vstar) return false;

    // Before Wonder Tag resolves, one Bench slot is needed for Tapu Lele-GX and a
    // second for Latias ex. Once Tapu is already in play, only the Latias slot must
    // remain for the continuation.
    // Bench procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2270
    const int required_open_slots = in_play(Card::TapuLeleGX) ? 1 : 2;
    return bench_space() >= required_open_slots;
  }

  bool needs_tapu_connector() {
    if (needs_tapu_connector_issue2270_original()) return true;

    // The late Tapu connector owns the full preflight. Once Wonder Tag establishes
    // K1, choose_supporter_after_search_started() legally falls back to Arven when
    // Arven is the live Supporter in the inspected deck; this avoids changing the
    // deliberately split part_006/part_007 implementation.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Post-search Supporter fallback implementation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_007.inc#L409-L425
    // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2270
    return issue_2270_wonder_tag_arven_latias_route_available();
  }
