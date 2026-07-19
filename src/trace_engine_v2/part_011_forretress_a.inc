  bool use_fss() {
    if (state_.vstar_power_used) return false;
    Pokemon* holder = nullptr;
    if (state_.active && state_.active->tool == Tool::ForestSealStone) holder = &*state_.active;
    if (holder == nullptr) {
      for (Pokemon& p : state_.bench) if (p.tool == Tool::ForestSealStone) { holder = &p; break; }
    }
    // Star Alchemy is printed on Forest Seal Stone, a Pokémon Tool; the attached
    // Pokémon V can use that Tool's VSTAR Power: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Path to the Peak removes Abilities from Pokémon with Rule Boxes. The published
    // ruling says the Pokémon V only has access to the Tool's Ability, so Path does
    // not remove Star Alchemy: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
    // Path to the Peak card text: https://api.pokemontcg.io/v2/cards/swsh6-148
    if (holder == nullptr || !is_pokemon_v(holder->card)) return false;
    const bool tate_solves_only_missing_axis = need_active_vstar() && !need_energy() && !need_payload() &&
                                               supporter_allowed() && hand_count(Card::TateLiza) > 0;
    const bool crispin_solves_only_missing_axis = need_energy() && !need_regi() && !need_vstar() &&
                                                  !need_active_vstar() && !need_payload() && supporter_allowed() &&
                                                  hand_count(Card::Crispin) > 0 && crispin_can_advance_energy_axis();
    const bool useful = !tate_solves_only_missing_axis && !crispin_solves_only_missing_axis &&
                        (should_play_steven() || need_regi() || need_vstar() || need_energy() || need_payload() || need_active_vstar());
    // Star Alchemy's unrestricted search is mandatory because its text gives no
    // choice: https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://compendium.pokegym.net/category/2-meta-rulings/
    // With one card left in deck, using it necessarily empties Professor Burnet's
    // only payload zone. Preserve Burnet through Item lock instead:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    const bool mandatory_search_breaks_only_burnet_line = need_payload() && item_locked() &&
        supporter_allowed() && hand_count(Card::ProfessorBurnet) > 0 && state_.deck.size() == 1U &&
        payload_might_be_in_deck();
    if (!useful || mandatory_search_breaks_only_burnet_line) return false;
    state_.vstar_power_used = true;
    outcome_.used_fss = true;
    // Card text source: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Star Alchemy is a deck search; target policy runs after that legal inspection.
    record_deck_search_knowledge("Star Alchemy");
    Card target = fss_target_after_search_started();
    const bool burnet_overrides_dead_fallback = need_payload() && payload_might_be_in_deck() &&
        (item_locked() || deck_count_after_search_started(Card::BrilliantBlender) == 0) &&
        deck_count_after_search_started(Card::ProfessorBurnet) > 0 &&
        (target == Card::Arven || target == Card::MysteriousTreasure || target == Card::QuickBall || target == Card::Serena);
    if (burnet_overrides_dead_fallback) {
      // Burnet directly searches and discards up to 2 cards: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
      // It replaces a dead Blender fallback only while a payload may remain in deck.
      target = Card::ProfessorBurnet;
    }
    bool found = move_deck_to_hand(target);
    if (!found) {
      for (const Card fallback : {Card::RegidragoVstar, Card::Crispin, Card::BrilliantBlender,
                                  Card::ProfessorBurnet, Card::Arven, Card::MysteriousTreasure,
                                  Card::QuickBall, Card::EarthenVessel, Card::LatiasEx}) {
        if (fallback == Card::Crispin && !crispin_can_advance_energy_axis()) continue;
        if ((fallback == Card::BrilliantBlender || fallback == Card::ProfessorBurnet) && need_payload() &&
            !payload_might_be_in_deck()) continue;
        if (move_deck_to_hand(fallback)) { target = fallback; found = true; break; }
      }
    }
    // Star Alchemy searches for any card. Once its VSTAR Power has been used and
    // the deck is legally inspected, take an actually present card rather than
    // falsely resolving for nothing from a nonempty deck:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://compendium.pokegym.net/category/2-meta-rulings/
    if (!found && !state_.deck.empty()) {
      target = state_.deck.back();
      found = move_deck_to_hand(target);
    }
    shuffle(state_.deck);
    trace("STAR ALCHEMY", "R-FSS-02; R-VSTAR-01", found ? "Searched any card: " + std::string(name(target)) + "." : "No card remained in deck.");
    return true;
  }

  bool play_crispin() {
    if (!supporter_allowed() || hand_count(Card::Crispin) == 0 || target_regi() == nullptr ||
        !crispin_can_advance_energy_axis()) return false;

    // A one-type search still has value: Crispin puts that Energy into hand,
    // where the unused manual attachment can advance the missing GGF component.
    const bool deck_has_grass = deck_count_after_search_started(Card::Grass) > 0;
    const bool deck_has_fire = deck_count_after_search_started(Card::Fire) > 0;

    remove_one(state_.hand, Card::Crispin);
    state_.discard.push_back(Card::Crispin);
    state_.supporter_used = true;

    // Crispin still inspects the whole deck if only one or no type exists.
    // Card text source: https://api.pokemontcg.io/v2/cards/sv7-133
    record_deck_search_knowledge("Crispin");
    if (!deck_has_grass || !deck_has_fire) {
      if (deck_has_grass) move_deck_to_hand(Card::Grass);
      else if (deck_has_fire) move_deck_to_hand(Card::Fire);
      shuffle(state_.deck);
      trace("PLAY SUPPORTER", "R-CRISPIN-01; R-GAME-SUPPORTER", "Only one Basic Energy type was searchable; put it into hand and attached none.");
      return true;
    }

    Pokemon* target = target_regi();
    const Card attach = grass_needed() > 0 ? Card::Grass : Card::Fire;
    const Card to_hand = attach == Card::Grass ? Card::Fire : Card::Grass;
    if (!remove_one(state_.deck, attach) || !move_deck_to_hand(to_hand)) {
      throw std::logic_error("Crispin target disappeared during legal search");
    }
    if (attach == Card::Grass) ++target->grass; else ++target->fire;
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-CRISPIN-01; R-GAME-SUPPORTER", "Searched " + std::string(name(attach)) + " and " + std::string(name(to_hand)) + "; attached " + std::string(name(attach)) + " to " + std::string(name(target->card)) + " and put " + std::string(name(to_hand)) + " into hand.");
    return true;
  }

  bool play_professor_burnet() {
    // Burnet searches deck and discards its selected cards: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // K1 must preserve the Supporter slot once the inspected deck has no payload.
    if (!supporter_allowed() || hand_count(Card::ProfessorBurnet) == 0 || !can_play_payload_this_turn() ||
        !payload_might_be_in_deck()) return false;
    remove_one(state_.hand, Card::ProfessorBurnet);
    state_.discard.push_back(Card::ProfessorBurnet);
    state_.supporter_used = true;
    std::vector<Card> discarded;
    for (const Card card : {Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar, Card::DialgaGX}) {
      if (move_deck_to_discard(card)) { discarded.push_back(card); break; }
    }
    if (move_deck_to_discard(Card::Dipplin)) discarded.push_back(Card::Dipplin);
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-BURNET-01; R-GAME-SUPPORTER; P-DCI-01", "Searched and discarded: " + join_cards(discarded) + ".");
    return true;
  }

  bool steven_held_routes_complete_next_turn(
      const std::vector<Card>& wanted, const bool next_turn_item_locked,
      const bool projected_vstar_available) const {
    const bool prior_turn_active_regi_can_evolve = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn && projected_vstar_available;
    const bool held_manual_energy_completes_next_turn =
        (grass_needed() == 1 && fire_needed() == 0 && hand_count(Card::Grass) > 0) ||
        (grass_needed() == 0 && fire_needed() == 1 && hand_count(Card::Fire) > 0);
    const auto projected_deck_copy_remains = [&wanted, this](const Card card) {
      return deck_count_after_search_started(card) >
          static_cast<int>(std::count(wanted.begin(), wanted.end(), card));
    };
    const bool quick_ball_target_remains = std::any_of(
        state_.deck.begin(), state_.deck.end(), [projected_deck_copy_remains](const Card card) {
          return is_basic(card) && projected_deck_copy_remains(card);
        });
    const auto quick_ball_cost = choose_discard(true, true, true, Card::QuickBall);
    const bool held_quick_ball_payload_route = !next_turn_item_locked &&
        hand_count(Card::QuickBall) > 0 && quick_ball_target_remains &&
        quick_ball_cost && is_payload(*quick_ball_cost);

    // Steven searches up to three cards. When the prior-turn Active Regidrago V can
    // evolve next turn, a held Basic Energy completes GGF, and target-legal Quick
    // Ball can discard the held Dragon payload, the VSTAR card is the only missing
    // search target. Preserve Crispin and the one-copy ACE SPEC in deck:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/978
    return scenario_.dci == DciProfile::StrictJit &&
           state_.turn < scenario_.max_turn && prior_turn_active_regi_can_evolve &&
           held_manual_energy_completes_next_turn && need_payload() &&
           held_quick_ball_payload_route;
  }

  bool play_steven() {
    if (!supporter_allowed() || hand_count(Card::StevensResolve) == 0 || !should_play_steven()) return false;
    const bool known_t3_compression = late_steven_has_known_t3_compression_route();
    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
