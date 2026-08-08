#include "part_issue_1879_treasure_quick_ball_tapu_crispin.inc"

  static long double issue_1721_combination(const int n, const int k) {
    if (k < 0 || n < k) return 0.0L;
    long double result = 1.0L;
    for (int i = 1; i <= k; ++i) {
      result *= static_cast<long double>(n - k + i);
      result /= static_cast<long double>(i);
    }
    return result;
  }

  long double issue_1721_t2_payload_probability_after_route() const {
    // Project only identities known through K1. The T2 draw succeeds directly on
    // a current-turn payload outlet. A Dragon payload or irrelevant draw may then
    // succeed through Legacy Star's seven-card discard and two-card recovery:
    // Regidrago VSTAR / Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Observable K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Future-card oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Corrected conditional proof: https://github.com/FlareZ123/pokemon-sims/issues/1721
    constexpr int searched_cards = 7;  // Vessel, Tool, Oricorio, G, F, G, G.
    const int projected_deck = static_cast<int>(state_.deck.size()) - searched_cards;
    const int direct_outlets =
        deck_count_after_search_started(Card::QuickBall) +
        deck_count_after_search_started(Card::MysteriousTreasure) +
        deck_count_after_search_started(Card::BrilliantBlender) +
        deck_count_after_search_started(Card::ProfessorBurnet) +
        deck_count_after_search_started(Card::Serena);
    const int payloads =
        deck_count_after_search_started(Card::Dragapult) +
        deck_count_after_search_started(Card::MegaDragonite) +
        deck_count_after_search_started(Card::DialgaGX);
    const int irrelevant = projected_deck - direct_outlets - payloads;
    if (projected_deck <= 7 || direct_outlets < 0 || payloads < 0 || irrelevant < 0) {
      return 0.0L;
    }

    const long double denominator =
        issue_1721_combination(projected_deck - 1, 7);
    if (denominator == 0.0L) return 0.0L;
    const long double direct =
        static_cast<long double>(direct_outlets) / projected_deck;
    const long double after_payload = 1.0L -
        issue_1721_combination(irrelevant, 7) / denominator;
    const long double after_irrelevant = 1.0L -
        issue_1721_combination(irrelevant - 1, 7) / denominator;
    return direct +
        static_cast<long double>(payloads) / projected_deck * after_payload +
        static_cast<long double>(irrelevant) / projected_deck * after_irrelevant;
  }

  bool issue_1721_arven_powerglass_route_available() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2096

    // This narrow route is chosen only from the public hand, board, lock state,
    // and a completed K1 deck inspection. Field Blower has zero modeled setup
    // value because no Stadium or Tool target exists in the goldfish state:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, Tool, Ability, Bench, attachment, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1721
    if (scenario_.dci != DciProfile::StrictJit || scenario_.going_first ||
        state_.turn != 1 || !prizes_known() || !supporter_allowed() ||
        state_.manual_energy_used || item_locked() ||
        rule_box_abilities_locked() || state_.stadium != Stadium::None ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->tool != Tool::None || bench_space() <= 0 ||
        state_.vstar_power_used || in_play(Card::Oricorio) ||
        hand_count(Card::Arven) == 0 || hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::QuickBall) == 0 || hand_count(Card::FieldBlower) == 0 ||
        hand_count(Card::GoodraVstar) == 0 ||
        hand_count(Card::MegaDragonite) == 0 ||
        deck_count_after_search_started(Card::EarthenVessel) == 0 ||
        deck_count_after_search_started(Card::Powerglass) == 0 ||
        deck_count_after_search_started(Card::Oricorio) == 0 ||
        deck_count_after_search_started(Card::Grass) < 3 ||
        deck_count_after_search_started(Card::Fire) < 1) {
      return false;
    }
    constexpr long double minimum_probability =
        700597.0L / 740962.0L;  // 94.55235221239415%.
    return issue_1721_t2_payload_probability_after_route() + 1.0e-15L >=
        minimum_probability;
  }

  bool play_issue_1721_arven_powerglass_route() {
    if (!issue_1721_arven_powerglass_route_available()) return false;

    // Arven searches Earthen Vessel and Powerglass. Vessel spends setup-dead
    // Field Blower for Grass plus Fire. Quick Ball then spends that Fire for
    // Oricorio, Vital Dance takes two Grass, the manual attachment supplies T1
    // Grass, and Powerglass restores Fire at end of turn. Celestial Roar is held
    // because the deterministic GF board preserves every unresolved T2 outlet:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core action order and end-of-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, DCI, and resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1721
    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven issue-1721 route");

    if (!move_deck_to_hand(Card::EarthenVessel) ||
        !move_deck_to_hand(Card::Powerglass)) {
      throw std::logic_error("issue-1721 Arven targets disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-COMPRESS-01",
          "Arven searched Earthen Vessel and Powerglass for the K1 GF compression route.");

    if (!remove_one(state_.hand, Card::EarthenVessel)) {
      throw std::logic_error("issue-1721 Earthen Vessel disappeared");
    }
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(Card::FieldBlower,
                           "setup-dead K1 Earthen Vessel cost",
                           "R-EV-01; P-DCI-01")) {
      throw std::logic_error("issue-1721 Field Blower cost disappeared");
    }
    if (!move_deck_to_hand(Card::Grass) || !move_deck_to_hand(Card::Fire)) {
      throw std::logic_error("issue-1721 Vessel Energy targets disappeared");
    }
    shuffle(state_.deck);
    trace("EARTHEN VESSEL", "R-EV-01; R-GAME-ITEM; P-DCI-01",
          "Discarded setup-dead Field Blower; searched Grass Energy and Fire Energy.");

    remove_one(state_.hand, Card::Powerglass);
    state_.active->tool = Tool::Powerglass;
    trace("ATTACH TOOL", "R-POWERGLASS-01; R-GAME-TOOL",
          "Attached Powerglass to the Active Regidrago V.");

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(Card::Fire,
                           "Powerglass-recouped Quick Ball cost",
                           "R-QB-01; R-POWERGLASS-01; P-DCI-01")) {
      throw std::logic_error("issue-1721 Fire cost disappeared");
    }
    if (!move_deck_to_hand(Card::Oricorio)) {
      throw std::logic_error("issue-1721 Oricorio target disappeared");
    }
    shuffle(state_.deck);
    trace("QUICK BALL", "R-QB-01; R-GAME-ITEM; P-CONNECTOR-01",
          "Discarded Fire Energy; searched Oricorio for Vital Dance.");

    if (!bench_from_hand(Card::Oricorio, false)) {
      throw std::logic_error("issue-1721 Oricorio could not be Benched");
    }
    for (int copy = 0; copy < 2; ++copy) {
      if (!move_deck_to_hand(Card::Grass)) {
        throw std::logic_error("issue-1721 Vital Dance Grass target disappeared");
      }
    }
    shuffle(state_.deck);
    trace("VITAL DANCE", "R-ORICORIO-01; P-COMPRESS-01",
          "Searched two Grass Energy for the T1 and T2 manual attachments.");

    if (!attach_manual()) {
      throw std::logic_error("issue-1721 T1 Grass attachment failed");
    }
    // All T1 setup actions are complete. End the turn voluntarily so Celestial
    // Roar cannot churn the unresolved VSTAR and payload-out window. Powerglass
    // still resolves during the ordinary end-of-turn phase:
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // Official attack and end-of-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1721
    state_.turn_ended = true;
    trace("HOLD ATTACK", "R-RV-01; R-POWERGLASS-01; P-CONNECTOR-01",
          "Held Celestial Roar because the deterministic Powerglass GF route preserves the T2 VSTAR and payload-out window.");
    return true;
  }

  bool play_quick_ball(const bool permit_payload) {
    // Compose Treasure, Quick Ball, Wonder Tag, and Crispin before generic Quick
    // Ball handling can spend the connector or choose a weaker independent cost.
    // https://github.com/FlareZ123/pokemon-sims/issues/1879
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (complete_issue_1879_treasure_quick_ball_tapu_crispin_route()) return true;
    if (play_issue_1721_arven_powerglass_route()) return true;
    const bool issue_1595_hold_quick_ball_for_turo =
        scenario_.dci == DciProfile::MatchupFlexJit && state_.turn == 4 &&
        state_.active && state_.active->card == Card::TapuLeleGX &&
        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [this](const Pokemon& pokemon) {
                      // Held Turo directly promotes any semantically powered VSTAR.
                      // DDE + either Basic Energy already pays Apex Dragon's GGF:
                      // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
                      // https://api.pokemontcg.io/v2/cards/swsh12-136
                      // https://github.com/FlareZ123/pokemon-sims/issues/2418
                      return pokemon.card == Card::RegidragoVstar &&
                             pays_apex_energy_cost(pokemon);
                    }) &&
        hand_count(Card::ProfessorTuro) > 0 &&
        (hand_count(Card::BrilliantBlender) > 0 || payload_ready()) &&
        hand_count(Card::QuickBall) > 0 && !state_.supporter_used;
    if (issue_1595_hold_quick_ball_for_turo) {
      // Quick Ball is redundant because Professor Turo already solves the Active
      // axis without a discard cost. Preserve Turo, let Blender establish the
      // current-turn payload, then return Tapu Lele-GX and promote Regidrago VSTAR:
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
      // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
      // Core Item, Supporter, return, and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1595
      trace("HOLD QUICK BALL", "R-QB-01; P-DCI-01; P-COMPRESS-01; P-CONNECTOR-01",
            "Held redundant Quick Ball because Professor Turo already completes the post-Blender Active route.");
      return false;
    }
    return play_quick_ball_issue1595_original(permit_payload);
  }
