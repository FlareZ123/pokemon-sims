#define choose_supporter choose_supporter_issue1873_original
  bool issue_1235_t1_quick_ball_connector_state() const {
    if (scenario_.dci != DciProfile::StrictJit || state_.turn != 1 ||
        scenario_.going_first || scenario_.locks != LockMode::None ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->grass != 0 || state_.active->fire != 0 ||
        bench_space() < 2 || hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::RegidragoV) == 0 ||
        hand_count(Card::TapuLeleGX) > 0 || in_play(Card::TapuLeleGX) ||
        hand_count(Card::Oricorio) > 0 || in_play(Card::Oricorio) ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }
    const bool prior_turn_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn;
        });
    const auto cost = choose_discard(false, true, true, Card::QuickBall);
    return prior_turn_benched_regi && cost == Card::RegidragoV &&
        might_be_unseen(Card::Oricorio) && might_be_unseen(Card::TapuLeleGX);
  }

  bool issue_1235_oricorio_route_available_after_quick_ball_search() const {
    if (!deck_seen_ || hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }
    // Vital Dance must leave one Grass and one Fire for the following turn's
    // Crispin search. Mysterious Treasure then discards the held Dragon payload,
    // searches Tapu Lele-GX, and Wonder Tag obtains Crispin. This comparison is
    // made only after Quick Ball legally exposes the deck, so it does not read
    // future draws or unrevealed Prize identities:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Ability, Supporter, Bench, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1235
    return deck_count_after_search_started(Card::Oricorio) > 0 &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::Fire) >= 2;
  }

  bool play_quick_ball(const bool permit_payload) {
    if (!issue_1235_t1_quick_ball_connector_state()) {
      return play_quick_ball_issue1235_original(permit_payload);
    }

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    discard_from_hand(Card::RegidragoV, "Quick Ball cost", "R-QB-01; P-DCI-01");
    record_deck_search_knowledge("Quick Ball");

    Card selected = Card::TapuLeleGX;
    if (issue_1235_oricorio_route_available_after_quick_ball_search()) {
      selected = Card::Oricorio;
    } else if (deck_count_after_search_started(Card::TapuLeleGX) == 0) {
      selected = fallback_quick_ball_target_after_search_started();
    }
    const bool found = move_deck_to_hand(selected);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-COMPRESS-01",
          found ? "Discarded a redundant Regidrago V and searched the fastest K1 Basic connector: " +
                      std::string(name(selected)) + "."
                : "Discarded a redundant Regidrago V; no Basic Pokemon remained after search.");
    return true;
  }

  bool issue_1235_t2_treasure_tapu_context_available() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2070

    if (scenario_.dci != DciProfile::StrictJit || state_.turn != 2 ||
        scenario_.locks != LockMode::None || !prizes_known() ||
        !supporter_allowed() || state_.supporter_used || bench_space() == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        (hand_count(Card::RegidragoVstar) == 0 && !active_is_vstar()) ||
        hand_count(Card::TapuLeleGX) > 0 || in_play(Card::TapuLeleGX) ||
        hand_count(Card::Crispin) > 0 ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        !state_.active ||
        (state_.active->card != Card::RegidragoV &&
         state_.active->card != Card::RegidragoVstar) ||
        state_.active->entered_turn >= state_.turn ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        deck_count_after_search_started(Card::TapuLeleGX) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }

    // Both different Basic Energy types are present in the inspected deck, so
    // Crispin can legally attach either searched type and put the other into hand.
    // Project that effect attachment first, then the other searched Basic as the
    // manual attachment only when the manual attachment remains unused. DDE plus
    // either Basic already pays Apex after Crispin's effect attachment:
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Crispin: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/sv07/133
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Manual Energy attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2431
    for (const Card crispin_attachment : {Card::Grass, Card::Fire}) {
      Pokemon after_crispin = *state_.active;
      if (!attach_energy_card(after_crispin, crispin_attachment)) continue;
      if (pays_apex_energy_cost(after_crispin)) return true;
      if (state_.manual_energy_used) continue;
      const Card hand_basic = crispin_attachment == Card::Grass
          ? Card::Fire : Card::Grass;
      Pokemon after_manual = after_crispin;
      if (attach_energy_card(after_manual, hand_basic) &&
          pays_apex_energy_cost(after_manual)) {
        return true;
      }
    }
    return false;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (!permit_payload || !issue_1235_t2_treasure_tapu_context_available()) {
      return play_mysterious_treasure_issue1235_original(permit_payload);
    }

    const auto cost = choose_discard(true, true, true, Card::MysteriousTreasure);
    if (!cost || !is_payload(*cost)) return false;
    const Card discarded_payload = *cost;

    // Mysterious Treasure may pay the current-turn Dragon payload because the
    // searched Tapu Lele-GX obtains Crispin, which deterministically completes GGF:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Ability, Supporter, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, earliest-route, and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1235
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(discarded_payload,
                      "Mysterious Treasure cost", "R-MT-01; P-JIT-01");
    record_deck_search_knowledge("Mysterious Treasure");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error(
          "projected issue-1235 Mysterious Treasure Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-JIT-01; P-COMPRESS-01",
          "Discarded the current-turn Dragon payload and searched Tapu Lele-GX for the Crispin T2 route.");
    return true;
  }

  bool issue_1235_t2_treasure_tapu_crispin_completion_available() const {
    if (!issue_1235_t2_treasure_tapu_context_available()) return false;

    // A copied Engine retains the live rng_ reference. Preserve the fixed-seed trial
    // stream while this silent route check resolves searches and projected shuffles:
    // C++ reference-member copy semantics: https://eel.is/c++draft/class.copy.ctor#15
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Official search and shuffle procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Fixed-seed sampling contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#sampling-and-comparison-method
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1691
    const std::mt19937_64 live_rng = rng_;
    Engine projected = *this;
    projected.trace_ = nullptr;
    const bool available = [&projected]() {
      if (!projected.play_mysterious_treasure(true)) return false;
      projected.play_basics_from_hand();
      if (projected.hand_count(Card::Crispin) == 0 ||
          !projected.play_crispin()) return false;
      projected.attach_manual();
      projected.evolve_best_regi();
      // The exact shadow resolves the real Crispin and manual-attachment sequence,
      // then judges the resulting attacker by semantic Apex payment. DDE plus
      // either Basic Energy is a complete GGF payment:
      // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2431
      return projected.active_is_vstar() &&
          projected.pays_apex_energy_cost(*projected.state_.active) &&
          projected.payload_ready();
    }();
    rng_ = live_rng;
    return available;
  }

  void choose_supporter() {
    if (!issue_1235_t2_treasure_tapu_crispin_completion_available()) {
      choose_supporter_issue1235_original();
      return;
    }

    // Preserve the Supporter play for Treasure -> Tapu Lele-GX -> Crispin:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, Ability, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-route and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1235
    if (trace_ != nullptr) {
      trace_->add_policy_once(
          state_, "issue-1235-treasure-tapu-crispin-before-supporter", state_.turn,
          "HOLD SUPPORTER", "R-MT-01; R-TAPU-01; R-CRISPIN-01; P-COMPRESS-01",
          "Retained the Supporter play for the proven Active-Regidrago Treasure to Tapu Lele-GX to Crispin T2 finish.");
    }
  }
#undef choose_supporter
#include "part_issue_1873_blender_crispin_supporter_override.inc"