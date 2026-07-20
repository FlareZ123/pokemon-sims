// This fragment is composed inside Engine by part_014c.inc. The later top-level
// compatibility include is intentionally inert after this first expansion:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc#L168-L179
// The following part_015.inc remains the continuation of the free scenario lookup:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_015.inc#L1-L4
#ifndef POKEMON_SIMS_PART_014C_LATIAS_BENCH_OVERRIDE_INCLUDED
#define POKEMON_SIMS_PART_014C_LATIAS_BENCH_OVERRIDE_INCLUDED

// part_014c.inc composes this fragment while Engine remains open. Replace the
// macro-renamed legacy Field Blower policy at the same valid member boundary:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c.inc#L168-L179
// https://github.com/FlareZ123/pokemon-sims/issues/791
#undef play_field_blower
#include "part_014c_field_blower_override.inc"

  bool held_item_can_find_pending_vstar() {
    if (item_locked() || !might_be_unseen(Card::RegidragoVstar)) return false;

    // A held direct Pokémon-search Item can make Regidrago VSTAR available later in
    // this turn. Pokémon Communication is live only when its exact exchange plan
    // returns a safe held Pokémon and selects the pending VSTAR:
    // Evolution Incense: https://api.pokemontcg.io/v2/cards/swsh1-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/687
    // https://github.com/FlareZ123/pokemon-sims/issues/701
    if (hand_count(Card::EvolutionIncense) > 0) return true;
    if (hand_count(Card::PokemonCommunication) > 0) {
      const auto plan = pokemon_communication_plan(false);
      if (plan && plan->target == Card::RegidragoVstar) return true;
    }

    const bool permit_payload_cost = can_play_payload_this_turn();
    if (hand_count(Card::MysteriousTreasure) > 0 &&
        choose_discard(permit_payload_cost, true, true,
                       Card::MysteriousTreasure).has_value()) {
      return true;
    }
    if (hand_count(Card::UltraBall) == 0) return false;

    const std::vector<Card> original_hand = state_.hand;
    const auto first = choose_discard(permit_payload_cost, true, true, Card::UltraBall);
    if (!first || !remove_one(state_.hand, *first)) {
      state_.hand = original_hand;
      return false;
    }
    const auto second = choose_discard(
        can_play_payload_this_turn() && !is_payload(*first), true, true, Card::UltraBall);
    state_.hand = original_hand;
    return second.has_value();
  }

  bool bench_latias_for_pending_vstar_promotion() {
    if (bench_space() != 1 || hand_count(Card::LatiasEx) == 0 ||
        in_play(Card::LatiasEx) || !ability_available_for_pokemon(Card::LatiasEx) ||
        state_.retreat_used || !state_.active || !is_basic(state_.active->card) ||
        state_.active->card == Card::RegidragoV ||
        (hand_count(Card::RegidragoVstar) == 0 &&
         !held_item_can_find_pending_vstar())) {
      return false;
    }

    // Latias ex gives a Basic Active no Retreat Cost. Preserve the final Bench slot
    // when a prior-turn Benched Regidrago V can evolve now, then use Skyliner to
    // promote that VSTAR. Tate & Liza is an alternate switch route and does not need
    // this scarce Bench slot:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://www.pokemon.com/us/pokemon-tcg/rules
    const bool prior_turn_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
                 pokemon.entered_turn < state_.turn;
        });
    const bool tate_switch_available = supporter_allowed() &&
        hand_count(Card::TateLiza) > 0;
    if (!prior_turn_benched_regi || tate_switch_available) return false;

    return bench_from_hand(Card::LatiasEx, false);
  }

  bool pay_tapu_retreat_to_ready_benched_vstar() {
    if (!need_active_vstar() || need_energy() || need_payload() ||
        state_.retreat_used || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX) {
      return false;
    }

    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || target->grass < 2 || target->fire < 1) return false;

    const Card payment = hand_count(Card::Grass) > 0 ? Card::Grass : Card::Fire;
    if (hand_count(payment) == 0 || !remove_one(state_.hand, payment)) return false;

    // Tapu Lele-GX has a one-Colorless Retreat Cost, so either modeled Basic
    // Energy may be attached for the turn and immediately discarded to promote an
    // already-GGF Regidrago VSTAR. Restrict this route to states where Energy and
    // payload axes are already complete, preserving the attachment for setup when
    // the Benched VSTAR still needs it:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/802
    if (payment == Card::Grass) {
      ++state_.active->grass;
    } else {
      ++state_.active->fire;
    }
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY",
          std::string(name(payment)) +
              " manually to Tapu Lele-GX for its Retreat Cost.");

    if (payment == Card::Grass) {
      --state_.active->grass;
    } else {
      --state_.active->fire;
    }
    state_.discard.push_back(payment);
    std::swap(*state_.active, *target);
    state_.retreat_used = true;
    trace("RETREAT", "R-GAME-RETREAT",
          "Paid Tapu Lele-GX's one-Energy Retreat Cost and promoted the ready "
          "Benched Regidrago VSTAR.");
    return true;
  }

  bool resolve_final_promotion_and_attachment() {
    // When the selected Benched VSTAR already pays GGF, Skyliner promotes it for
    // zero before the #802 paid Tapu fallback can spend a Basic Energy. An
    // Energy-incomplete VSTAR still receives the manual attachment before the free
    // retreat, preserving the legal Earthen Vessel route:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
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

  bool complete_late_steven_vstar_vessel_continuation() {
    if (scenario_.dci != DciProfile::StrictJit || item_locked() ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass < 2 || state_.active->fire < 1 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::EarthenVessel) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        (deck_count_after_search_started(Card::Grass) == 0 &&
         deck_count_after_search_started(Card::Fire) == 0)) {
      return false;
    }

    // Evolve before resolving the held Vessel so the Dragon discard is permitted by
    // strict-JIT on the same turn that Regidrago VSTAR becomes attack-ready. Vessel
    // still performs its printed Basic Energy search after paying that discard cost:
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Core evolution and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-ready and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1017
    evolve_best_regi();
    if (!active_is_vstar() || !play_earthen_vessel(true)) {
      throw std::logic_error("Known late-Steven VSTAR-Vessel continuation failed");
    }
    trace("POLICY", "P-AXIS-01", "End: " + state_line());
    return true;
  }

  void run_turn() {
    trace("POLICY", "P-AXIS-01", "Start: " + state_line());
    if (complete_late_steven_vstar_vessel_continuation()) return;
    play_chaotic_swell();
    play_field_blower();
    // Resolve the proven K1 Steven route before generic Items can consume Quick Ball
    // and erase the public-state predicate. Steven searches any three cards and ends
    // the turn, so attaching Fire and evolving the prior-turn Regidrago V first banks
    // the exact Latias ex, Mysterious Treasure, and Mega Dragonite ex continuation:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    if (late_steven_has_known_t3_compression_route()) {
      attach_manual();
      evolve_best_regi();
      choose_supporter();
      if (state_.turn_ended) {
        trace("TURN END", "R-STEVEN-01", state_line());
        return;
      }
    }
    // The banked going-first route must resolve before generic actions can consume
    // Blender or alter its K1 continuation. Attach the second public Energy, then let
    // Steven search VSTAR, the direct third Grass, and Latias. Steven ends T2, while
    // the T3 Supporter slot remains unused for any later discrete-value action:
    // https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/946
    if (banked_steven_has_known_t3_blender_route()) {
      attach_manual();
      if (play_banked_steven_for_known_t3_blender_route()) {
        trace("TURN END", "R-STEVEN-01", state_line());
        return;
      }
    }
    // A redundant Regidrago V must not consume the final Bench slot before the
    // immediately live Latias ex promotion route:
    // https://api.pokemontcg.io/v2/cards/sv8-76
    // https://www.pokemon.com/us/pokemon-tcg/rules
    bench_latias_for_pending_vstar_promotion();
    play_basics_from_hand();
    attach_opening_oricorio_for_known_t4_fss_route();
    if (banked_steven_has_known_t4_fss_route() &&
        play_banked_steven_for_known_t4_fss_route()) {
      trace("TURN END", "R-STEVEN-01", state_line());
      return;
    }
    // Preserve the exact #869 Mysterious Treasure payload outlet before a generic
    // Supporter route can shuffle it away:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://github.com/FlareZ123/pokemon-sims/issues/869
    play_known_steven_t3_payload_outlet();
    play_items_until_stable(!strict_payload_timing());
    attach_live_fss();
    use_fss();
    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());

    attach_manual();
    // A manual attachment can complete GGF and thereby make a held Mysterious
    // Treasure payload discard legal under strict JIT. Reopen Items and evolution
    // only for that proven same-turn route. Earlier payload banking remains rejected
    // when the selected Regidrago V still lacks part of Apex Dragon's GGF cost:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/1032
    const Pokemon* post_attach_target = target_regi();
    const bool post_attach_treasure_vstar_route = strict_payload_timing() &&
        !item_locked() && post_attach_target != nullptr &&
        post_attach_target->card == Card::RegidragoV &&
        post_attach_target->entered_turn < state_.turn &&
        post_attach_target->grass >= 2 && post_attach_target->fire >= 1 &&
        hand_count(Card::RegidragoVstar) == 0 &&
        might_be_unseen(Card::RegidragoVstar) &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) &&
        choose_discard(true, true, true, Card::MysteriousTreasure).has_value();
    const bool post_attach_treasure_latias_route =
        post_attach_treasure_latias_route_available();
    if (post_attach_treasure_vstar_route || post_attach_treasure_latias_route) {
      play_items_until_stable(true);
      evolve_best_regi();
      if (post_attach_treasure_latias_route) {
        play_basics_from_hand();
        resolve_final_promotion_and_attachment();
      }
    }
    // Steven's Resolve ends the turn, so attach a live Powerglass before choosing
    // that Supporter: https://api.pokemontcg.io/v2/cards/sm7-145
    if (should_play_steven()) attach_powerglass();
    choose_supporter();
    if (state_.turn_ended) {
      trace("TURN END", "R-STEVEN-01 or R-RV-01", state_line());
      return;
    }

    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    attach_live_fss();
    use_fss();
    play_chaotic_swell();
    play_field_blower();
    play_items_until_stable(!strict_payload_timing());
    attach_manual();
    evolve_best_regi();
    play_basics_from_hand();
    play_items_until_stable(true);
    // A strict-JIT Item route may now discard an in-hand Dragon payload and search
    // Regidrago VSTAR through Mysterious Treasure (https://api.pokemontcg.io/v2/cards/sm6-113).
    // Retry evolution so a Regidrago V that entered play on a prior turn can evolve
    // before this turn's ready check under the core evolution procedure (https://www.pokemon.com/us/pokemon-tcg/rules).
    evolve_best_regi();
    if (late_steven_active_vstar_crispin_treasure_route_available() &&
        play_late_steven_active_vstar_crispin_treasure_route()) {
      // Steven's Resolve ends the turn immediately after the two-card route search.
      // https://api.pokemontcg.io/v2/cards/sm7-145
      // https://github.com/FlareZ123/pokemon-sims/issues/1098
      trace("TURN END", "R-STEVEN-01", state_line());
      return;
    }
    // A late Item search can put Tapu Lele-GX into hand, and Bench play can then
    // fetch a new Supporter with Wonder Tag. Reconsider that unused Supporter slot
    // before the ready check: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Professor Burnet can discard a current-turn deck payload after this chain:
    // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    if (!state_.supporter_used && setup_axis_missing()) choose_supporter();
    if (use_legacy_star()) {
      // Core procedure source: https://www.pokemon.com/us/pokemon-tcg/rules
      // Recovered Items may be played later in the same turn.
      play_chaotic_swell();
      play_field_blower();
      play_basics_from_hand();
      play_items_until_stable(true);
      // Legacy Star can return a Supporter from discard, including Crispin:
      // https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/sv7-133
      // A player may use one available Supporter during the turn before attacking:
      // https://www.pokemon.com/us/pokemon-tcg/rules
      if (!state_.supporter_used && setup_axis_missing()) choose_supporter();
    }
    resolve_final_promotion_and_attachment();
    if (need_active_vstar() && !state_.supporter_used) play_tate_switch();
    // Attach only after the final modeled switch decision, while the intended holder
    // is known to remain Active for Powerglass's end-of-turn condition:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    attach_powerglass();
    use_celestial_roar();
    trace("POLICY", "P-AXIS-01", "End: " + state_line());
  }

#endif
