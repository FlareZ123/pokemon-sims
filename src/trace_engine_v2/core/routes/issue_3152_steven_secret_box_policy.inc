  // part_issue_1118_secret_box.inc calls bench_pineco_if_useful immediately after
  // the T1 manual attachment and before its Prize-Supporter selector. Keep the
  // #3152 comparator in this single route owner so the route definition and the
  // macro handoff remain adjacent.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Policy boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_issue_1118_secret_box.inc
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3152
  bool issue_3152_complete_steven_t2_route_available() const {
    const bool registered_pineco_recipe =
        registered_recipe_count(Card::SecretBox) == 1 &&
        registered_recipe_count(Card::Pineco) == 2 &&
        registered_recipe_count(Card::ForretressEx) == 2;

    // Project only the public next-turn clock before asking the simulator's shared
    // Item-lock predicate. This keeps the comparator aligned with every modeled
    // lock variant instead of hard-coding the current LockMode enum:
    // Advanced Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Lock model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3152
    Engine next_turn = *this;
    ++next_turn.state_.turn;
    next_turn.state_.turn_ended = false;
    const bool next_turn_item_locked = next_turn.item_locked();

    // Steven's Resolve may search any three cards and ends the turn. The exact K1
    // route banks VSTAR + Crispin + Mysterious Treasure while preserving held Fire
    // and a held Dragon payload. On T2, Crispin only needs one draw-safe Grass in
    // deck because its printed effect says "up to 2" Basic Energy of different
    // types. The held Fire remains available for the ordinary manual attachment:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR / Apex Dragon GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official turn, Supporter, Item, search, evolution, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and equal-turn resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3152
    if (!registered_pineco_recipe || !secret_box_combo_enabled() ||
        !strict_payload_timing() || state_.turn != 1 || scenario_.max_turn < 2 ||
        state_.turn_ended || !supporter_allowed() || next_turn_item_locked ||
        !prizes_known() || !known_prize(Card::SecretBox) ||
        hand_count(Card::Gladion) == 0 || hand_count(Card::StevensResolve) == 0 ||
        !state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn > state_.turn ||
        state_.active->grass != 1 || state_.active->fire != 0 ||
        hand_count(Card::Fire) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::MysteriousTreasure) == 0 ||
        deck_count_after_search_started(Card::Grass) <= 1 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // Any Regidrago V already in play by the end of T1 can evolve on T2. This
    // includes the opening Active, whose entered_turn predates T1, and a V played
    // during T1. Use that real next-turn evolution rule instead of an entry-turn
    // witness tied to one route shape:
    // Evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Advanced evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3152

    // Steven removes one Psychic/Dragon target from deck when it banks Regidrago
    // VSTAR. Require two more before the mandatory T2 draw, so every possible draw
    // still leaves Mysterious Treasure a legal search target. The held Dragon is
    // DCI 1 on T2 because Treasure's mandatory discard establishes Apex Dragon's
    // current-turn payload while its search supplies the evolution/connector axis:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Hisuian Goodra VSTAR payload witness: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Dynamic DCI/JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // No-future-card oracle: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3152
    const int treasure_targets = static_cast<int>(std::count_if(
        state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic));
    return treasure_targets >= 3;
  }

  bool play_issue_3152_complete_steven_t2_route() {
    if (!issue_3152_complete_steven_t2_route_available()) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    state_.turn_ended = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    std::vector<Card> selected;
    for (const Card card : {Card::RegidragoVstar, Card::Crispin,
                            Card::MysteriousTreasure}) {
      if (move_deck_to_hand(card)) selected.push_back(card);
    }
    if (selected.size() != 3U) {
      throw std::logic_error(
          "Issue 3152 Steven targets disappeared after proven K1 preflight");
    }
    shuffle(state_.deck);
    issue_1420_direct_t2_treasure_route_ = true;

    // This complete route reaches the same earliest T2 readiness while retaining
    // Gladion and the known prized ACE SPEC. It also avoids Secret Box's mandatory
    // three-card discard, so the lower-resource connector dominates at equal speed.
    // The existing #1420 continuation resolves Crispin, held Fire attachment,
    // Regidrago VSTAR evolution, and the same-turn Treasure payload:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced turn procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Equal-turn resource preservation: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed connector-domination bug: https://github.com/FlareZ123/pokemon-sims/issues/3152
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-CRISPIN-01; R-MT-01; P-AXIS-01; P-DCI-01",
          "Steven's Resolve banked Regidrago VSTAR, Crispin, and Mysterious "
          "Treasure for the complete T2 route, preserving prized Secret Box "
          "and Gladion; the turn ended.");
    return true;
  }

  bool bench_pineco_if_useful_issue3152() {
    const bool benched = bench_pineco_if_useful();
    if (play_issue_3152_complete_steven_t2_route()) return true;
    return benched;
  }

  #define bench_pineco_if_useful bench_pineco_if_useful_issue3152
