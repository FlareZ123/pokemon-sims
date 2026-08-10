  std::optional<Card> issue_1878_vessel_cost() const {
    // Crispin must consume this turn's Supporter action, so a held Supporter that
    // cannot advance another axis before the immediate ready state is route-replaced
    // and may pay Earthen Vessel without spending a Dragon payload prematurely.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // One-Supporter-per-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Dynamic DCI, Supporter contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1878
    for (const Card card : {Card::StevensResolve, Card::Channeler,
                            Card::ProfessorTuro, Card::ErikasInvitation,
                            Card::Klara, Card::Lusamine,
                            Card::RoseannesBackup, Card::TeamYellsCheer,
                            Card::Gladion, Card::Arven}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  std::optional<Card> issue_1878_held_payload() const {
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_1878_vessel_quick_ball_tapu_crispin_route_available() const {
    const bool held_manual_energy_finishes = [this]() {
      if (state_.manual_energy_used || !state_.active) return false;
      const auto energy = preferred_manual_energy_from_hand(*state_.active);
      if (!energy) return false;
      Pokemon projected = *state_.active;
      return attach_energy_card(projected, *energy) &&
             pays_apex_energy_cost(projected);
    }();

    // A resolved deck search or full Prize inspection establishes the same public
    // K1 state. The exact deck composition is then known by direct inspection or
    // fixed-list deduction, so this route must use prizes_known() rather than a
    // deck-search-only provenance flag.
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Hisuian Heavy Ball full-Prize inspection: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Double Dragon Energy can make the old raw Fire-only shape already complete,
    // or make one direct manual attachment dominate the entire Vessel/Tapu/Crispin
    // package. Reject both with semantic Apex payment checks before using the old
    // Basic-Energy route shape: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // StrictJit and MatchupFlexJit share same-ready-turn payload timing; use the
    // semantic timing predicate instead of overfitting this legal route to one enum.
    // JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Profile-gate bug: https://github.com/FlareZ123/pokemon-sims/issues/2756
    // Confirmed route/DDE bugs: https://github.com/FlareZ123/pokemon-sims/issues/1878 https://github.com/FlareZ123/pokemon-sims/issues/2368
    if (!strict_payload_timing() || state_.turn != 3 ||
        !prizes_known() || item_locked() || !supporter_allowed() ||
        state_.manual_energy_used || !deck_search_available() || !state_.active ||
        state_.active->card != Card::RegidragoVstar ||
        !need_energy() || held_manual_energy_finishes ||
        state_.active->grass != 0 || state_.active->fire != 1 ||
        !need_payload() || hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::QuickBall) == 0 || hand_count(Card::Crispin) > 0 ||
        bench_space() == 0 || in_play(Card::TapuLeleGX) ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        deck_count_after_search_started(Card::TapuLeleGX) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::Grass) < 3 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }

    return issue_1878_vessel_cost().has_value() &&
        issue_1878_held_payload().has_value();
  }

  bool complete_issue_1878_vessel_quick_ball_tapu_crispin_route() {
    if (!issue_1878_vessel_quick_ball_tapu_crispin_route_available()) {
      return false;
    }

    const std::optional<Card> vessel_cost = issue_1878_vessel_cost();
    const std::optional<Card> payload = issue_1878_held_payload();
    if (!vessel_cost.has_value() || !payload.has_value()) {
      throw std::logic_error("Issue-1878 projected costs disappeared");
    }

    // Earthen Vessel first spends a Supporter that Crispin replaces on this turn,
    // then searches two Grass Energy. The manual attachment supplies one Grass.
    // Quick Ball next discards the held Dragon as the current-turn JIT payload and
    // searches Tapu Lele-GX. Wonder Tag obtains Crispin, whose printed two-type
    // search attaches the final Grass while Fire remains searchable.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Dragapult ex payload witness: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Official Item-cost, search, Bench, Ability, Supporter, attachment, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI/JIT, Supporter contention, connector composition, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1878 https://github.com/FlareZ123/pokemon-sims/issues/2756
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(*vessel_cost, "Earthen Vessel issue-1878 route cost",
                           "R-EV-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("Issue-1878 Earthen Vessel cost disappeared");
    }
    search_energy_to_hand(2, "R-EV-01; R-GAME-ITEM; P-COMPRESS-01",
                          "Earthen Vessel");
    if (!attach_manual()) {
      throw std::logic_error("Issue-1878 manual Grass attachment disappeared");
    }

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(*payload, "Quick Ball issue-1878 current-turn JIT payload",
                           "R-QB-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1878 Quick Ball payload disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-1878 Tapu-Crispin route");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error("Issue-1878 Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-JIT-01; P-COMPRESS-01",
          "Quick Ball discarded the current-turn Dragon payload and searched Tapu Lele-GX for Crispin.");

    if (!bench_from_hand(Card::TapuLeleGX, false)) {
      throw std::logic_error("Issue-1878 Tapu Lele-GX Bench action disappeared");
    }
    record_deck_search_knowledge("Wonder Tag issue-1878 Crispin route");
    if (!move_deck_to_hand(Card::Crispin)) {
      throw std::logic_error("Issue-1878 Crispin target disappeared");
    }
    shuffle(state_.deck);
    trace("WONDER TAG", "R-TAPU-01; R-CRISPIN-01; P-COMPRESS-01",
          "Searched and revealed Crispin for the final Grass attachment.");
    if (!play_crispin()) {
      throw std::logic_error("Issue-1878 Crispin completion disappeared");
    }

    return active_is_vstar() && !need_energy() && !need_payload();
  }
