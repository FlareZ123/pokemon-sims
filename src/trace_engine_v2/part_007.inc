    if (card == Card::CrobatV) {
      // Dark Asset resolves only when Crobat V is played from hand to the Bench
      // during the turn, and no more than one Dark Asset may be used each turn:
      // https://api.pokemontcg.io/v2/cards/swsh3-104
      // Path to the Peak suppresses Pokémon V Abilities while it is in play:
      // https://api.pokemontcg.io/v2/cards/swsh6-148
      if (!ability_available_for_pokemon(Card::CrobatV)) {
        trace("ABILITY BLOCKED", "R-PATH-01", "Crobat V Dark Asset is suppressed by the Rule Box Ability lock.");
        return;
      }
      if (state_.dark_asset_used) {
        trace("ABILITY LIMITED", "R-CROBAT-01", "Dark Asset was already used this turn.");
        return;
      }
      state_.dark_asset_used = true;
      outcome_.used_dark_asset = true;
      std::uint64_t drawn = 0;
      while (state_.hand.size() < 6U && !state_.deck.empty()) {
        draw_card("Dark Asset");
        ++drawn;
      }
      outcome_.dark_asset_cards_drawn += drawn;
      trace("DARK ASSET", "R-CROBAT-01", "Drew " + std::to_string(drawn) +
            " card(s) until the hand contained six cards.");
      return;
    }
    if (card == Card::TapuLeleGX) {
      if (!ability_available_for_pokemon(Card::TapuLeleGX)) {
        trace("ABILITY BLOCKED", "R-PATH-01", "Tapu Lele-GX Wonder Tag is suppressed by the Rule Box Ability lock.");
        return;
      }
      // Card text source: https://api.pokemontcg.io/v2/cards/sm2-60
      // The search permits inspecting the deck to select a Supporter.
      record_deck_search_knowledge("Wonder Tag");
      const Card target = choose_supporter_after_search_started();
      if (move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("WONDER TAG", "R-TAPU-01", "Searched and revealed " + std::string(name(target)) + ".");
      } else {
        shuffle(state_.deck);
        trace("WONDER TAG", "R-TAPU-01", "No selected Supporter was in deck after legal search.");
      }
      return;
    }
    if (card == Card::Oricorio) {
      // Card text source: https://api.pokemontcg.io/v2/cards/sm2-55
      // Oricorio has no Rule Box; Path-style rule-box suppression does not stop Vital Dance.
      search_energy_to_hand(2, "R-ORICORIO-01", "Vital Dance");
    }
  }

  bool wonder_tag_future_gladion_treasure_route() const {
    if (!strict_payload_timing() || scenario_.locks != LockMode::None ||
        state_.turn + 2 > scenario_.max_turn || !state_.supporter_used ||
        !state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass < 2 || state_.active->fire != 0 ||
        // K1 is complete after either legal deck inspection or full Prize inspection:
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
        // https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
        // https://github.com/FlareZ123/pokemon-sims/issues/1923
        !prizes_known() ||
        hand_count(Card::Fire) == 0 || hand_count(Card::Gladion) > 0 ||
        hand_count(Card::MysteriousTreasure) > 0 ||
        !need_vstar() || !need_energy() || !need_payload() ||
        deck_count_after_search_started(Card::Gladion) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        prize_count_after_reveal(Card::MysteriousTreasure) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // The current Supporter slot was spent to replay Tapu Lele-GX. With GG already
    // attached and Fire held for the next manual attachment, Crispin adds no missing
    // setup component. Wonder Tag should bank the known in-deck Gladion, which takes
    // the known prized Treasure on T4. Treasure then discards the protected payload
    // and searches Regidrago VSTAR on T5:
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
    // Regidrago V and VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter, attachment, search, and evolution procedure:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1166
    return true;
  }

  bool issue_1797_wonder_tag_steven_route_available() const {
    const int deck_grass = deck_count_after_search_started(Card::Grass);
    const int deck_fire = deck_count_after_search_started(Card::Fire);
    const bool deck_payload = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_payload);
    const bool route_cost_paid = count_of(state_.discard, Card::TateLiza) > 0 ||
        count_of(state_.discard, Card::Lusamine) > 0;

    // Wonder Tag resolves on T1 after K1 Quick Ball. It may search Steven's Resolve
    // even though the going-first rule prevents playing that Supporter until T2:
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Official first-turn and Ability search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and complete-route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1797
    return scenario_.dci == DciProfile::StrictJit && scenario_.going_first &&
        scenario_.locks == LockMode::None && state_.turn == 1 &&
        scenario_.max_turn >= 3 && prizes_known() && !state_.supporter_used &&
        state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->grass == 0 && state_.active->fire == 0 &&
        count_of(state_.discard, Card::QuickBall) > 0 && route_cost_paid &&
        in_play(Card::TapuLeleGX) &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        hand_count(Card::RegidragoVstar) > 0 && hand_count(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::StevensResolve) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        deck_grass > 0 && deck_fire > 0 && deck_grass + deck_fire >= 5 &&
        deck_payload;
  }

  bool issue_1796_wonder_tag_steven_route_available() const {
    const bool prior_turn_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pokemon.grass == 0 &&
              pokemon.fire == 0;
        });
    const bool deck_payload = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_payload);

    // Wonder Tag has already established K1. The held Grass attaches on T2, while
    // Steven's Resolve banks Regidrago VSTAR, Crispin, and a Dragon payload. Held
    // Mysterious Treasure then discards that payload on T3 and searches Latias ex,
    // whose Skyliner Ability gives the Basic Oricorio Active free Retreat:
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, search, attachment, evolution, Bench, Ability, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1796
    return scenario_.dci == DciProfile::StrictJit && scenario_.going_first &&
        scenario_.locks == LockMode::None && state_.turn == 2 &&
        scenario_.max_turn >= 3 && prizes_known() && supporter_allowed() &&
        !state_.manual_energy_used && !item_locked() && !state_.retreat_used &&
        state_.active && state_.active->card == Card::Oricorio &&
        prior_turn_regidrago && bench_space() > 0 &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::Grass) > 0 && need_vstar() && need_energy() &&
        need_payload() && deck_payload &&
        deck_count_after_search_started(Card::StevensResolve) > 0 &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
  }

  bool issue_2343_burnet_quick_ball_latias_route_available() const {
    const bool burnet_available = hand_count(Card::ProfessorBurnet) > 0 ||
        deck_count_after_search_started(Card::ProfessorBurnet) > 0;
    const bool deck_payload = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_payload);
    const bool held_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const bool blender_payload_route =
        hand_count(Card::BrilliantBlender) > 0 && can_play_payload_this_turn() &&
        payload_might_be_in_deck();
    const bool held_manual_completes_energy = !state_.manual_energy_used &&
        ((grass_needed() == 1 && fire_needed() == 0 &&
          hand_count(Card::Grass) > 0) ||
         (grass_needed() == 0 && fire_needed() == 1 &&
          hand_count(Card::Fire) > 0));
    const bool energy_axis_complete = benched_vstar_promotion_ready();

    // At K1, the seed-220 route is fully observable. Wonder Tag should take Burnet
    // instead of Crispin when the held manual Energy already completes GGF, or when
    // GGF has just been completed, and Quick Ball can turn the route-replaced
    // Steven's Resolve into Latias ex. Skyliner then gives the Basic Active a free
    // Retreat while Burnet creates the same-turn Dragon payload. A held payload or
    // live Blender is a cheaper payload axis, so this route does not displace them.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Supporter, Bench, Ability, attachment, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2343
    return scenario_.dci == DciProfile::MatchupFlexJit &&
        scenario_.locks == LockMode::None && state_.turn == 3 &&
        scenario_.max_turn >= 3 && prizes_known() && supporter_allowed() &&
        !item_locked() && !state_.retreat_used && state_.active &&
        is_basic(state_.active->card) && need_active_vstar() && need_payload() &&
        can_play_payload_this_turn() && bench_space() > 0 &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
        deck_count_after_search_started(Card::LatiasEx) > 0 &&
        hand_count(Card::QuickBall) > 0 &&
        hand_count(Card::StevensResolve) > 0 && burnet_available && deck_payload &&
        !held_payload && !blender_payload_route &&
        (energy_axis_complete || held_manual_completes_energy);
  }

  bool issue_2291_wonder_tag_arven_fss_vessel_route() const {
    if (scenario_.dci != DciProfile::StrictJit || !scenario_.going_first ||
        state_.turn != 4 || !prizes_known() || !supporter_allowed() ||
        item_locked() || state_.manual_energy_used || state_.vstar_power_used ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 1 || state_.active->fire != 1 ||
        state_.active->tool != Tool::None ||
        (deck_count_after_search_started(Card::Arven) == 0 && hand_count(Card::Arven) == 0) ||
        deck_count_after_search_started(Card::ForestSealStone) == 0 ||
        deck_count_after_search_started(Card::EarthenVessel) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) != 0 ||
        !in_play(Card::TapuLeleGX) ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // K1 proves Crispin can find only Grass, so its conditional attachment cannot
    // happen. Arven can instead search Forest Seal Stone and Earthen Vessel; Star
    // Alchemy supplies Regidrago VSTAR, Vessel supplies Grass, and the held Dragon
    // pays Vessel's discard cost as the same-turn strict-JIT payload.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Tool, VSTAR Power, Item, discard, search, attachment and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, DCI/AMR and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2291
    return true;
  }

  Card choose_supporter_after_search_started() const {
    if (issue_2291_wonder_tag_arven_fss_vessel_route()) {
      // The complete K1 route is Arven -> FSS/Vessel: https://github.com/FlareZ123/pokemon-sims/issues/2291
      return Card::Arven;
    }
    if (issue_1797_wonder_tag_steven_route_available()) {
      return Card::StevensResolve;
    }
    if (issue_1796_wonder_tag_steven_route_available()) {
      return Card::StevensResolve;
    }
    // A same-turn strict-JIT payload plus Crispin's final Energy completion
    // outranks Gladion's one-Prize exchange. Quick Ball has already paid the
    // Dragon payload cost, Wonder Tag has established K1, and Crispin can supply
    // the last Energy without consuming a later turn:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, search, discard, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, supporter contention, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1870
    const bool crispin_finishes_current_turn_prized_energy_route =
        strict_payload_timing() && supporter_allowed() && active_is_vstar() &&
        payload_ready() && final_prized_energy_for_gladion().has_value() &&
        crispin_can_advance_energy_axis() &&
        deck_count_after_search_started(Card::Crispin) > 0;
    if (crispin_finishes_current_turn_prized_energy_route) return Card::Crispin;

    // Wonder Tag is resolving after legal deck inspection. A known prized
    // immediate Pokemon axis, or final attachable Basic Energy, makes Gladion
    // the fastest Supporter line when no current-turn Crispin completion exists.
    const bool prized_regi = need_regi() && deck_count_after_search_started(Card::RegidragoV) == 0 &&
                             prize_count_after_reveal(Card::RegidragoV) > 0;
    const bool prized_vstar = need_vstar() && deck_count_after_search_started(Card::RegidragoVstar) == 0 &&
                              prize_count_after_reveal(Card::RegidragoVstar) > 0;
    if ((prized_regi || prized_vstar || final_prized_energy_for_gladion().has_value()) &&
        deck_count_after_search_started(Card::Gladion) > 0) return Card::Gladion;

    // Steven's Resolve may take Regidrago V before it enters play, making Wonder Tag
    // a legal turn-one setup connector: https://api.pokemontcg.io/v2/cards/sm7-145 https://api.pokemontcg.io/v2/cards/swsh12-135
    if (state_.turn == 1 && !scenario_.going_first && should_play_steven() &&
        deck_count_after_search_started(Card::StevensResolve) > 0) return Card::StevensResolve;

    const bool pineco_in_play = in_play_count(Card::Pineco) > 0 ||
        in_play_count(Card::ForretressEx) > 0;
    const bool pineco_available = pineco_in_play || hand_count(Card::Pineco) > 0 ||
        deck_count_after_search_started(Card::Pineco) > 0;
    const bool forretress_available = in_play_count(Card::ForretressEx) > 0 ||
        hand_count(Card::ForretressEx) > 0 ||
        deck_count_after_search_started(Card::ForretressEx) > 0;
    const bool prior_turn_pineco = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::Pineco && pokemon.entered_turn < state_.turn;
        }) || (state_.active && state_.active->card == Card::Pineco &&
               state_.active->entered_turn < state_.turn);
    const bool forest_route = state_.turn > 1 &&
        (state_.stadium == Stadium::ForestOfVitality ||
         hand_count(Card::ForestOfVitality) > 0);
    const bool dawn_timing_live =
        (state_.turn == 1 && !scenario_.going_first) || prior_turn_pineco ||
        forest_route;
    // Dawn can collect Pineco, Forretress ex, and a Stage 2 Dragon in one legal
    // Supporter search. Wonder Tag selects that compression route only when the
    // Pineco Bench slot, evolution timing, and Exploding Energy Ability are live:
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Pineco and Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://github.com/FlareZ123/pokemon-sims/issues/1096
    if (forretress_combo_enabled() && need_energy() &&
        ability_available_for_pokemon(Card::ForretressEx) &&
        deck_count_after_search_started(Card::Dawn) > 0 && pineco_available &&
        forretress_available && dawn_timing_live &&
        (pineco_in_play || bench_space() > 0)) {
      return Card::Dawn;
    }

    const bool held_manual_completes_energy = !state_.manual_energy_used &&
        ((grass_needed() == 1 && fire_needed() == 0 && hand_count(Card::Grass) > 0) ||
         (grass_needed() == 0 && fire_needed() == 1 && hand_count(Card::Fire) > 0));
    const bool current_turn_active_vstar_route = active_is_vstar() ||
        (state_.active && state_.active->card == Card::RegidragoV &&
         state_.active->entered_turn < state_.turn && hand_count(Card::RegidragoVstar) > 0);
    const bool held_payload =
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    // Serena's draw mode may discard a held Dragon payload. When one held Basic
    // Energy and the unused manual attachment already complete GGF, Wonder Tag must
    // select that deterministic current-turn payload Supporter before an unused
    // Crispin Energy connector. This preserves the once-per-game Legacy Star:
    // Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR and Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter, evolution, and manual attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1086
    const bool serena_completes_current_turn_route = strict_payload_timing() &&
        supporter_allowed() && need_payload() && can_play_payload_this_turn() &&
        held_manual_completes_energy && current_turn_active_vstar_route && held_payload &&
        deck_count_after_search_started(Card::Serena) > 0;
    if (serena_completes_current_turn_route) return Card::Serena;

    if (wonder_tag_future_gladion_treasure_route()) return Card::Gladion;

    // Burnet plus Quick Ball into Latias ex is the complete same-turn finish when
    // Crispin's Energy axis is already covered by the held manual attachment.
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://github.com/FlareZ123/pokemon-sims/issues/2343
    if (issue_2343_burnet_quick_ball_latias_route_available() &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0) {
      return Card::ProfessorBurnet;
    }

    // Crispin source: https://api.pokemontcg.io/v2/cards/sv7-133
    // Do not choose it after K1 if the inspected deck contains no needed Energy.
    if (need_energy() && crispin_can_advance_energy_axis() && deck_count_after_search_started(Card::Crispin) > 0) return Card::Crispin;

    // Wonder Tag may search Arven, Arven may search Earthen Vessel, and Vessel may
    // search the final Basic Energy for this turn's unused manual attachment:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool exactly_one_energy_missing = grass_needed() + fire_needed() == 1;
    const bool needed_energy_in_deck =
        (grass_needed() == 1 && deck_count_after_search_started(Card::Grass) > 0) ||
        (fire_needed() == 1 && deck_count_after_search_started(Card::Fire) > 0);
    const bool arven_vessel_energy_route = need_energy() && exactly_one_energy_missing &&
        !state_.manual_energy_used && !item_locked() && needed_energy_in_deck &&
        deck_count_after_search_started(Card::Arven) > 0 &&
        deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        is_legal_cost_available(can_play_payload_this_turn(), true);
    if (arven_vessel_energy_route) return Card::Arven;

    // After Wonder Tag establishes K1, choose Arven for the target-legal
    // held-payload Item bridge recognized by the preflight. A direct held Item stays
    // preferable because it avoids spending Tapu's Bench slot and the Supporter play:
    // Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/648
    if (!has_live_one_discard_hand_payload_line() &&
        wonder_tag_arven_held_payload_route_available()) {
      return Card::Arven;
    }

    // Wonder Tag has now inspected the deck. Burnet can only discard cards it
    // selects from that deck, so it is not a payload connector after K1 proves
    // no modeled payload remains: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    const bool deck_payload_available = payload_might_be_in_deck();
    const bool blender_is_playable = deck_payload_available && !item_locked() &&
                                     hand_count(Card::BrilliantBlender) > 0 && can_play_payload_this_turn();
    if (need_payload() && can_play_payload_this_turn() && deck_payload_available && !blender_is_playable &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0) {
      // Burnet directly searches and discards up to 2 cards: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // Wonder Tag must take it when Blender cannot supply this payload line: https://api.pokemontcg.io/v2/cards/sv8-164
      return Card::ProfessorBurnet;
    }
    if (need_vstar() && deck_count_after_search_started(Card::Arven) > 0) return Card::Arven;
    // Wonder Tag can search Tate & Liza, whose switch mode immediately promotes a
    // Benched Regidrago VSTAR. Select Tate when the target already pays GGF and the
    // payload is ready or a payable held one-discard Item can establish it afterward:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // Serena is only a fallback because it cannot promote the Benched attacker:
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_tapu_tate_switch_override.inc#L14-L29
    const bool payload_axis_completable_after_tate = !need_payload() ||
        (state_.deck.size() > 1U && has_live_one_discard_hand_payload_line());
    if (need_active_vstar() && benched_vstar_promotion_ready() &&
        payload_axis_completable_after_tate && !can_free_retreat_with_latias() &&
        deck_count_after_search_started(Card::TateLiza) > 0) {
      return Card::TateLiza;
    }
    if (deck_count_after_search_started(Card::Serena) > 0) return Card::Serena;
    const bool tate_would_promote_incomplete_vstar = need_active_vstar() &&
        !benched_vstar_promotion_ready() && !can_free_retreat_with_latias();
    // Wonder Tag is optional. Do not fall through to Tate's switch route after the
    // target-aware branch rejected an Energy-incomplete promotion:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (deck_count_after_search_started(Card::TateLiza) > 0 &&
        !tate_would_promote_incomplete_vstar) {
      return Card::TateLiza;
    }

    // Wonder Tag may search any Supporter. A completed deck inspection must not
    // nominate an absent hard-coded card while another legal Supporter remains:
    // https://api.pokemontcg.io/v2/cards/sm2-60
    for (const Card fallback : {Card::Dawn, Card::Crispin, Card::Arven,
                                Card::ProfessorBurnet, Card::StevensResolve,
                                Card::Gladion, Card::Lusamine, Card::Guzma,
                                Card::Channeler, Card::TeamYellsCheer,
                                Card::RoseannesBackup, Card::Klara, Card::ProfessorTuro}) {
      if (deck_count_after_search_started(fallback) > 0) return fallback;
    }
    // Wonder Tag is optional. Returning an absent sentinel makes the caller
    // resolve the legal no-card case when the deck contains no Supporter.
    return Card::Crispin;
  }

  std::vector<Card> choose_energy_targets_after_search_started(const int max_cards) const {
    std::vector<Card> targets;
    int g_need = grass_needed();
    int f_need = fire_needed();
    int g_available = deck_count_after_search_started(Card::Grass);
    int f_available = deck_count_after_search_started(Card::Fire);
    while (static_cast<int>(targets.size()) < max_cards) {
      Card wanted = Card::Grass;
      if (g_need > 0 && f_need > 0) {
        wanted = std::find(targets.begin(), targets.end(), Card::Grass) == targets.end() ? Card::Grass : Card::Fire;
      } else if (g_need > 0) {
        wanted = Card::Grass;
      } else if (f_need > 0) {
        wanted = Card::Fire;
      } else {
        // Vital Dance and Earthen Vessel each allow up to 2 Basic Energy cards.
        // After GGF is covered, select another legal Basic Energy for deck thinning
        // and potential duplicate-Energy discard capacity: https://api.pokemontcg.io/v2/cards/sm2-55 https://api.pokemontcg.io/v2/cards/sv4-163
        wanted = g_available > 0 ? Card::Grass : Card::Fire;
      }
      if ((wanted == Card::Grass && g_available == 0) || (wanted == Card::Fire && f_available == 0)) {
        const Card other = wanted == Card::Grass ? Card::Fire : Card::Grass;
        if ((other == Card::Grass && g_available == 0) || (other == Card::Fire && f_available == 0)) break;
        wanted = other;
      }
      targets.push_back(wanted);
      if (wanted == Card::Grass) {
        --g_available;
        if (g_need > 0) --g_need;
      } else {
        --f_available;
        if (f_need > 0) --f_need;
      }
    }
    return targets;
  }

  void search_energy_to_hand(const int max_cards, std::string_view rules, std::string_view source) {
    // A legal Energy search reveals the whole deck even when it finds zero cards.
    record_deck_search_knowledge(source);
    const std::vector<Card> targets = choose_energy_targets_after_search_started(max_cards);
    for (const Card target : targets) {
      if (!move_deck_to_hand(target)) throw std::logic_error("energy search target disappeared");
    }
    shuffle(state_.deck);
    trace(std::string(source), rules, "Searched up to " + std::to_string(max_cards) + " Basic Energy: " + join_cards(targets) + ".");
  }

  bool attach_manual() {
    if (state_.manual_energy_used) return false;
    Pokemon* target = target_regi();
    if (target == nullptr || !need_energy()) return false;
    const auto wanted = preferred_manual_energy_from_hand(*target);
    if (!wanted) return false;
    if (!remove_one(state_.hand, *wanted)) {
      throw std::logic_error("selected manual Energy disappeared");
    }
    if (!attach_energy_card(*target, *wanted)) {
      // DDE can only remain attached to Dragon Pokémon. target_regi() is
      // Regidrago V/VSTAR, both Dragon, so reaching this branch is a bug:
      // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // https://api.pokemontcg.io/v2/cards/swsh12-135
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/issues/2238
      throw std::logic_error("selected manual Energy was illegal on Regidrago");
    }
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY",
          std::string(name(*wanted)) + " manually to " +
              std::string(name(target->card)) + ".");  // DDE-aware selection preserves the existing action contract: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ ; https://github.com/FlareZ123/pokemon-sims/issues/2238
    return true;
  }

  bool evolve_best_regi() {
    if (state_.turn == 1 || hand_count(Card::RegidragoVstar) == 0) return false;
    Pokemon* best = nullptr;