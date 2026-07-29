bool issue_1845_proactive_tapu_attachment_available() const {
  // This decision uses only the current public state. Three visible Grass
  // preserve the two-Grass Apex Dragon reserve while the third pays Tapu's
  // printed one-Energy Retreat Cost. No later draw identity is consulted:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official attachment, retreat, and first-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Observable-information, lock, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-model https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1845
  return !secret_box_combo_enabled() &&
      scenario_.dci == DciProfile::StrictJit &&
      scenario_.locks == LockMode::FullCombined && scenario_.going_first &&
      state_.turn == 1 && scenario_.max_turn >= 5 &&
      !state_.manual_energy_used && !state_.retreat_used &&
      state_.active && state_.active->card == Card::TapuLeleGX &&
      state_.active->grass == 0 && state_.active->fire == 0 &&
      target_regi() == nullptr && bench_space() > 0 &&
      hand_count(Card::Grass) >= 3 && hand_count(Card::TateLiza) == 0 &&
      hand_count(Card::LatiasEx) == 0 && !in_play(Card::LatiasEx);
}

bool retreat_banked_tapu_to_regidrago() {
  if (state_.retreat_used || !state_.active ||
      state_.active->card != Card::TapuLeleGX ||
      state_.active->grass + state_.active->fire < 1 ||
      can_free_retreat_with_latias() ||
      (supporter_allowed() && hand_count(Card::TateLiza) > 0)) {
    return false;
  }

  Pokemon* target = target_regi();
  // Hold Tapu Active until Regidrago has evolved and the deterministic
  // two-Grass axis is complete. Retreating to Regidrago V one turn earlier
  // would force the simulator into a random Celestial Roar branch:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official retreat, evolution, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Earliest deterministic route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1845
  if (target == nullptr || target->card != Card::RegidragoVstar ||
      target->grass < 2 || target->fire != 0) {
    return false;
  }

  const Card payment = state_.active->grass > 0 ? Card::Grass : Card::Fire;
  if (payment == Card::Grass) {
    --state_.active->grass;
  } else {
    --state_.active->fire;
  }
  state_.discard.push_back(payment);
  std::swap(*state_.active, *target);
  state_.retreat_used = true;

  // Tapu's previously banked surplus Energy pays the printed one-Colorless
  // Retreat Cost once a live Regidrago line exists. Free Latias and Tate routes
  // remain preferred, and the target must already carry setup Energy:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Official retreat and promotion procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1845
  trace("RETREAT", "R-GAME-RETREAT; R-TAPU-01; P-DCI-01",
        "Paid Tapu Lele-GX's banked one-Energy Retreat Cost and promoted Regidrago.");
  return true;
}

