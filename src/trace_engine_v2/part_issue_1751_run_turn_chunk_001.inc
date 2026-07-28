  bool resolve_final_promotion_and_attachment() {
    // When the selected Benched VSTAR already pays GGF, Skyliner promotes it for
    // zero before the #802 paid Tapu fallback can spend a Basic Energy. An
    // Energy-incomplete VSTAR still receives the manual attachment before the free
    // retreat, preserving the legal Earthen Vessel route:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/sm2-60
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/905
    if (pay_oricorio_retreat_for_known_t4_fss_route()) return true;
    if (benched_vstar_promotion_ready() &&
        retreat_to_benched_vstar_with_latias()) {
      return true;
    }
    if (pay_tapu_retreat_to_ready_benched_vstar()) return true;
    attach_manual();
    return retreat_to_benched_vstar_with_latias();
  }

  bool attach_opening_oricorio_for_known_t4_fss_route() {
    if (!issue_1022_banked_route_ || state_.turn != 1 ||
        !scenario_.going_first || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::Oricorio || hand_count(Card::Grass) == 0) {
      return false;
    }

    remove_one(state_.hand, Card::Grass);
    ++state_.active->grass;
    state_.manual_energy_used = true;
    // Oricorio has a one-Colorless Retreat Cost. Reserving the opening Grass on the
    // Active pays that future retreat without consuming Regidrago V's T3 and T4
    // attachment windows:
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Core Energy and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1022
    trace("ATTACH", "R-GAME-ENERGY",
          "Grass Energy manually to Oricorio for its future Retreat Cost.");
    return true;
  }

  bool pay_oricorio_retreat_for_known_t4_fss_route() {
    if (!issue_1022_banked_route_ || state_.turn < 4 || state_.retreat_used ||
        !state_.active || state_.active->card != Card::Oricorio ||
        state_.active->grass + state_.active->fire < 1 || need_energy() ||
        need_vstar() || need_payload()) {
      return false;
    }
    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || target->grass < 2 || target->fire < 1) return false;

    const Card payment = state_.active->grass > 0 ? Card::Grass : Card::Fire;
    if (payment == Card::Grass) {
      --state_.active->grass;
    } else {
      --state_.active->fire;
    }
    state_.discard.push_back(payment);
    std::swap(*state_.active, *target);
    state_.retreat_used = true;
    issue_1022_banked_route_ = false;
    // Pay Oricorio's printed one-Energy Retreat Cost and promote the completed
    // Regidrago VSTAR after Star Alchemy and Brilliant Blender resolve:
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1022
    trace("RETREAT", "R-GAME-RETREAT",
          "Paid Oricorio's one-Energy Retreat Cost and promoted Regidrago VSTAR.");
    return true;
  }

  bool post_attach_treasure_latias_route_available() {
    const Pokemon* target = target_regi();
    if (!strict_payload_timing() || item_locked() || target == nullptr ||
        target->card != Card::RegidragoVstar || target->grass < 2 ||
        target->fire < 1 || !need_active_vstar() || state_.retreat_used ||
        !state_.active || !is_basic(state_.active->card) || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) || in_play(Card::LatiasEx) ||
        hand_count(Card::LatiasEx) > 0 || !might_be_unseen(Card::LatiasEx) ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // A manual attachment can complete GGF on the prior-turn Regidrago VSTAR.
    // Mysterious Treasure may then discard the held Dragon payload, search Latias
    // ex, and Skyliner may immediately promote that VSTAR before the unused
    // Supporter slot is considered. This is the shortest observable complete route:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1145
    return choose_discard(true, true, true, Card::MysteriousTreasure).has_value();
  }

  bool late_supporter_quick_ball_latias_completion_available() const {
    if (!state_.supporter_used || state_.turn_ended || item_locked() ||
        hand_count(Card::QuickBall) == 0 || !need_active_vstar() ||
        need_energy() || !benched_vstar_promotion_ready() || state_.retreat_used ||
        !state_.active || !is_basic(state_.active->card) || bench_space() == 0 ||
        !ability_available_for_pokemon(Card::LatiasEx) || in_play(Card::LatiasEx) ||
        hand_count(Card::LatiasEx) > 0 || !might_be_unseen(Card::LatiasEx)) {
      return false;
    }

    const bool payload_axis_completable = !need_payload() ||
        (hand_count(Card::BrilliantBlender) > 0 && payload_might_be_in_deck() &&
         can_play_payload_this_turn());
    if (!payload_axis_completable) return false;

    // A late Arven or Wonder Tag Supporter may make Quick Ball newly live after the
    // turn's previous Item fixed point. Replay Items only when Quick Ball already
    // has a legal ordinary cost, Latias ex is searchable, Skyliner solves the final
    // Active axis, and any remaining payload is covered by held Brilliant Blender:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, discard, Bench, Ability, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
