  std::optional<Card> issue_1875_held_payload_cost() const {
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> issue_1875_blender_quick_ball_cost() const {
    if (hand_count(Card::BrilliantBlender) == 0) return std::nullopt;
    for (const Card card : {Card::Klara, Card::Lusamine,
                            Card::RoseannesBackup, Card::StevensResolve,
                            Card::Gladion, Card::Arven,
                            Card::ProfessorTuro, Card::ErikasInvitation,
                            Card::Channeler, Card::TeamYellsCheer}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> issue_1875_deck_payload() const {
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (deck_count_after_search_started(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_1875_quick_ball_tapu_crispin_route_available() const {
    // A resolved Hisuian Heavy Ball reveals every face-down Prize card, so the
    // fixed-list K1 composition knowledge is equivalent to a resolved deck search.
    // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Official Prize and search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1979
    if (scenario_.dci != DciProfile::StrictJit || state_.turn != 3 ||
        !prizes_known() || item_locked() || !supporter_allowed() ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar ||
        state_.active->grass != 1 || state_.active->fire != 0 ||
        !need_energy() ||  // DDE + Grass already pays GGF; do not spend Fire, Quick Ball, Wonder Tag, or Crispin redundantly: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ ; https://api.pokemontcg.io/v2/cards/swsh12-136 ; https://github.com/FlareZ123/pokemon-sims/issues/2238
        !need_payload() || hand_count(Card::Fire) == 0 ||
        hand_count(Card::QuickBall) == 0 || hand_count(Card::Crispin) > 0 ||
        bench_space() == 0 || in_play(Card::TapuLeleGX) ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        deck_count_after_search_started(Card::TapuLeleGX) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }

    if (issue_1875_held_payload_cost().has_value()) return true;
    return issue_1875_blender_quick_ball_cost().has_value() &&
        issue_1875_deck_payload().has_value();
  }

  bool complete_issue_1875_quick_ball_tapu_crispin_route() {
    if (!issue_1875_quick_ball_tapu_crispin_route_available()) return false;

    const std::optional<Card> held_payload = issue_1875_held_payload_cost();
    const std::optional<Card> future_supporter_cost =
        issue_1875_blender_quick_ball_cost();

    // The manual Fire attachment leaves exactly one Grass missing. Quick Ball
    // then searches Tapu Lele-GX, Wonder Tag searches Crispin, and Crispin's
    // printed two-type search attaches that final Grass during the unused
    // Supporter action. The current-turn payload is supplied either by Quick
    // Ball's held Dragon cost or by Brilliant Blender before Quick Ball.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Ability, Supporter, search, discard, Bench, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // K1, dynamic DCI/JIT, Supporter contention, connector composition, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1875
    if (!remove_one(state_.hand, Card::Fire)) {
      throw std::logic_error("Issue-1875 manual Fire disappeared");
    }
    ++state_.active->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; P-COMPRESS-01",
          "Fire Energy manually to Regidrago VSTAR before the Tapu-Crispin finish.");

    Card quick_ball_cost{};
    if (held_payload.has_value()) {
      quick_ball_cost = *held_payload;
    } else {
      if (!future_supporter_cost.has_value()) {
        throw std::logic_error("Issue-1875 projected Quick Ball cost disappeared");
      }
      const std::optional<Card> payload = issue_1875_deck_payload();
      if (!payload.has_value()) {
        throw std::logic_error("Issue-1875 projected Blender payload disappeared");
      }
      remove_one(state_.hand, Card::BrilliantBlender);
      state_.discard.push_back(Card::BrilliantBlender);
      record_deck_search_knowledge("Brilliant Blender issue-1875 payload route");
      if (!move_deck_to_discard(*payload)) {
        throw std::logic_error("Issue-1875 Brilliant Blender payload disappeared");
      }
      shuffle(state_.deck);
      trace("PLAY ITEM", "R-BLENDER-01; R-GAME-ITEM; P-JIT-01; P-COMPRESS-01",
            "Brilliant Blender discarded the deterministic current-turn Dragon payload.");
      quick_ball_cost = *future_supporter_cost;
    }

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(quick_ball_cost,
                           "Quick Ball issue-1875 Tapu-Crispin route cost",
                           "R-QB-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1875 Quick Ball cost disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-1875 Tapu-Crispin route");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error("Issue-1875 Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-COMPRESS-01",
          "Quick Ball searched Tapu Lele-GX for the immediate Crispin Energy finish.");

    if (!bench_from_hand(Card::TapuLeleGX, false)) {
      throw std::logic_error("Issue-1875 Tapu Lele-GX Bench action disappeared");
    }
    record_deck_search_knowledge("Wonder Tag issue-1875 Crispin route");
    if (!move_deck_to_hand(Card::Crispin)) {
      throw std::logic_error("Issue-1875 Crispin target disappeared");
    }
    shuffle(state_.deck);
    trace("WONDER TAG", "R-TAPU-01; R-CRISPIN-01; P-COMPRESS-01",
          "Searched and revealed Crispin for the final Grass attachment.");
    if (!play_crispin()) {
      throw std::logic_error("Issue-1875 Crispin completion disappeared");
    }
    return active_is_vstar() && !need_energy() && !need_payload();
  }