bool attach_manual() {
  if (issue_1845_proactive_tapu_attachment_available()) {
    if (!remove_one(state_.hand, Card::Grass)) {
      throw std::logic_error("Issue-1845 surplus Grass disappeared");
    }
    ++state_.active->grass;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; R-TAPU-01; P-DCI-01",
          "Grass Energy manually to Tapu Lele-GX for a future paid Retreat.");
    return true;
  }
  if (issue_1772_t3_crispin_completion_available()) {
    // The banked K1 route needs Crispin to supply the first Grass before the
    // ordinary manual attachment supplies the second. Spending the attachment
    // first makes Crispin unable to complete GGF and lets Gladion preempt the
    // deterministic T3 package:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter and Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1772
    if (!play_crispin()) {
      throw std::logic_error("Issue-1772 projected Crispin completion disappeared");
    }
  }
  const bool issue_1552_public_t2_state =
      !secret_box_combo_enabled() &&
      scenario_.dci == DciProfile::StrictJit &&
      scenario_.locks == LockMode::None && scenario_.going_first &&
      state_.turn == 2 && deck_seen_ && prizes_known() &&
      !state_.manual_energy_used && supporter_allowed() &&
      state_.active && state_.active->card == Card::RegidragoV &&
      state_.active->entered_turn < state_.turn &&
      state_.active->grass == 1 && state_.active->fire == 0 &&
      bench_space() > 0 &&
      ability_available_for_pokemon(Card::TapuLeleGX) &&
      count_of(state_.discard, Card::EarthenVessel) > 0 &&
      count_of(state_.discard, Card::MysteriousTreasure) > 0 &&
      hand_count(Card::RegidragoVstar) > 0 &&
      hand_count(Card::QuickBall) > 0 && hand_count(Card::Fire) > 0 &&
      deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
      deck_count_after_search_started(Card::Crispin) > 0 &&
      deck_count_after_search_started(Card::Grass) > 0 &&
      deck_count_after_search_started(Card::Fire) > 0;
  const auto issue_1552_quick_ball_cost = issue_1552_public_t2_state
      ? choose_discard(true, true, true, Card::QuickBall)
      : std::optional<Card>{};
  const bool issue_1552_t2_vessel_finish =
      issue_1552_quick_ball_cost &&
      is_payload(*issue_1552_quick_ball_cost);
  if (issue_1552_t2_vessel_finish) {
    // The T1 Earthen Vessel search established K1 and attached the first Grass.
    // Before spending the T2 manual attachment, evolve, let Quick Ball discard a
    // current-turn Dragon payload, Bench Tapu Lele-GX, resolve Wonder Tag for
    // Crispin, and let Crispin attach the second Grass. The ordinary attachment
    // then supplies the held Fire and completes GGF on the earliest legal turn:
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR / GGF: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Evolution, Item, Bench, Ability, Supporter, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, DCI, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k1-after-a-legal-deck-or-prize-inspection https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1552
    if (!evolve_best_regi()) {
      throw std::logic_error(
          "projected issue-1552 T2 evolution disappeared");
    }
    if (!remove_one(state_.hand, Card::QuickBall)) {
      throw std::logic_error(
          "projected issue-1552 Quick Ball disappeared");
    }
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(
            *issue_1552_quick_ball_cost,
            "Quick Ball issue-1552 route cost",
            "R-QB-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error(
          "projected issue-1552 Dragon payload cost disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-1552 T2 route");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error(
          "projected issue-1552 Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM",
          "Searched a Basic Pokémon: Tapu Lele-GX.");
    if (!bench_from_hand(Card::TapuLeleGX, true)) {
      throw std::logic_error(
          "projected issue-1552 Tapu Lele-GX Bench play disappeared");
    }
    if (hand_count(Card::Crispin) == 0 || !play_crispin()) {
      throw std::logic_error(
          "projected issue-1552 Wonder Tag to Crispin route disappeared");
    }
  }
  if (late_steven_has_known_t3_compression_route() &&
      !state_.manual_energy_used && hand_count(Card::Fire) > 0) {
    for (Pokemon& pokemon : state_.bench) {
      if (pokemon.card != Card::RegidragoV ||
          pokemon.entered_turn >= state_.turn || pokemon.grass != 1 ||
          pokemon.fire != 0) {
        continue;
      }
      remove_one(state_.hand, Card::Fire);
      ++pokemon.fire;
      state_.manual_energy_used = true;
      // Preserve Grass for the next turn of the exact Steven compression route:
      // https://api.pokemontcg.io/v2/cards/sm7-145
      // https://api.pokemontcg.io/v2/cards/swsh12-136
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://github.com/FlareZ123/pokemon-sims/issues/869
      trace("ATTACH",
            "R-GAME-ENERGY; R-STEVEN-01; R-RVS-01; P-DCI-01",
            "Fire Energy manually to the prior-turn Regidrago V for the known T3 Steven route.");
      return true;
    }
  }
  const int supporters_in_hand = static_cast<int>(
      std::count_if(state_.hand.begin(), state_.hand.end(), is_supporter));
  const bool only_tate_supporter_held =
      supporters_in_hand == hand_count(Card::TateLiza);
  const bool active_can_evolve_now = state_.active &&
      state_.active->card == Card::RegidragoV &&
      state_.active->entered_turn < state_.turn &&
      hand_count(Card::RegidragoVstar) > 0;
  const bool blender_payload_line = has_live_blender_payload_line();
  const bool hand_payload_line = has_live_one_discard_hand_payload_line();

  // Tate & Liza promotes a Benched Pokémon. Brilliant Blender or a payable
  // one-discard Item can then establish the current-turn Dragon payload, while Apex
  // Dragon still requires GGF. Direct the once-per-turn attachment toward that
  // intended VSTAR before either resource is spent when Tate is the only held
  // Supporter and the Active V cannot evolve now:
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  const bool coordinate_tate_payload_line = !state_.manual_energy_used &&
      strict_payload_timing() && supporter_allowed() &&
      only_tate_supporter_held && hand_count(Card::TateLiza) > 0 &&
      (blender_payload_line || hand_payload_line) && need_active_vstar() &&
      !can_free_retreat_with_latias() && state_.active &&
      state_.active->card == Card::RegidragoV && !active_can_evolve_now;
  if (coordinate_tate_payload_line) {
    // Use the exact selector that Tate & Liza later uses for promotion. A separate
    // first-match attachment can change the selector's stable tie and strand GGF:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    Pokemon* target = best_benched_vstar_for_promotion();
    if (target != nullptr) {
      Card energy = Card::Grass;
      if (target->grass < 2 && hand_count(Card::Grass) > 0) {
        energy = Card::Grass;
      } else if (target->fire < 1 && hand_count(Card::Fire) > 0) {
        energy = Card::Fire;
      } else {
        return attach_manual_tate_blender_original();
      }

      remove_one(state_.hand, energy);
      if (energy == Card::Grass) {
        ++target->grass;
      } else {
        ++target->fire;
      }
      state_.manual_energy_used = true;
      trace("ATTACH", "R-GAME-ENERGY; R-TATE-01; R-RVS-01; P-DCI-01",
            std::string(name(energy)) +
                " manually to the Benched Regidrago VSTAR selected for Tate & Liza promotion.");
      return true;
    }
  }

  const bool attached = attach_manual_tate_blender_original();
  if (retreat_banked_tapu_to_regidrago()) return true;
  return attached;
}
