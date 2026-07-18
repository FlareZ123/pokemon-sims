  std::vector<Card> lusamine_recovery_targets() const {
    // Lusamine says to put 2 Supporter and/or Stadium cards from discard into hand:
    // https://api.pokemontcg.io/v2/cards/sm4-96
    // A policy-first recovery requires one card that advances an unresolved setup
    // axis. Once that card makes Lusamine worth playing, the second selection is
    // mandatory and uses a deterministic low-impact eligible card.
    std::vector<Card> targets;
    const auto recover_once = [this, &targets](const Card card) {
      if (targets.size() < 2U && count_of(state_.discard, card) > count_of(targets, card)) {
        targets.push_back(card);
      }
    };

    const bool legal_search_cost = is_legal_cost_available(can_play_payload_this_turn(), true);
    const bool known_gladion_axis = prizes_known() &&
        ((need_vstar() && prize_count_after_reveal(Card::RegidragoVstar) > 0) ||
         (need_regi() && bench_space() > 0 && prize_count_after_reveal(Card::RegidragoV) > 0) ||
         final_prized_energy_for_gladion().has_value());
    const bool arven_vstar_axis = need_vstar() && !item_locked() &&
                                  (might_be_unseen(Card::EvolutionIncense) ||
                                   (legal_search_cost && might_be_unseen(Card::MysteriousTreasure)));
    const bool arven_regi_axis = need_regi() && bench_space() > 0 && !item_locked() &&
                                 legal_search_cost &&
                                 (might_be_unseen(Card::QuickBall) || might_be_unseen(Card::MysteriousTreasure));
    const bool arven_energy_axis = need_energy() && !item_locked() &&
                                   legal_search_cost && might_be_unseen(Card::EarthenVessel);
    const bool arven_payload_axis = need_payload() && !item_locked() &&
                                    might_be_unseen(Card::BrilliantBlender);
    const bool arven_communication_axis = !item_locked() &&
        might_be_unseen(Card::PokemonCommunication) &&
        pokemon_communication_plan_after_playing_supporter(
            Card::Lusamine, false).has_value();
    // Lusamine may recover Arven for a future turn where Arven searches Pokémon
    // Communication. The exchange must already have a safe hand Pokémon and a live
    // target under the same route-aware planner used by direct Arven resolution:
    // https://api.pokemontcg.io/v2/cards/sm4-96
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://github.com/FlareZ123/pokemon-sims/issues/701
    // Arven is a Supporter, so Item lock does not prohibit it. Arven may select no
    // Item and search Forest Seal Stone as its Pokémon Tool target:
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
    // Pokémon V includes Pokémon VSTAR, and Forest Seal Stone's Star Alchemy may
    // search any card. Under Item lock, Professor Burnet is the concrete delayed
    // payload outlet because it searches and discards the Dragon on the next turn:
    // https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/705
    const bool open_fss_holder =
        (state_.active && is_pokemon_v(state_.active->card) && state_.active->tool == Tool::None) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return is_pokemon_v(pokemon.card) && pokemon.tool == Tool::None;
        });
    // Lusamine spends the current turn. An unlocked Blender route needs the next
    // turn for Arven and Star Alchemy; the Item-lock Burnet route needs one further
    // turn to play the banked Supporter. Do not begin a chain beyond the configured
    // simulation horizon:
    // https://api.pokemontcg.io/v2/cards/sm4-96
    // https://api.pokemontcg.io/v2/cards/sv1-166
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // https://github.com/FlareZ123/pokemon-sims/issues/705
    const bool fss_payload_axis = need_payload() && can_play_payload_this_turn() &&
        payload_might_be_in_deck() &&
        ((!item_locked() && state_.turn < scenario_.max_turn &&
          might_be_unseen(Card::BrilliantBlender)) ||
         (item_locked() && state_.turn + 2 <= scenario_.max_turn &&
          might_be_unseen(Card::ProfessorBurnet)));
    const bool known_prized_vstar_fss_axis = prizes_known() && need_vstar() &&
        prize_count_after_reveal(Card::RegidragoVstar) > 0 && might_be_unseen(Card::Gladion);
    const bool fss_live_axis = known_prized_vstar_fss_axis ||
        (need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV)) ||
        (need_vstar() && might_be_unseen(Card::RegidragoVstar)) ||
        (need_energy() && !state_.manual_energy_used &&
         ((grass_needed() > 0 && might_be_unseen(Card::Grass)) ||
          (fire_needed() > 0 && might_be_unseen(Card::Fire)))) ||
        (need_active_vstar() && bench_space() > 0 && !in_play(Card::LatiasEx) &&
         might_be_unseen(Card::LatiasEx)) ||
        fss_payload_axis;
    const bool arven_fss_axis = !state_.vstar_power_used && open_fss_holder &&
        might_be_unseen(Card::ForestSealStone) && fss_live_axis;


    if (known_gladion_axis) recover_once(Card::Gladion);
    if (arven_vstar_axis || arven_regi_axis || arven_fss_axis ||
        arven_communication_axis) recover_once(Card::Arven);
    if (need_active_vstar()) recover_once(Card::TateLiza);
    if (need_energy() && crispin_can_advance_energy_axis()) recover_once(Card::Crispin);
    if (need_payload()) recover_once(Card::ProfessorBurnet);
    if (arven_energy_axis || arven_payload_axis) recover_once(Card::Arven);

    if (targets.empty()) return targets;

    // These are the remaining modeled Supporter and Stadium cards, ordered from
    // least direct setup value to the connectors retained above. The list contains
    // only legal Lusamine targets and may select a second copy where available.
    // Erika's Invitation is a Supporter and the canonical low-impact replacement
    // for retired Mawile-GX. Once an advancing target makes Lusamine worth playing,
    // Erika may satisfy the printed mandatory second Supporter/Stadium recovery:
    // https://api.pokemontcg.io/v2/cards/sm4-96
    // https://api.pokemontcg.io/v2/cards/sv3pt5-160
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/CARD_AUDIT.md#erikas-invitation
    // https://github.com/FlareZ123/pokemon-sims/issues/854
    for (const Card fallback : {Card::ErikasInvitation, Card::ChaoticSwell, Card::PathToPeak,
                                Card::TeamYellsCheer, Card::RoseannesBackup, Card::ProfessorTuro,
                                Card::Channeler, Card::Guzma, Card::Serena, Card::TateLiza,
                                Card::StevensResolve, Card::Arven, Card::Crispin,
                                Card::ProfessorBurnet, Card::Gladion}) {
      recover_once(fallback);
    }
    return targets;
  }

  bool play_lusamine() {
    if (!supporter_allowed() || hand_count(Card::Lusamine) == 0) return false;
    const std::vector<Card> targets = lusamine_recovery_targets();
    // Lusamine requires exactly 2 Supporter and/or Stadium cards from the public
    // discard pile. It does not say "up to 2", so one eligible target is insufficient:
    // https://api.pokemontcg.io/v2/cards/sm4-96
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
    if (targets.size() != 2U) return false;
    remove_one(state_.hand, Card::Lusamine);
    state_.discard.push_back(Card::Lusamine);
    state_.supporter_used = true;
    for (const Card target : targets) {
      if (!remove_one(state_.discard, target)) throw std::logic_error("Lusamine recovery target disappeared");
      state_.hand.push_back(target);
    }
    trace("PLAY SUPPORTER", "R-LUSAMINE-01; R-GAME-SUPPORTER; P-FUTURE-01",
          "Recovered from discard: " + join_cards(targets) + ".");
    return true;
  }

  bool has_regidrago_v_that_can_evolve_this_turn() const {
    // An Evolution card may be played on any matching Pokémon that was already in
    // play before this turn, whether that Pokémon is Active or Benched:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // Regidrago VSTAR evolves from Regidrago V:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const auto can_evolve = [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV && pokemon.entered_turn < state_.turn;
    };
    return (state_.active && can_evolve(*state_.active)) ||
           std::any_of(state_.bench.begin(), state_.bench.end(), can_evolve);
  }

  bool has_payable_roseanne_vstar_search() {
    if (item_locked()) return false;

    const std::vector<Card> original_hand = state_.hand;
    const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
    if (!remove_one(state_.hand, Card::RoseannesBackup)) return false;

    // Evolution Incense searches an Evolution Pokémon without a discard cost:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    if (hand_count(Card::EvolutionIncense) > 0) {
      restore_hand();
      return true;
    }

    // Roseanne's Backup restores Regidrago VSTAR before Pokémon Communication
    // resolves. Require an exact post-Supporter exchange plan for that restored
    // target instead of counting the Item by name alone:
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // https://api.pokemontcg.io/v2/cards/sm9-152
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/701
    restore_hand();
    if (pokemon_communication_can_find_after_recovery(
            Card::RegidragoVstar, Card::RoseannesBackup,
            {Card::RegidragoVstar})) {
      return true;
    }
    if (!remove_one(state_.hand, Card::RoseannesBackup)) return false;

    // Mysterious Treasure requires another hand card before it searches a Dragon
    // Pokémon. Test that cost after the Supporter has left hand:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    const bool permit_payload = can_play_payload_this_turn();
    if (hand_count(Card::MysteriousTreasure) > 0 &&
        choose_discard(permit_payload, true, true, Card::MysteriousTreasure).has_value()) {
      restore_hand();
      return true;
    }

    // Ultra Ball needs two other cards after Roseanne's Backup is played:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    if (hand_count(Card::UltraBall) > 0) {
      remove_one(state_.hand, Card::UltraBall);
      const auto first = choose_discard(permit_payload, true);
      if (first && remove_one(state_.hand, *first)) {
        const auto second = choose_discard(permit_payload && !is_payload(*first), true);
        if (second) {
          restore_hand();
          return true;
        }
      }
    }

    restore_hand();
    return false;
  }

  bool play_roseanne_vstar_recovery() {
    if (!supporter_allowed() || hand_count(Card::RoseannesBackup) == 0 || !need_vstar() ||
        !has_regidrago_v_that_can_evolve_this_turn() ||
        count_of(state_.discard, Card::RegidragoVstar) == 0) {
      return false;
    }

    // Use the recovery only after legal knowledge proves no VSTAR remains in deck and
    // a currently payable Item can search the restored Evolution this turn.
    if (might_be_unseen(Card::RegidragoVstar) || !has_payable_roseanne_vstar_search()) return false;

    remove_one(state_.hand, Card::RoseannesBackup);
    state_.discard.push_back(Card::RoseannesBackup);
    state_.supporter_used = true;

    // Roseanne's Backup may choose its Pokémon mode and shuffle one Pokémon from
    // discard into the deck: https://api.pokemontcg.io/v2/cards/swsh9-148
    if (!remove_one(state_.discard, Card::RegidragoVstar)) {
      throw std::logic_error("Roseanne's Backup VSTAR target disappeared");
    }
    const auto turn_it = std::find(state_.discarded_this_turn.begin(),
                                   state_.discarded_this_turn.end(),
                                   Card::RegidragoVstar);
    if (turn_it != state_.discarded_this_turn.end()) state_.discarded_this_turn.erase(turn_it);
    state_.deck.push_back(Card::RegidragoVstar);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER",
          "Roseanne's Backup shuffled Regidrago VSTAR from discard into the deck for a payable same-turn search.");
    return true;
  }

  bool has_payable_roseanne_energy_vessel() {
    if (item_locked() || hand_count(Card::EarthenVessel) == 0) return false;

    const std::vector<Card> original_hand = state_.hand;
    const auto restore_hand = [this, &original_hand]() { state_.hand = original_hand; };
    if (!remove_one(state_.hand, Card::RoseannesBackup)) return false;

    // Roseanne's Backup leaves hand before Earthen Vessel pays its required discard
    // cost. The Vessel itself cannot be that other discarded card:
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // When a different Dragon already established the strict-JIT payload this turn,
    // an additional modeled Dragon is a legal Vessel cost and adds another Apex Dragon option:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    const bool payable = choose_discard(payload_ready(), true, true, Card::EarthenVessel).has_value();
    restore_hand();
    return payable;
  }

  bool play_roseanne_energy_recovery() {
    if (!supporter_allowed() || hand_count(Card::RoseannesBackup) == 0 ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoVstar || need_regi() || need_vstar() ||
        need_active_vstar() || need_payload()) {
      return false;
    }

    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed + active_fire_needed != 1) return false;
    const Card needed_energy = active_grass_needed == 1 ? Card::Grass : Card::Fire;

    // Roseanne's Backup may choose its Energy mode and shuffle one Energy from the
    // discard pile into the deck. Earthen Vessel can then search that restored Basic
    // Energy for the still-unused manual attachment in the same turn:
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (count_of(state_.discard, needed_energy) == 0 || might_be_unseen(needed_energy) ||
        !has_payable_roseanne_energy_vessel()) {
      return false;
    }

    remove_one(state_.hand, Card::RoseannesBackup);
    state_.discard.push_back(Card::RoseannesBackup);
    state_.supporter_used = true;
    if (!remove_one(state_.discard, needed_energy)) {
      throw std::logic_error("Roseanne's Backup Energy target disappeared");
    }
    const auto turn_it = std::find(state_.discarded_this_turn.begin(),
                                   state_.discarded_this_turn.end(), needed_energy);
    if (turn_it != state_.discarded_this_turn.end()) state_.discarded_this_turn.erase(turn_it);
    state_.deck.push_back(needed_energy);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-GAME-SUPPORTER",
          "Roseanne's Backup shuffled the missing Basic Energy into the deck for a payable same-turn Earthen Vessel search.");
    return true;
  }

  void choose_supporter() {
    if (!supporter_allowed() || !setup_axis_missing()) return;

    // Tate & Liza switch mode can repair the Active axis before a remaining Item
    // payload route resolves later in the same turn. Pick switch mode over draw
    // mode when Brilliant Blender already supplies the current-turn payload:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool tate_switch_only_active_axis = need_active_vstar() && !need_energy() && !need_payload();
    const bool tate_switch_then_blender_payload = need_active_vstar() && !can_free_retreat_with_latias() &&
        !need_energy() && need_payload() && has_live_blender_payload_line();
    if ((tate_switch_only_active_axis || tate_switch_then_blender_payload) && play_tate_switch()) return;

    // Exact Serena text: discard up to 3 cards, but "You must discard at least 1 card."
    // A zero-discard draw is illegal: https://api.pokemontcg.io/v2/cards/swsh12-164
    // This route requires an in-hand payload as that mandatory discard, then resolves
    // Serena before Prize recovery spends the only Supporter play.
    const bool serena_completes_only_missing_axis = need_payload() && !need_regi() && !need_vstar() &&
        !need_energy() && !need_active_vstar() && can_play_payload_this_turn() &&
        hand_count(Card::Serena) > 0 && std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    // The dedicated route may accept zero draw because the mandatory Dragon
    // discard itself completes the final current-turn strict-JIT axis:
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://github.com/FlareZ123/pokemon-sims/issues/882
    if (serena_completes_only_missing_axis && play_serena(true)) return;

    // Roseanne's Backup can restore a known discarded Regidrago VSTAR before a
    // payable Item searches and evolves it in the same turn:
    // https://api.pokemontcg.io/v2/cards/swsh9-148
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    if (play_roseanne_vstar_recovery()) return;
    if (play_roseanne_energy_recovery()) return;

    if (prizes_known() && play_gladion()) return;
    if (play_steven()) return;
    if (need_energy() && hand_count(Card::Crispin) > 0 && play_crispin()) return;
    // Brilliant Blender is an Item that searches the deck and discards the current-turn payload: https://api.pokemontcg.io/v2/cards/sv8-164
    // Preserve Burnet's one Supporter play while that live Item line supplies the same payload axis: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26 https://www.pokemon.com/us/pokemon-tcg/rules
    if (need_payload() && hand_count(Card::ProfessorBurnet) > 0 && !has_live_blender_payload_line() && play_professor_burnet()) return;
    if (hand_count(Card::Arven) > 0 && play_arven()) return;
    if (hand_count(Card::Serena) > 0 && play_serena()) return;
    if (hand_count(Card::TateLiza) > 0 && play_tate_draw()) return;
    if (need_active_vstar() && play_tate_switch()) return;

    // Lusamine is a delayed recovery line and must not preempt a live direct connector.
    if (play_lusamine()) return;
    play_gladion();
  }
