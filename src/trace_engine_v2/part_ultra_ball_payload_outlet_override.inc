  bool ultra_ball_one_discard_outlet_has_legal_target_after_payload(
      const Card outlet) const {
    // K0 preserves hidden-deck uncertainty. K1 removes the exact payload Ultra Ball
    // would select, then checks the remaining card classes for the promised Item:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    if (!deck_seen_) return true;

    std::optional<Card> planned_payload;
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::GoodraVstar, Card::DialgaGX}) {
      if (std::find(state_.deck.begin(), state_.deck.end(), payload) !=
          state_.deck.end()) {
        planned_payload = payload;
        break;
      }
    }
    if (!planned_payload) return false;

    bool removed_payload = false;
    for (const Card card : state_.deck) {
      if (!removed_payload && card == *planned_payload) {
        removed_payload = true;
        continue;
      }
      const bool legal_target =
          (outlet == Card::MysteriousTreasure && is_dragon_or_psychic(card)) ||
          (outlet == Card::QuickBall && is_basic(card)) ||
          (outlet == Card::EarthenVessel && is_energy(card));
      if (legal_target) return true;
    }
    return false;
  }

  bool play_ultra_ball(const bool permit_payload) {
    const bool can_jit_discard = permit_payload && can_play_payload_this_turn() &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    const bool direct_non_payload_target =
        (need_regi() && bench_space() > 0 && might_be_unseen(Card::RegidragoV)) ||
        (need_vstar() && hand_count(Card::RegidragoVstar) == 0 &&
         might_be_unseen(Card::RegidragoVstar)) ||
        (need_active_vstar() && bench_space() > 0 &&
         ability_available_for_pokemon(Card::LatiasEx) &&
         !in_play(Card::LatiasEx) && hand_count(Card::LatiasEx) == 0 &&
         might_be_unseen(Card::LatiasEx)) ||
        (need_energy() && hand_count(Card::Oricorio) == 0 &&
         !in_play(Card::Oricorio) && bench_space() > 0 &&
         needs_oricorio_connector() && might_be_unseen(Card::Oricorio)) ||
        (hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
         bench_space() > 0 && needs_tapu_connector() &&
         might_be_unseen(Card::TapuLeleGX));

    const bool evaluating_payload_search = permit_payload &&
        can_play_payload_this_turn() && need_payload() && !can_jit_discard &&
        !direct_non_payload_target;
    if (evaluating_payload_search) {
      // A held one-discard Item is a live strict-JIT continuation only when the
      // post-Ultra-Ball deck still contains a target for that Item. Serena remains
      // live because its discard mode does not search the deck:
      // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      // https://api.pokemontcg.io/v2/cards/sv4-163
      // https://api.pokemontcg.io/v2/cards/swsh12-164
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      const bool live_one_discard_outlet =
          (hand_count(Card::MysteriousTreasure) > 0 &&
           ultra_ball_one_discard_outlet_has_legal_target_after_payload(
               Card::MysteriousTreasure)) ||
          (hand_count(Card::QuickBall) > 0 &&
           ultra_ball_one_discard_outlet_has_legal_target_after_payload(
               Card::QuickBall)) ||
          (hand_count(Card::EarthenVessel) > 0 &&
           ultra_ball_one_discard_outlet_has_legal_target_after_payload(
               Card::EarthenVessel));
      const bool live_payload_outlet = live_one_discard_outlet ||
          second_ultra_ball_can_discard_fetched_payload() ||
          (supporter_allowed() && hand_count(Card::Serena) > 0);
      if (!live_payload_outlet) return false;
    }

    return play_ultra_ball_payload_outlet_original(permit_payload);
  }
