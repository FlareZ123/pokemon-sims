  bool issue_1821_steven_latias_grass_route_available() const {
    const bool tapu_benched = std::any_of(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::TapuLeleGX;
        });

    // Wonder Tag has established K1. The held VSTAR, Earthen Vessel, and Dragon
    // already cover the evolution and same-turn JIT payload axes. Steven must spend
    // its three unrestricted targets on Regidrago V, Latias ex, and Grass Energy.
    // The held Grass first pays Oricorio's printed one-Energy Retreat Cost so the
    // already Benched Tapu Lele-GX becomes Active before Steven ends the turn:
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official attachment, retreat, Supporter, search, evolution, Ability, and Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, same-turn JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1821
    // Cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2770
    // Full Prize inspection and a resolved deck search establish the same K1
    // knowledge for this deterministic Steven target package:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
    // https://github.com/FlareZ123/pokemon-sims/issues/1928
    return prizes_known() && state_.turn == 1 &&
        !scenario_.going_first && strict_payload_timing() &&
        scenario_.locks == LockMode::None && scenario_.max_turn >= 3 &&
        supporter_allowed() && !item_locked() && !state_.manual_energy_used &&
        !state_.retreat_used && state_.active &&
        state_.active->card == Card::Oricorio && state_.active->grass == 0 &&
        state_.active->fire == 0 && tapu_benched && bench_space() >= 2 &&
        hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::EarthenVessel) > 0 && hand_count(Card::Grass) > 0 &&
        hand_count(Card::Crispin) > 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        deck_count_after_search_started(Card::RegidragoV) > 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::Fire) > 0;
  }

  bool issue_2153_steven_latias_blender_route_available() const {
    const bool tapu_benched = std::any_of(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::TapuLeleGX;
        });

    // Wonder Tag establishes K1 before Steven resolves. In the exact seed-408
    // state, the held VSTAR, Vessel, Crispin, and Forest Seal Stone already cover
    // evolution and GGF. The scarce remaining axes are Regidrago V, Skyliner, and
    // the current-turn JIT payload outlet, so Steven's three targets must be
    // Regidrago V, Latias ex, and Brilliant Blender:
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Official Supporter, search, attachment, evolution, Ability, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, same-turn JIT, lock, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2153
    // Cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2773
    return prizes_known() && state_.turn == 1 && !scenario_.going_first &&
        strict_payload_timing() &&
        scenario_.locks == LockMode::None && scenario_.max_turn >= 3 &&
        supporter_allowed() && state_.active &&
        state_.active->card == Card::Oricorio && tapu_benched &&
        bench_space() >= 2 && !in_play(Card::RegidragoV) &&
        !in_play(Card::RegidragoVstar) && !in_play(Card::LatiasEx) &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        hand_count(Card::StevensResolve) > 0 &&
        hand_count(Card::RegidragoV) == 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::LatiasEx) == 0 &&
        hand_count(Card::BrilliantBlender) == 0 &&
        hand_count(Card::EarthenVessel) > 0 &&
        hand_count(Card::Crispin) > 0 &&
        hand_count(Card::ForestSealStone) > 0 &&
        deck_count_after_search_started(Card::RegidragoV) > 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        deck_count_after_search_started(Card::BrilliantBlender) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 3 &&
        deck_count_after_search_started(Card::Fire) >= 3 &&
        payload_might_be_in_deck();
  }

  bool play_issue_2153_steven_latias_blender_route() {
    if (!issue_2153_steven_latias_blender_route_available()) return false;

    if (!remove_one(state_.hand, Card::StevensResolve)) {
      throw std::logic_error("Issue-2153 Steven's Resolve disappeared");
    }
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-2153 route");

    if (!move_deck_to_hand(Card::RegidragoV) ||
        !move_deck_to_hand(Card::LatiasEx) ||
        !move_deck_to_hand(Card::BrilliantBlender)) {
      throw std::logic_error("Issue-2153 Steven targets disappeared");
    }
    shuffle(state_.deck);
    state_.turn_ended = true;

    // Steven legally takes the exact three K1-visible connectors and ends T1.
    // T2 benches Regidrago V and Latias ex, uses Star Alchemy plus Earthen Vessel
    // and Crispin for GG with Fire held, then T3 evolves, attaches Fire, retreats
    // Oricorio for free through Skyliner, and uses Blender for same-turn JIT:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed route: https://github.com/FlareZ123/pokemon-sims/issues/2153
    // Cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2773
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the deterministic T3 Latias route: Regidrago V, Latias ex, Brilliant Blender. Turn ended.");
    return true;
  }

  bool play_issue_1821_steven_latias_grass_route() {
    if (play_issue_2153_steven_latias_blender_route()) return true;
    if (!issue_1821_steven_latias_grass_route_available()) return false;

    if (!remove_one(state_.hand, Card::Grass)) {
      throw std::logic_error("Issue-1821 retreat Grass disappeared");
    }
    ++state_.active->grass;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; R-STEVEN-01; P-COMPRESS-01",
          "Grass Energy manually to Active Oricorio for its Retreat Cost before Steven's Resolve.");

    const auto tapu = std::find_if(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::TapuLeleGX;
        });
    if (tapu == state_.bench.end() || state_.active->grass < 1) {
      throw std::logic_error("Issue-1821 Tapu retreat target disappeared");
    }
    --state_.active->grass;
    state_.discard.push_back(Card::Grass);
    std::swap(*state_.active, *tapu);
    state_.retreat_used = true;
    trace("RETREAT", "R-GAME-RETREAT; R-STEVEN-01; P-COMPRESS-01",
          "Paid Oricorio's one-Energy Retreat Cost and promoted Tapu Lele-GX before Steven's Resolve.");

    if (!remove_one(state_.hand, Card::StevensResolve)) {
      throw std::logic_error("Issue-1821 Steven's Resolve disappeared");
    }
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-1821 route");
    if (!move_deck_to_hand(Card::RegidragoV) ||
        !move_deck_to_hand(Card::LatiasEx) ||
        !move_deck_to_hand(Card::Grass)) {
      throw std::logic_error("Issue-1821 Steven targets disappeared");
    }
    shuffle(state_.deck);
    state_.turn_ended = true;
    trace("PLAY SUPPORTER",
          "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Searched the complete T3 route: Regidrago V, Latias ex, Grass Energy. Turn ended.");
    return true;
  }

// Keep the composed Steven policy available to the final issue-2622 overlay.
#define play_steven play_steven_issue2622_original
