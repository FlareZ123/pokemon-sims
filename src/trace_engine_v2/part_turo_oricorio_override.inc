  bool turo_oricorio_energy_route_live() const {
    if (!supporter_allowed() || hand_count(Card::ProfessorTuro) == 0 ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar ||
        need_regi() || need_vstar() || need_active_vstar() || need_payload()) {
      return false;
    }

    // Professor Turo's Scenario can replay Oricorio, but Vital Dance cannot begin a
    // deck search when zero cards remain. Reject the route before using the Supporter:
    // https://api.pokemontcg.io/v2/cards/sv4-171
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
    if (state_.deck.empty()) return false;

    // This route completes the Active attacker, so compute the deficit from that
    // exact Pokémon rather than from the policy's separately preferred Regidrago:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed + active_fire_needed != 1) return false;

    const Card needed_energy = active_grass_needed == 1 ? Card::Grass : Card::Fire;
    const bool needed_energy_might_be_in_deck = might_be_unseen(needed_energy);
    if (!needed_energy_might_be_in_deck) return false;

    // Crispin is the direct Energy Supporter and remains preferred whenever the
    // same exact Basic Energy is searchable. With one type available, the unused
    // manual attachment can still complete the route:
    // https://api.pokemontcg.io/v2/cards/sv7-133
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (hand_count(Card::Crispin) > 0) return false;

    return std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
      return pokemon.card == Card::Oricorio;
    });
  }

  void discard_turo_attachment(const Card card) {
    state_.discard.push_back(card);
    state_.discarded_this_turn.push_back(card);
  }

  bool play_turo_active_promotion_route() {
    // A direct evolution route requires a prior-turn Regidrago V, the matching
    // Regidrago VSTAR card in hand, and GGF already attached to that Active line.
    // Otherwise Professor Turo may promote an already complete Benched VSTAR:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/sv4-171
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const bool active_regidrago_has_ready_direct_evolution = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0 &&
        state_.active->grass >= 2 && state_.active->fire >= 1;

    // Under Item lock, Turo may solve the Active axis one turn before Burnet solves
    // the strict-JIT payload axis. Admit only a held, searchable Burnet line with a
    // remaining modeled turn, so the current Supporter play has a deterministic
    // completion rather than merely postponing the missing payload:
    // https://api.pokemontcg.io/v2/cards/sv4-171
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/issues/900
    const bool delayed_burnet_payload_route = strict_payload_timing() && item_locked() &&
        need_payload() && state_.turn < scenario_.max_turn &&
        hand_count(Card::ProfessorBurnet) > 0 && payload_might_be_in_deck();
    if (!supporter_allowed() || hand_count(Card::ProfessorTuro) == 0 || !state_.active ||
        !is_basic(state_.active->card) || active_regidrago_has_ready_direct_evolution ||
        !need_active_vstar() || need_regi() || need_vstar() || need_energy() ||
        (need_payload() && !delayed_burnet_payload_route) || can_free_retreat_with_latias() ||
        hand_count(Card::TateLiza) > 0) {
      return false;
    }

    const auto promoted = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoVstar && pokemon.grass >= 2 && pokemon.fire >= 1;
    });
    if (promoted == state_.bench.end()) return false;

    const Pokemon returned = *state_.active;
    remove_one(state_.hand, Card::ProfessorTuro);
    state_.discard.push_back(Card::ProfessorTuro);
    state_.supporter_used = true;

    // Professor Turo's Scenario returns one Pokémon in play to hand and discards
    // every attached card. With the Active Pokémon gone, a Benched Pokémon becomes
    // the replacement Active Pokémon under the core game procedure:
    // https://api.pokemontcg.io/v2/cards/sv4-171
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    for (int energy = 0; energy < returned.grass; ++energy) discard_turo_attachment(Card::Grass);
    for (int energy = 0; energy < returned.fire; ++energy) discard_turo_attachment(Card::Fire);
    if (returned.tool == Tool::ForestSealStone) discard_turo_attachment(Card::ForestSealStone);
    if (returned.tool == Tool::Powerglass) discard_turo_attachment(Card::Powerglass);

    state_.hand.push_back(returned.card);
    state_.active = *promoted;
    state_.bench.erase(promoted);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER",
          "Professor Turo returned the Basic Active Pokémon and promoted the complete Benched Regidrago VSTAR.");
    return true;
  }

  bool play_guzma_active_promotion_route() {
    // Guzma switches the user's Active Pokémon only if the opponent's Benched-to-
    // Active switch was performed. The goldfish scenarios default to Unknown, so
    // this route is legal only under an explicit Available scenario assumption:
    // https://api.pokemontcg.io/v2/cards/sm3-115
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#opponent-actions
    // https://github.com/FlareZ123/pokemon-sims/issues/741
    if (scenario_.opponent_bench != OpponentBenchState::Available ||
        !supporter_allowed() || hand_count(Card::Guzma) == 0 ||
        hand_count(Card::TateLiza) > 0 || !state_.active ||
        !need_active_vstar() || need_regi() || need_vstar() ||
        need_energy() || need_payload() || !benched_vstar_promotion_ready()) {
      return false;
    }

    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr) return false;
    remove_one(state_.hand, Card::Guzma);
    state_.discard.push_back(Card::Guzma);
    state_.supporter_used = true;
    std::swap(*state_.active, *target);
    trace("PLAY SUPPORTER", "R-GUZMA-01; R-GAME-SUPPORTER",
          "The explicit opponent-Bench assumption satisfied Guzma's conditional switch, promoting the complete Benched Regidrago VSTAR.");
    return true;
  }

  bool play_turo_oricorio_energy_route() {
    if (!turo_oricorio_energy_route_live()) return false;

    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const Card needed_energy = active_grass_needed == 1 ? Card::Grass : Card::Fire;
    auto oricorio = std::find_if(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
      return pokemon.card == Card::Oricorio;
    });
    if (oricorio == state_.bench.end()) return false;

    const Pokemon returned = *oricorio;
    remove_one(state_.hand, Card::ProfessorTuro);
    state_.discard.push_back(Card::ProfessorTuro);
    state_.supporter_used = true;

    // Professor Turo's Scenario returns the selected Pokémon to hand and discards
    // every attached card: https://api.pokemontcg.io/v2/cards/sv4-171
    for (int energy = 0; energy < returned.grass; ++energy) discard_turo_attachment(Card::Grass);
    for (int energy = 0; energy < returned.fire; ++energy) discard_turo_attachment(Card::Fire);
    if (returned.tool == Tool::ForestSealStone) discard_turo_attachment(Card::ForestSealStone);
    if (returned.tool == Tool::Powerglass) discard_turo_attachment(Card::Powerglass);
    state_.bench.erase(oricorio);
    state_.hand.push_back(Card::Oricorio);

    // Replaying the returned Basic Pokémon onto the open Bench triggers Vital Dance.
    // Search the exact Basic Energy required by the Active Regidrago VSTAR; Vital
    // Dance permits searching for up to 2, so selecting only the required card is legal:
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (!bench_from_hand(Card::Oricorio, false)) {
      throw std::logic_error("Professor Turo returned Oricorio but its replay failed");
    }
    record_deck_search_knowledge("Vital Dance");
    const bool found = move_deck_to_hand(needed_energy);
    shuffle(state_.deck);
    trace("VITAL DANCE", "R-ORICORIO-01",
          found ? "Searched the exact Basic Energy required by the Active Regidrago VSTAR."
                : "The required Basic Energy was not in the deck after legal search.");

    // A player may attach one Energy from hand during the turn. Complete that
    // attachment here so a separately preferred Benched Regidrago cannot redirect
    // the generic attachment helper away from the Active attacker:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (found) {
      if (!remove_one(state_.hand, needed_energy)) {
        throw std::logic_error("Vital Dance Energy disappeared before manual attachment");
      }
      if (needed_energy == Card::Grass) {
        ++state_.active->grass;
      } else {
        ++state_.active->fire;
      }
      state_.manual_energy_used = true;
      trace("ATTACH", "R-GAME-ENERGY",
            "Manually attached the Vital Dance Energy to the Active Regidrago VSTAR.");
    }

    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER; R-ORICORIO-01",
          "Professor Turo returned and replayed Oricorio for the Active-specific final Energy route.");
    return true;
  }

  bool tate_draw_has_held_non_supporter_completion() const {
    if (!supporter_allowed() || !deck_seen_ || hand_count(Card::TateLiza) == 0 ||
        !setup_axis_missing()) {
      return false;
    }

    // Remove Tate & Liza from the shadow hand and reserve the Supporter play. The
    // exact public remainder of the turn must still complete Active VSTAR, GGF, and
    // the ready-turn payload through held non-Supporter cards:
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Supporter, Item, evolution, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and earliest complete-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1057
    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    while (remove_one(projected.state_.hand, Card::TateLiza)) {
    }
    projected.state_.supporter_used = true;
    projected.run_turn();

    return projected.active_is_vstar() && projected.state_.active->grass >= 2 &&
        projected.state_.active->fire >= 1 && projected.payload_ready();
  }

  void choose_supporter() {
    if (!supporter_allowed()) return;
    if (tate_draw_has_held_non_supporter_completion()) {
      // Shuffling the held Item, Evolution, Energy, or payload cards would replace a
      // deterministic current-turn finish with a weaker random route. Preserve Tate:
      // https://api.pokemontcg.io/v2/cards/sm7-148
      // https://api.pokemontcg.io/v2/cards/sv8-164
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://github.com/FlareZ123/pokemon-sims/issues/1057
      if (trace_ != nullptr) {
        trace_->add_policy_once(
            state_, "issue-1057-held-non-supporter-route", state_.turn,
            "HOLD SUPPORTER", "R-TATE-01; P-KNOWLEDGE-01; P-COMPRESS-01",
            "Retained Tate & Liza because held non-Supporter cards already complete the current-turn ready route.");
      }
      return;
    }
    // Roseanne's Backup may choose its Pokémon and Energy modes together, so resolve
    // that two-axis route before single-axis recovery policies:
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    if (play_roseanne_vstar_energy_recovery()) return;
    // Evaluate Active-specific routes before the generic preferred-Regidrago axis
    // gate, which may consider a different Benched Regidrago already complete.
    if (play_guzma_active_promotion_route()) return;
    if (play_turo_oricorio_energy_route()) return;
    if (play_turo_active_promotion_route()) return;
    if (!setup_axis_missing()) return;
    choose_supporter_turo_original();
  }
