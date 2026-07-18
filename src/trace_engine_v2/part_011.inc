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

  bool play_steven() {
    if (!supporter_allowed() || hand_count(Card::StevensResolve) == 0 || !should_play_steven()) return false;
    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;

    // Card text source: https://api.pokemontcg.io/v2/cards/sm7-145
    // Steven's Resolve is a legal deck inspection. It can therefore reserve
    // Gladion when that inspection proves a missing VSTAR is prized.
    record_deck_search_knowledge("Steven's Resolve");
    const bool next_turn_item_locked = scenario_.locks == LockMode::FullItem || scenario_.locks == LockMode::FullCombined ||
                                       (scenario_.locks == LockMode::TurnTwoItem && state_.turn == 1);
    const bool blender_available_next_turn = !next_turn_item_locked && payload_might_be_in_deck() &&
                                             deck_count_after_search_started(Card::BrilliantBlender) > 0;
    // Burnet directly searches and discards up to 2 cards: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Steven's Resolve reserves an outlet only where it can still find a payload.
    const Card payload_outlet = blender_available_next_turn ? Card::BrilliantBlender : Card::ProfessorBurnet;
    std::vector<Card> wanted;
    const auto add_wanted = [&wanted](const Card card) {
      if (std::find(wanted.begin(), wanted.end(), card) == wanted.end()) wanted.push_back(card);
    };

    // Steven searches up to three unrestricted cards at once. Project prerequisite
    // targets before choosing later axes: https://api.pokemontcg.io/v2/cards/sm7-145
    // Regidrago V evolves into Regidrago VSTAR. Gladion exchanges for exactly one
    // Prize card, while Hisuian Heavy Ball can recover a prized Basic independently:
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm4-95
    // https://api.pokemontcg.io/v2/cards/swsh10-146
    // Repository route priority and multi-Prize accounting:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed defect: https://github.com/FlareZ123/pokemon-sims/issues/867
    const bool regi_in_deck = deck_count_after_search_started(Card::RegidragoV) > 0;
    const bool vstar_in_deck = deck_count_after_search_started(Card::RegidragoVstar) > 0;
    const bool regi_known_prized = prize_count_after_reveal(Card::RegidragoV) > 0;
    const bool vstar_known_prized = prize_count_after_reveal(Card::RegidragoVstar) > 0;
    const bool heavy_ball_available = hand_count(Card::HisuianHeavyBall) > 0 ||
                                      deck_count_after_search_started(Card::HisuianHeavyBall) > 0;
    const bool gladion_available = hand_count(Card::Gladion) > 0 ||
                                   deck_count_after_search_started(Card::Gladion) > 0;
    const auto reserve_if_needed = [&add_wanted, this](const Card card) {
      if (hand_count(card) == 0) add_wanted(card);
    };

    bool projected_regi_available = has_regi_v() || hand_count(Card::RegidragoV) > 0;
    bool projected_regi_uses_gladion = false;
    bool projected_vstar_uses_gladion = false;
    if (need_regi() && bench_space() > 0 && hand_count(Card::RegidragoV) == 0) {
      if (regi_in_deck) {
        add_wanted(Card::RegidragoV);
        projected_regi_available = true;
      } else if (regi_known_prized && gladion_available) {
        // One Gladion cannot recover both a prized Basic and a prized VSTAR. When
        // both axes are prized, reserve Heavy Ball for the Basic and Gladion for the
        // evolution. Otherwise Gladion may bridge the Basic by itself.
        if (!vstar_in_deck && vstar_known_prized && heavy_ball_available) {
          reserve_if_needed(Card::HisuianHeavyBall);
          reserve_if_needed(Card::Gladion);
          projected_regi_available = true;
          projected_vstar_uses_gladion = true;
        } else {
          reserve_if_needed(Card::Gladion);
          projected_regi_available = true;
          projected_regi_uses_gladion = true;
        }
      }
    }

    const bool projected_vstar_missing = projected_regi_available && !has_vstar() &&
                                         hand_count(Card::RegidragoVstar) == 0;
    bool projected_vstar_direct = false;
    if (projected_vstar_missing) {
      if (vstar_in_deck) {
        add_wanted(Card::RegidragoVstar);
        projected_vstar_direct = true;
      } else if (vstar_known_prized && gladion_available && !projected_regi_uses_gladion) {
        reserve_if_needed(Card::Gladion);
        projected_vstar_uses_gladion = true;
      }
    }

    // Crispin source: https://api.pokemontcg.io/v2/cards/sv7-133
    if (need_energy() && hand_count(Card::Crispin) == 0 && crispin_can_advance_energy_axis() &&
        deck_count_after_search_started(Card::Crispin) > 0) {
      add_wanted(Card::Crispin);
    }

    const bool projected_vstar_available = has_vstar() || hand_count(Card::RegidragoVstar) > 0 ||
        projected_vstar_direct || projected_vstar_uses_gladion;
    const bool projected_crispin_available = hand_count(Card::Crispin) > 0 ||
        std::find(wanted.begin(), wanted.end(), Card::Crispin) != wanted.end();
    const Pokemon* projected_regi = target_regi();
    const bool projected_legacy_star_payload_route = projected_vstar_available &&
        !state_.vstar_power_used && ability_available_for_pokemon(Card::RegidragoVstar) &&
        payload_might_be_in_deck();
    const bool payload_route_preserved = !need_payload() ||
        (!next_turn_item_locked && hand_count(Card::BrilliantBlender) > 0) ||
        hand_count(Card::ProfessorBurnet) > 0 || projected_legacy_star_payload_route;
    const bool basic_active_needs_promotion = state_.active && is_basic(state_.active->card) &&
                                              state_.active->card != Card::RegidragoV;
    const bool crispin_and_manual_complete_ggf = projected_regi != nullptr &&
        projected_regi->grass >= 1 &&
        deck_count_after_search_started(Card::Grass) > 0 &&
        deck_count_after_search_started(Card::Fire) > 0;
    // Latias ex gives Basic Pokémon no Retreat Cost. Once Steven's first targets
    // establish VSTAR plus GGF behind a Basic Active, promotion outranks a redundant
    // Pokémon fallback: https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (wanted.size() < 3U && projected_vstar_available && projected_crispin_available &&
        crispin_and_manual_complete_ggf && payload_route_preserved && basic_active_needs_promotion &&
        bench_space() > 0 && !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        deck_count_after_search_started(Card::LatiasEx) > 0) {
      add_wanted(Card::LatiasEx);
    }

    if (wanted.size() < 3U && need_payload() && payload_might_be_in_deck() &&
        hand_count(Card::BrilliantBlender) == 0 && hand_count(Card::ProfessorBurnet) == 0 &&
        deck_count_after_search_started(payload_outlet) > 0) {
      add_wanted(payload_outlet);
    }
    for (const Card fallback : {Card::RegidragoV, Card::RegidragoVstar, Card::Gladion, Card::Crispin, payload_outlet,
                                Card::MysteriousTreasure, Card::EarthenVessel, Card::LatiasEx}) {
      if (wanted.size() >= 3U) break;
      if (fallback == Card::Crispin && !crispin_can_advance_energy_axis()) continue;
      if (fallback == payload_outlet && !payload_might_be_in_deck()) continue;
      add_wanted(fallback);
    }
    std::vector<Card> found;
    for (const Card target : wanted) {
      if (found.size() >= 3U) break;
