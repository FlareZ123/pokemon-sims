  bool issue_1478_t1_field_blower_direct_regi_route_available() const {
    if (current_deck_id() != "regidrago-shell" ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || state_.turn != 1 ||
        !scenario_.going_first || scenario_.max_turn < 3 || item_locked() ||
        state_.supporter_used || state_.manual_energy_used ||
        state_.retreat_used || state_.vstar_power_used || !state_.active ||
        state_.active->card != Card::Oricorio ||
        state_.active->grass != 0 || state_.active->fire != 0 ||
        state_.active->tool != Tool::None || bench_space() <= 0 ||
        state_.stadium != Stadium::None ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::FieldBlower) == 0 ||
        hand_count(Card::ForestSealStone) == 0 ||
        hand_count(Card::Crispin) == 0 ||
        hand_count(Card::Arven) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        hand_count(Card::Fire) == 0 ||
        hand_count(Card::RegidragoV) > 0 ||
        hand_count(Card::RegidragoVstar) > 0) {
      return false;
    }
    const bool benched_tool_target = std::any_of(
        state_.bench.begin(), state_.bench.end(),
        [](const Pokemon& pokemon) { return pokemon.tool != Tool::None; });
    if (benched_tool_target) return false;

    // Field Blower has no Tool or Stadium target in this exact public no-lock
    // state. Mysterious Treasure may spend it to establish Regidrago V, then the
    // held Fire pays Oricorio's one-Energy Retreat Cost. Forest Seal Stone,
    // Crispin, Arven, Earthen Vessel, and Brilliant Blender provide a complete
    // public T3 strict-JIT continuation without reading future draws or Prizes:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Core Item, search, Bench, attachment, retreat, Supporter, Tool, evolution, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K0/K1, dynamic DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1478
    return issue_1419_publicly_possible_count(Card::RegidragoV) >= 1 &&
        issue_1419_publicly_possible_count(Card::RegidragoVstar) >= 1 &&
        issue_1419_publicly_possible_count(Card::EarthenVessel) >= 1 &&
        issue_1419_publicly_possible_count(Card::Grass) >= 2 &&
        issue_1419_publicly_possible_count(Card::Fire) >= 1 &&
        issue_1419_possible_payload_count() >= 1;
  }

  bool issue_1478_play_t1_field_blower_direct_regi_route() {
    if (!issue_1478_t1_field_blower_direct_regi_route_available()) {
      return false;
    }

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::FieldBlower,
                           "Mysterious Treasure direct-Regidrago cost",
                           "R-MT-01; P-DCI-01; P-COMPRESS-01")) {
      throw std::logic_error("projected issue-1478 Field Blower cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");

    if (!move_deck_to_hand(Card::RegidragoV)) {
      const Card fallback = fallback_mysterious_target_after_search_started();
      const bool found_fallback = move_deck_to_hand(fallback);
      shuffle(state_.deck);
      trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
            found_fallback
                ? "Spent setup-dead Field Blower; Regidrago V was unavailable after inspection, so Mysterious Treasure searched " +
                      std::string(name(fallback)) + "."
                : "Spent setup-dead Field Blower; no Psychic or Dragon Pokemon remained after inspection.");
      return true;
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
          "Discarded setup-dead Field Blower and searched Regidrago V for the complete T3 route.");

    if (!bench_from_hand(Card::RegidragoV, false)) {
      throw std::logic_error("projected issue-1478 Regidrago V Bench action disappeared");
    }
    if (!remove_one(state_.hand, Card::Fire)) {
      throw std::logic_error("projected issue-1478 retreat Energy disappeared");
    }
    ++state_.active->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY",
          "Fire Energy manually to Active Oricorio for its Retreat Cost.");

    --state_.active->fire;
    state_.discard.push_back(Card::Fire);
    const auto promoted = std::find_if(
        state_.bench.rbegin(), state_.bench.rend(),
        [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn == state_.turn;
        });
    if (promoted == state_.bench.rend()) {
      throw std::logic_error("projected issue-1478 retreat target disappeared");
    }
    std::swap(*state_.active, *promoted);
    state_.retreat_used = true;
    trace("RETREAT", "R-GAME-RETREAT",
          "Paid Oricorio's one-Energy Retreat Cost and promoted Regidrago V.");
    return true;
  }

  bool issue_1476_t2_oricorio_first_route_available_after_search_started() const {
    if (!deck_seen_ || state_.turn != 1 || !scenario_.going_first ||
        scenario_.dci != DciProfile::StrictJit ||
        scenario_.locks != LockMode::None || scenario_.max_turn < 2 ||
        item_locked() || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn || bench_space() < 2 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::QuickBall) == 0 || hand_count(Card::LatiasEx) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 ||
        !ability_available_for_pokemon(Card::Oricorio) ||
        !ability_available_for_pokemon(Card::TapuLeleGX)) {
      return false;
    }

    int deck_payloads = 0;
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      deck_payloads += deck_count_after_search_started(card);
    }

    // Mysterious Treasure has already paid its cost and legally established K1.
    // The inspected deck can now select the shortest complete line: Oricorio puts
    // one Grass and one Fire Energy into hand, one attaches on T1, Quick Ball spends
    // route-inert Latias ex for Tapu Lele-GX, Wonder Tag banks Crispin, and T2
    // evolution plus the second manual attachment and Crispin complete GGF before
    // Blender. Crispin says "up to 2", so one remaining Energy of the missing type
    // is sufficient after Vital Dance; requiring two remaining copies overprotects
    // resources and rejects a legal complete route.
    // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core first-turn, Item, Bench, Ability, attachment, Supporter, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1476#issuecomment-5068202693
    return deck_count_after_search_started(Card::Oricorio) > 0 &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::Fire) >= 1 && deck_payloads >= 2;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (issue_1478_play_t1_field_blower_direct_regi_route()) return true;
    if (!issue_1476_redundant_burnet_route_available(
            Card::MysteriousTreasure)) {
      return play_mysterious_treasure_issue1476_original(permit_payload);
    }

    // Mysterious Treasure pays its printed cost before the first legal inspection.
    // Held Blender strictly covers Burnet's remaining deck-payload role, so Burnet
    // is legal dynamic-DCI fuel without reading the physical deck or Prize cards:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Core Item cost and search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Dynamic DCI and K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
    // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1476#issuecomment-5068202693
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(Card::ProfessorBurnet,
                           "Mysterious Treasure cost", "R-MT-01; P-DCI-01")) {
      throw std::logic_error("projected issue-1476 Burnet cost disappeared");
    }
    record_deck_search_knowledge("Mysterious Treasure");

    if (issue_1476_t2_oricorio_first_route_available_after_search_started()) {
      if (!move_deck_to_hand(Card::Oricorio)) {
        throw std::logic_error("projected issue-1476 Oricorio target disappeared");
      }
      shuffle(state_.deck);
      trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
            "Spent redundant Professor Burnet and searched Oricorio for the complete T2 route.");
      if (!bench_from_hand(Card::Oricorio, false)) {
        throw std::logic_error("projected issue-1476 Oricorio Bench action disappeared");
      }
      // Vital Dance searches up to two Basic Energy only when Oricorio is played
      // from hand to the Bench. Resolve the established helper here so the composite
      // route records the card-name action explicitly in its trace:
      // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
      // Core Bench and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      search_energy_to_hand(2, "R-ORICORIO-01", "VITAL DANCE");

      attach_manual();
      if (!state_.manual_energy_used ||
          state_.active->grass + state_.active->fire != 1) {
        throw std::logic_error("projected issue-1476 T1 manual attachment disappeared");
      }

      remove_one(state_.hand, Card::QuickBall);
      state_.discard.push_back(Card::QuickBall);
      if (!discard_from_hand(Card::LatiasEx, "Quick Ball cost",
                             "R-QB-01; P-DCI-01; P-COMPRESS-01")) {
        throw std::logic_error("projected issue-1476 Latias cost disappeared");
      }
      record_deck_search_knowledge("Quick Ball");
      if (!move_deck_to_hand(Card::TapuLeleGX)) {
        throw std::logic_error("projected issue-1476 Tapu target disappeared");
      }
      shuffle(state_.deck);
      trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-DCI-01; P-COMPRESS-01",
            "Discarded route-inert Latias ex and searched Tapu Lele-GX for the T2 Crispin finish.");

      if (!bench_from_hand(Card::TapuLeleGX, false)) {
        throw std::logic_error("projected issue-1476 Tapu Bench action disappeared");
      }
      record_deck_search_knowledge("Wonder Tag");
      if (!move_deck_to_hand(Card::Crispin)) {
        throw std::logic_error("projected issue-1476 Crispin target disappeared");
      }
      shuffle(state_.deck);
      trace("WONDER TAG", "R-TAPU-01; P-COMPRESS-01",
            "Searched and revealed Crispin for the complete T2 route.");
      return true;
    }

    const bool found_tapu = move_deck_to_hand(Card::TapuLeleGX);
    const Card fallback = found_tapu
        ? Card::TapuLeleGX
        : fallback_mysterious_target_after_search_started();
    const bool found_fallback = found_tapu || move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-DCI-01",
          found_fallback
              ? "Spent redundant Professor Burnet and searched " +
                    std::string(name(fallback)) + "."
              : "Spent redundant Professor Burnet; no Psychic or Dragon Pokemon remained after search.");

    if (found_tapu && !bench_from_hand(Card::TapuLeleGX, true)) {
      throw std::logic_error("projected issue-1476 Tapu Bench action disappeared");
    }
    return true;
  }

  bool play_quick_ball(const bool permit_payload) {
    // The complete issue-1476 T2 route resolves its K1 Quick Ball inside the
    // initiating Treasure sequence. Every other state retains the established
    // adaptive selector and all prior DCI boundaries:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1476#issuecomment-5068202693
    return play_quick_ball_issue1476_original(permit_payload);
  }
