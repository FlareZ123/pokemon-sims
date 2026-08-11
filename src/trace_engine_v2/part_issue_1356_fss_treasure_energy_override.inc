#include "part_issue_1411_treasure_tapu_steven_override.inc"

  int issue_1417_treasure_hold_turn_ = 0;

  bool issue_1549_mysterious_treasure_target_available() const {
    // At K0, might_be_unseen() uses only the fixed recipe and public zones. After
    // a legal deck or Prize inspection, it uses the exact remaining deck instead:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // K0 and K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Hidden-information specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1549
    return std::any_of(
        recipe_.begin(), recipe_.end(), [this](const auto& entry) {
          return entry.second > 0 && is_dragon_or_psychic(entry.first) &&
              might_be_unseen(entry.first);
        });
  }

  // DDE makes the two Basic types alternative final attachments; reserve the
  // actual semantic completion before choosing a Treasure cost.
  // https://github.com/FlareZ123/pokemon-sims/issues/2427
  std::optional<Card> issue_1356_held_completion_basic() const {
    if (!state_.active) return std::nullopt;
    return completing_basic_energy_for(*state_.active, [this](const Card card) {
      return hand_count(card) > 0;
    });
  }

  std::optional<Card> issue_1356_surplus_basic_after_completion(
      const Card completion) const {
    for (const Card basic : {Card::Grass, Card::Fire}) {
      const int reserved = basic == completion ? 1 : 0;
      if (hand_count(basic) > reserved) return basic;
    }
    return std::nullopt;
  }

  bool issue_1356_treasure_vstar_energy_split_available() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2073

    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        scenario_.max_turn < 3 || !prizes_known() || !state_.manual_energy_used ||
        !state_.vstar_power_used || item_locked() || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::MysteriousTreasure) < 2 ||
        hand_count(Card::RegidragoVstar) > 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const auto completion = issue_1356_held_completion_basic();
    const auto surplus = completion.has_value()
        ? issue_1356_surplus_basic_after_completion(*completion)
        : std::nullopt;

    // Star Alchemy has supplied the missing next-turn Energy. The opposite held
    // Energy is now a legal dynamic-DCI cost for Mysterious Treasure, which
    // searches Regidrago VSTAR and preserves a second Treasure plus the held
    // Dragon for the T3 strict-JIT payload:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, search, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1356
    return completion.has_value() && surplus.has_value();
  }

  bool issue_1356_complete_next_turn_route_holds_quick_ball() const {
    // A resolved deck search and complete Prize inspection establish the same fixed-list K1.
    // Hisuian Heavy Ball / complete Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Official Prize, search, Ability, Trainer, attachment, evolution, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2073

    if (scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 2 ||
        !prizes_known() || !state_.manual_energy_used || !state_.vstar_power_used ||
        item_locked() || !state_.active ||
        state_.active->entered_turn >= state_.turn ||
        (state_.active->card != Card::RegidragoV &&
         state_.active->card != Card::RegidragoVstar) ||
        state_.active->tool != Tool::ForestSealStone ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    const auto completion = issue_1356_held_completion_basic();
    const auto surplus = completion.has_value()
        ? issue_1356_surplus_basic_after_completion(*completion)
        : std::nullopt;
    const bool vstar_axis_complete = active_is_vstar() ||
        hand_count(Card::RegidragoVstar) > 0;
    const bool payable_treasure_bridge =
        hand_count(Card::MysteriousTreasure) >= 2 &&
        hand_count(Card::RegidragoVstar) == 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        surplus.has_value();

    // Quick Ball into Oricorio spends an Item, a second discard, a support
    // Pokémon, and a Bench slot after the Energy, VSTAR, and payload axes already
    // have complete public routes. It cannot improve the T3 deadline:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Bench, Ability, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Resource-preservation priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1335 https://github.com/FlareZ123/pokemon-sims/issues/1356
    return completion.has_value() &&
        (vstar_axis_complete || payable_treasure_bridge);
  }

  bool play_quick_ball(const bool permit_payload) {
    if (issue_1356_complete_next_turn_route_holds_quick_ball()) return false;
    return play_quick_ball_issue1356_original(permit_payload);
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (issue_1817_play_prelock_treasure_tapu_route()) return true;
    if (issue_1411_play_treasure_tapu_steven_route()) return true;
    if (issue_1417_treasure_hold_turn_ == state_.turn) {
      // The recovered Mysterious Treasure is a next-turn connector. Playing it on
      // the Legacy Star turn would discard the held Dragon before GGF is complete,
      // violating the repository strict-JIT timing contract:
      // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Official Item and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1417
      return false;
    }
    if (!issue_1356_treasure_vstar_energy_split_available()) {
      return play_mysterious_treasure_issue1356_original(permit_payload);
    }

    const auto completion = issue_1356_held_completion_basic();
    const auto surplus_energy = completion.has_value()
        ? issue_1356_surplus_basic_after_completion(*completion)
        : std::nullopt;
    if (!completion.has_value() || !surplus_energy.has_value()) {
      throw std::logic_error("issue-1356 semantic completion/surplus Energy disappeared");
    }
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(*surplus_energy, "Mysterious Treasure split-route cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-1356 surplus Energy cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");
    if (!move_deck_to_hand(Card::RegidragoVstar)) {
      throw std::logic_error("projected issue-1356 Regidrago VSTAR target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          "Discarded the surplus off-type Energy and searched Regidrago VSTAR; preserved the second Treasure for the T3 Dragon payload.");
    return true;
  }

  bool issue_1417_future_treasure_route_available_before_legacy() const {
    return scenario_.dci == DciProfile::StrictJit && !item_locked() &&
        state_.turn < scenario_.max_turn && active_is_vstar() &&
        state_.manual_energy_used && need_payload() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
        count_of(state_.discard, Card::MysteriousTreasure) > 0 &&
        issue_1549_mysterious_treasure_target_available() && state_.active &&
        completing_basic_energy_for(*state_.active, [this](const Card card) {
          return hand_count(card) > 0;
        }).has_value();
  }

  bool use_legacy_star_issue_1417() {
    const bool recover_future_treasure =
        issue_1417_future_treasure_route_available_before_legacy();
    const std::size_t hand_before = state_.hand.size();
    const bool used = use_legacy_star();
    if (!used || !recover_future_treasure ||
        state_.hand.size() >= hand_before + 2U ||
        count_of(state_.discard, Card::MysteriousTreasure) == 0 ||
        !issue_1549_mysterious_treasure_target_available()) {
      return used;
    }

    // Legacy Star permits up to two recovered cards. When its existing policy has
    // used fewer than two slots, recover Mysterious Treasure as the deterministic
    // next-turn payload connector. The held Basic Energy completes GGF by the next
    // manual attachment, and Treasure can then discard the held Dragon on the same
    // strict-JIT ready turn:
    // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Official attachment, Item, search, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest deterministic route and dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1417
    if (!recover_discard_to_hand(Card::MysteriousTreasure)) {
      throw std::logic_error("Legacy Star future-turn Treasure target disappeared");
    }
    issue_1417_treasure_hold_turn_ = state_.turn;
    trace("LEGACY STAR ROUTE", "R-VSTAR-01; R-MT-01; P-DCI-01; P-COMPRESS-01",
          "Recovered Mysterious Treasure for the next-turn strict-JIT Dragon payload.");
    return true;
  }
