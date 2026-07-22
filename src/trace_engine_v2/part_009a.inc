    return Card::RegidragoV;
  }

  Card fallback_ultra_ball_target_after_search_started() const {
    // Ultra Ball searches any Pokémon, so the K1 fallback covers the complete
    // modeled universe, including the Appletun and Pineco package:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://github.com/FlareZ123/pokemon-sims/issues/1254
    for (const Card target : {Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                              Card::DialgaGX, Card::MawileGX, Card::Dipplin, Card::Appletun,
                              Card::Pineco, Card::ForretressEx}) {
      if (deck_count_after_search_started(target) > 0) return target;
    }
    return Card::RegidragoV;
  }

  std::optional<Card> issue_1221_burnet_treasure_vstar_cost() const {
    // After Professor Burnet establishes the strict current-turn payload, one
    // setup-dead Supporter may pay Mysterious Treasure only when the searched
    // Regidrago VSTAR immediately completes an evolution-legal Active GGF route:
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, discard, search, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Strict-JIT and DCI specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1221
    const bool burnet_resolved_this_turn = state_.supporter_used &&
        count_of(state_.discard, Card::ProfessorBurnet) > 0 &&
        std::any_of(state_.discarded_this_turn.begin(),
                    state_.discarded_this_turn.end(), is_payload);
    const bool active_route_complete = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass >= 2 && state_.active->fire >= 1 &&
        !item_locked() && hand_count(Card::MysteriousTreasure) > 0 &&
        might_be_unseen(Card::RegidragoVstar);
    if (!burnet_resolved_this_turn || !active_route_complete) return std::nullopt;

    // These Supporters have no remaining setup role after Burnet has consumed the
    // Supporter play and every non-VSTAR axis is complete. Movement, recovery,
    // Energy, ACE SPEC, Pokémon, and Tool resources remain protected.
    for (const Card card : {Card::Channeler, Card::ErikasInvitation}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_1221_burnet_then_treasure_route_available() const {
    if (!supporter_allowed() || item_locked() ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        !payload_might_be_in_deck() ||
        !might_be_unseen(Card::RegidragoVstar) || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass < 2 || state_.active->fire < 1) {
      return false;
    }
    return hand_count(Card::Channeler) > 0 ||
           hand_count(Card::ErikasInvitation) > 0;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (item_locked() || hand_count(Card::MysteriousTreasure) == 0) return false;
    // is_payload is the A/S readiness set. Dipplin is a legal Dragon search
    // target, while maintainer policy keeps it out of payload-discard routing:
    // https://api.pokemontcg.io/v2/cards/sv6-127
    // A strict-JIT Treasure payload cost must share the turn in which the Active
    // Regidrago VSTAR can finish Apex Dragon's GGF Energy cost. Preserve the
    // one-discard outlet when the Energy axis cannot complete until a later turn:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/951
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
                                  blender_energy_axis_can_finish_this_turn() &&
                                  std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    // Mysterious Treasure searches for a Psychic or Dragon Pokémon. At K0, a target
    // remains plausible only when a fixed-list copy is not accounted for in public
    // zones. At K1, the inspected deck is checked exactly before paying the cost:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // https://github.com/FlareZ123/pokemon-sims/issues/693
    const bool search_has_target = deck_seen_
        ? std::any_of(state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic)
        : std::any_of(recipe_.begin(), recipe_.end(), [this](const auto& entry) {
            return is_dragon_or_psychic(entry.first) && might_be_unseen(entry.first);
          });
    if (!search_has_target) return false;
    // Mysterious Treasure searches only the deck after paying its discard cost.
    // K0 keeps fixed-list targets plausible; K1 requires the inspected deck to
    // contain the desired Psychic or Dragon Pokémon before spending both cards:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    const bool want_regi = need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV);
    const bool want_vstar = need_vstar() && hand_count(Card::RegidragoVstar) == 0 &&
                            might_be_unseen(Card::RegidragoVstar);
    const bool want_latias = need_active_vstar() && bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
                             !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
                             might_be_unseen(Card::LatiasEx);
    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) && bench_space() > 0 &&
                           needs_tapu_connector() && might_be_unseen(Card::TapuLeleGX);
    const bool want_oricorio = hand_count(Card::Oricorio) == 0 && !in_play(Card::Oricorio) && bench_space() > 0 &&
                                  needs_oricorio_connector() && might_be_unseen(Card::Oricorio);
    // Treasure must pay its discard before the fetched payload reaches hand. A later
    // Ultra Ball route is live only when its own two-card cost remains payable:
    // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Do not fetch a second Dragon when Serena can already discard the permitted
    // payload held in hand. The direct route uses fewer cards and preserves the
    // one-shot search connector:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-164
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/889
    const bool want_payload = need_payload() && !can_jit_discard &&
                              !has_live_serena_hand_payload_line() &&
                              has_payable_payload_outlet_after_costed_search(Card::MysteriousTreasure);
    if (!can_jit_discard && !want_regi && !want_vstar && !want_latias && !want_tapu && !want_oricorio && !want_payload) return false;
    // The Mysterious Treasure being played cannot discard itself. A distinct held
    // Ultra Ball remains a legal card from hand for Treasure's discard cost, even
    // when that Ultra Ball is not itself payable because it needs two other cards:
    // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    std::optional<Card> cost =
        choose_discard(can_jit_discard, true, true, Card::MysteriousTreasure);
    if (!cost && want_regi && issue_1304_t1_treasure_erika_cost_available()) {
      // Erika's Invitation cannot advance this single-player setup state, while
      // Mysterious Treasure establishes the prior-turn Regidrago V required by
      // the complete T2 Forest Seal Stone plus Exploding Energy route:
      // https://api.pokemontcg.io/v2/cards/sv3pt5-160
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh12-135
      // https://github.com/FlareZ123/pokemon-sims/issues/1304
      cost = Card::ErikasInvitation;
    }
    if (!cost && want_vstar) {
      cost = issue_1221_burnet_treasure_vstar_cost();
    }
    if (!cost && want_vstar && can_spend_redundant_payload_for_locked_vstar_search()) {
      // Mysterious Treasure may discard any one card. This fallback spends one
      // redundant payload only after every ordinary strict-DCI cost has failed:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/issues/925
      cost = choose_discard(false, true, true, Card::MysteriousTreasure, true);
    }
    if (!cost) return false;

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(*cost, "Mysterious Treasure cost", "R-MT-01");

    // Card text source: https://api.pokemontcg.io/v2/cards/sm6-113
    // The deck is now being legally searched. Select a legal target after inspecting it.
    record_deck_search_knowledge("Mysterious Treasure");
    // A proved payable Dragon-payload route is checked before the pre-search Tapu
    // estimate. The direct current-turn payload completes Apex Dragon's missing axis,
    // while Wonder Tag is only an indirect connector that may become stale at K1:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Appletun sv8-140 is a Dragon Pokémon and modeled Apex Dragon payload, so it
    // belongs in the preferred payload search sequence:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1253
    const std::array<Card, 10> targets{Card::RegidragoV, Card::RegidragoVstar, Card::LatiasEx,
                                       Card::MegaDragonite, Card::Dragapult, Card::GoodraVstar,
                                       Card::DialgaGX, Card::Appletun, Card::TapuLeleGX,
                                       Card::Oricorio};
    for (const Card target : targets) {
      const bool desired = (target == Card::RegidragoV && want_regi) ||
                           (target == Card::RegidragoVstar && want_vstar) ||
                           (target == Card::LatiasEx && want_latias) ||
                           (target == Card::TapuLeleGX && want_tapu) ||
                           (target == Card::Oricorio && want_oricorio) ||
                           (want_payload && is_payload(target));
      if (desired && move_deck_to_hand(target)) {
        shuffle(state_.deck);
        trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM", "Searched a Psychic or Dragon Pokémon: " + std::string(name(target)) + ".");
        return true;
      }
    }
    // Once this search proves the needed Regidrago is prized, Tapu Lele-GX can be
    // searched and Benched to find Gladion with Wonder Tag in the same turn:
    // https://api.pokemontcg.io/v2/cards/sm2-60 https://api.pokemontcg.io/v2/cards/sm4-95
    const bool prized_regi_axis = deck_count_after_search_started(Card::Gladion) > 0 &&
        ((need_regi() && deck_count_after_search_started(Card::RegidragoV) == 0 &&
          prize_count_after_reveal(Card::RegidragoV) > 0) ||
         (need_vstar() && deck_count_after_search_started(Card::RegidragoVstar) == 0 &&
          prize_count_after_reveal(Card::RegidragoVstar) > 0));
    const bool found_tapu_for_gladion = prized_regi_axis && bench_space() > 0 &&
        hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
        move_deck_to_hand(Card::TapuLeleGX);
    const Card fallback = found_tapu_for_gladion ? Card::TapuLeleGX : fallback_mysterious_target_after_search_started();
    const bool found_fallback = found_tapu_for_gladion || move_deck_to_hand(fallback);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM", found_fallback ? "No preferred target remained; legally searched " + std::string(name(fallback)) + "." : "No Psychic or Dragon Pokémon remained after search.");
