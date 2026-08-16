  bool issue_1516_quick_ball_tapu_duplicate_crispin_is_redundant(
      const bool permit_payload) const {
    // Full Prize inspection and a resolved deck search establish the same K1
    // knowledge, so this route depends on known zones rather than provenance:
    // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Correction precedent: https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1922
    if (!prizes_known() ||
        scenario_.dci != DciProfile::NoDiscardControl ||
        hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::Crispin) == 0 ||
        hand_count(Card::Gladion) == 0 ||
        !need_energy()) {
      return false;
    }

    constexpr std::array<Card, 5> kAcceptedPayloads{
        Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
        Card::GoodraVstar, Card::Appletun};
    const bool known_prized_payload = std::any_of(
        kAcceptedPayloads.begin(), kAcceptedPayloads.end(),
        [this](const Card card) {
          return prize_count_after_reveal(card) > 0;
        });
    if (!known_prized_payload) return false;

    std::mt19937_64 projected_rng = rng_;
    Engine projected(scenario_, recipe_, projected_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    const int tapu_before = projected.hand_count(Card::TapuLeleGX);
    if (!projected.play_quick_ball_issue1516_original(permit_payload) ||
        projected.hand_count(Card::TapuLeleGX) <= tapu_before) {
      return false;
    }

    const int crispin_before = projected.hand_count(Card::Crispin);
    const bool benched_tapu = projected.bench_tapu_if_useful();
    const bool selected_duplicate_crispin = benched_tapu &&
        projected.hand_count(Card::Crispin) > crispin_before;

    // Quick Ball pays one discard and searches one Basic Pokemon. This K1-only
    // projection executes the final target selector and Wonder Tag selector before
    // preserving the physical route. A duplicate held Crispin cannot advance the
    // current Supporter action while held Gladion covers the known payload Prize:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // One Supporter per turn and Item cost procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and resource-preserving route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1516
    return selected_duplicate_crispin;
  }

  bool issue_2164_quick_ball_latias_finish_available() const {
    if ((scenario_.dci != DciProfile::StrictJit &&
         scenario_.dci != DciProfile::MatchupFlexJit) ||
        !scenario_.going_first || state_.turn != 4 ||
        state_.turn > scenario_.max_turn || !prizes_known() ||
        item_locked() || rule_box_abilities_locked() ||
        state_.manual_energy_used || state_.retreat_used ||
        !state_.active || state_.active->card != Card::DialgaGX ||
        bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) ||
        in_play(Card::LatiasEx) || hand_count(Card::LatiasEx) > 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::MegaDragonite) == 0 || payload_ready()) {
      return false;
    }

    const bool prior_turn_regidrago_one_basic_from_apex = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          if (pokemon.card != Card::RegidragoV ||
              pokemon.entered_turn >= state_.turn) return false;
          for (const Card basic : {Card::Grass, Card::Fire}) {
            if (hand_count(basic) == 0) continue;
            Pokemon projected = pokemon;
            if (attach_energy_card(projected, basic) &&
                pays_apex_energy_cost(projected)) {
              // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
              // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
              // Energy attachment rule: https://www.pokemon.com/us/pokemon-tcg/rules
              // DDE semantic contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
              // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2438
              return true;
            }
          }
          return false;
        });
    if (!prior_turn_regidrago_one_basic_from_apex) return false;

    // This predicate checks the observed pre-evolution GG Regidrago V directly.
    // need_active_vstar() is false until a VSTAR is physically in play, so that
    // generic post-evolution predicate cannot gate this pre-evolution completion.
    // The held Fire and VSTAR complete the attacker after Quick Ball creates the
    // current-turn payload outlet and searches Latias ex for Skyliner promotion.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Dialga-GX is Basic: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago V / VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, Bench, Ability, attachment, evolution, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, DCI, current-turn JIT, connector contention, and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2164
    return true;
  }

  bool play_issue_2164_quick_ball_latias_finish() {
    if (!issue_2164_quick_ball_latias_finish_available()) return false;

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(Card::MegaDragonite,
                           "Quick Ball issue-2164 current-turn payload cost",
                           "R-QB-01; P-JIT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("Issue-2164 Mega Dragonite ex cost disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-2164 Latias route");
    if (!move_deck_to_hand(Card::LatiasEx)) {
      throw std::logic_error("Issue-2164 Latias ex target disappeared");
    }
    shuffle(state_.deck);
    trace("QUICK BALL", "R-QB-01; R-LATIAS-01; P-JIT-01; P-DCI-01; P-COMPRESS-01",
          "Discarded Mega Dragonite ex and searched Latias ex for the deterministic T4 promotion route.");
    if (!bench_from_hand(Card::LatiasEx, false)) {
      throw std::logic_error("Issue-2164 Latias ex Bench action failed");
    }
    return true;
  }

  bool play_quick_ball(const bool permit_payload) {
    if (play_issue_2164_quick_ball_latias_finish()) return true;
    if (issue_1516_quick_ball_tapu_duplicate_crispin_is_redundant(
            permit_payload)) {
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1516-qb-tapu-duplicate-crispin", state_.turn,
            "HOLD QUICK BALL",
            "R-QB-01; R-TAPU-01; R-CRISPIN-01; R-GLADION-01; R-GAME-SUPPORTER; P-CONNECTOR-01; P-KNOWLEDGE-01",
            "Preserved Quick Ball, its discard, Tapu Lele-GX, Wonder Tag, and the Bench slot because held Gladion covers the known no-control payload Prize and held Crispin covers the later Energy turn.");
      }
      return false;
    }
    return play_quick_ball_issue1516_original(permit_payload);
  }
