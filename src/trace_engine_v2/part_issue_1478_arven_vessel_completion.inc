  bool issue_1478_t3_arven_vessel_completion_available() const {
    // The completion packet is governed by observable action legality and the
    // same-ready-turn JIT payload contract. The original seed's seat, literal T3,
    // and no-lock identity are not card requirements once K1 and the finished
    // Forest Seal Stone state are already public.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, Tool, search, discard, attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Same-ready-turn JIT and lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3318
    if (!strict_payload_timing() || state_.turn > scenario_.max_turn ||
        item_locked() || !deck_seen_ || !supporter_allowed() ||
        state_.supporter_used || state_.manual_energy_used ||
        !state_.vstar_power_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar ||
        state_.active->grass != 1 || state_.active->fire != 1 ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::Arven) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::TeamYellsCheer) == 0 ||
        hand_count(Card::Grass) > 0 || !need_energy() || !need_payload()) {
      return false;
    }

    int deck_payloads = 0;
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      deck_payloads += deck_count_after_search_started(card);
    }
    return deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_payloads > 0;
  }

  bool play_issue_1478_t3_arven_vessel_completion() {
    if (!issue_1478_t3_arven_vessel_completion_available()) return false;

    // The earlier Treasure route has already established K1. Arven searches
    // Earthen Vessel, and the deterministic current-turn GGF plus Blender finish
    // makes Team Yell's Cheer dynamically discardable. Vessel may choose one
    // Grass, the unused manual attachment supplies it, and Blender then establishes
    // the Dragon payload on the same ready turn.
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Team Yell's Cheer: https://api.pokemontcg.io/v2/cards/swsh9-149
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, discard, search, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Original route bug: https://github.com/FlareZ123/pokemon-sims/issues/1478
    // Semantic-admission follow-up: https://github.com/FlareZ123/pokemon-sims/issues/3318
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven");
    if (!move_deck_to_hand(Card::EarthenVessel)) {
      throw std::logic_error(
          "projected issue-1478 Earthen Vessel target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01",
          "Searched Earthen Vessel for the final Grass Energy route.");

    if (!remove_one(state_.hand, Card::EarthenVessel)) {
      throw std::logic_error(
          "projected issue-1478 Earthen Vessel disappeared from hand");
    }
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(Card::TeamYellsCheer,
                           "Earthen Vessel deterministic completion cost",
                           "R-EV-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error(
          "projected issue-1478 Team Yell's Cheer cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel");
    if (!move_deck_to_hand(Card::Grass)) {
      throw std::logic_error(
          "projected issue-1478 final Grass target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-EV-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          "Discarded route-secondary Team Yell's Cheer and searched one Grass Energy for the current-turn manual attachment.");
    return true;
  }

  bool issue_3170_crispin_energy_axis_without_connector_domination() const {
    // MatchupFlexJit can preserve the already-held Crispin for the following Energy
    // turn when the completed #1038 K1 predicate proves that held Gladion owns the
    // current Supporter action. In that connector-dominated state, the generalized
    // #1795 Crispin -> Steven -> Vessel packet yields to the existing route.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // One Supporter per turn: https://www.pokemon.com/us/pokemon-tcg/rules
    // Connector and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Completed connector contract: https://github.com/FlareZ123/pokemon-sims/issues/1038
    // Refined semantic-admission bug: https://github.com/FlareZ123/pokemon-sims/issues/3170
    if (scenario_.dci == DciProfile::MatchupFlexJit &&
        wonder_tag_duplicate_held_crispin_has_no_marginal_route()) {
      return false;
    }
    return crispin_can_advance_energy_axis();
  }

#define crispin_can_advance_energy_axis issue_3170_crispin_energy_axis_without_connector_domination
