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

bool issue_1912_post_attachment_blender_available() const {
  // A legal manual attachment does not end the action phase. When it completes
  // the Active Regidrago VSTAR's GGF cost, the already-held Brilliant Blender
  // remains playable and can establish the required current-turn Dragon payload:
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Energy attachment, Item, search, discard, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, current-turn JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1912
  return strict_payload_timing() && prizes_known() && !item_locked() &&
      state_.active && state_.active->card == Card::RegidragoVstar &&
      pays_apex_energy_cost(*state_.active) &&
      hand_count(Card::BrilliantBlender) > 0 && need_payload() &&
      payload_might_be_in_deck();
}

bool issue_1912_replay_blender_after_attachment() {
  if (!issue_1912_post_attachment_blender_available()) return false;
  if (supporter_allowed() && hand_count(Card::TateLiza) > 0 && trace_ != nullptr) {
    // Executing the held non-Supporter finish still means Tate & Liza is being
    // preserved from shuffling that deterministic route away:
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Confirmed held-route policy: https://github.com/FlareZ123/pokemon-sims/issues/1057
    // Confirmed replay bug: https://github.com/FlareZ123/pokemon-sims/issues/1912
    trace_->add_policy_once(
        state_, "issue-1057-held-non-supporter-route", state_.turn,
        "HOLD SUPPORTER", "R-TATE-01; P-KNOWLEDGE-01; P-COMPRESS-01",
        "Retained Tate & Liza because held non-Supporter cards already complete the current-turn ready route.");
  }
  issue_1912_post_attachment_blender_replay_ = true;
  const bool played = play_brilliant_blender();
  issue_1912_post_attachment_blender_replay_ = false;
  return played;
}

bool issue_2158_paid_oricorio_retreat_burnet_available() const {
  if (!strict_payload_timing() || !prizes_known() || !supporter_allowed() ||
      state_.supporter_used || state_.manual_energy_used || state_.retreat_used ||
      state_.turn > scenario_.max_turn || !state_.active ||
      state_.active->card != Card::Oricorio || state_.active->grass != 0 ||
      state_.active->fire != 0 || hand_count(Card::ProfessorBurnet) == 0 ||
      (hand_count(Card::Grass) == 0 && hand_count(Card::Fire) == 0) ||
      !need_active_vstar() || need_energy() || !need_payload() ||
      can_free_retreat_with_latias()) {
    return false;
  }

  // Project only the public K1 state. The held Basic Energy pays Oricorio's
  // one-Colorless Retreat Cost, promotes the already-GGF Benched Regidrago VSTAR,
  // and preserves the unused Supporter action for Professor Burnet to discard a
  // Dragon payload during the same turn:
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Latias ex / superior free-retreat route: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official attachment, Retreat, Supporter, search, and discard procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1, current-turn JIT, DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Reclaimed confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2158
  std::mt19937_64 shadow_rng = rng_;
  Engine projected(scenario_, recipe_, shadow_rng);
  projected.state_ = state_;
  projected.deck_seen_ = deck_seen_;
  projected.prizes_revealed_ = prizes_revealed_;
  projected.trace_ = nullptr;

  Pokemon* target = projected.best_benched_vstar_for_promotion();
  if (target == nullptr || target->grass < 2 || target->fire < 1) return false;

  const Card payment = projected.hand_count(Card::Grass) > 0
      ? Card::Grass
      : Card::Fire;
  if (!remove_one(projected.state_.hand, payment)) return false;
  projected.state_.manual_energy_used = true;
  projected.state_.discard.push_back(payment);
  std::swap(*projected.state_.active, *target);
  projected.state_.retreat_used = true;
  return projected.professor_burnet_has_live_ready_turn_route();
}

bool play_issue_2158_paid_oricorio_retreat_burnet() {
  if (!issue_2158_paid_oricorio_retreat_burnet_available()) return false;

  Pokemon* target = best_benched_vstar_for_promotion();
  if (target == nullptr || target->grass < 2 || target->fire < 1) {
    throw std::logic_error("Issue-2158 projected GGF promotion target disappeared");
  }
  const Card payment = hand_count(Card::Grass) > 0 ? Card::Grass : Card::Fire;
  if (!remove_one(state_.hand, payment)) {
    throw std::logic_error("Issue-2158 projected Retreat Energy disappeared");
  }

  // Attachment and Retreat are separate legal actions. Record the temporary
  // Energy on Oricorio, then discard it to pay the printed one-Colorless cost:
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Official attachment and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Reclaimed confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2158
  if (payment == Card::Grass) {
    ++state_.active->grass;
  } else {
    ++state_.active->fire;
  }
  state_.manual_energy_used = true;
  trace("ATTACH", "R-GAME-ENERGY; R-ORICORIO-01; P-COMPRESS-01",
        std::string(name(payment)) +
            " manually to Oricorio for its immediate Retreat Cost.");

  if (payment == Card::Grass) {
    --state_.active->grass;
  } else {
    --state_.active->fire;
  }
  state_.discard.push_back(payment);
  std::swap(*state_.active, *target);
  state_.retreat_used = true;
  trace("RETREAT", "R-GAME-RETREAT; R-ORICORIO-01; P-COMPRESS-01",
        "Paid Oricorio's one-Energy Retreat Cost and promoted the GGF Regidrago VSTAR before Professor Burnet.");
  return true;
}

bool attach_manual() {
  if (play_issue_2158_paid_oricorio_retreat_burnet()) return true;
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
  if (!state_.manual_energy_used) {
    Pokemon* dde_target = target_regi();
    if (dde_target != nullptr) {
      const auto preferred = preferred_manual_energy_from_hand(*dde_target);
      if (preferred && *preferred == Card::DoubleDragonEnergy) {
        // DDE preempts older Basic-only attachment packages only when the
        // shared selector proves strictly greater current-turn Energy
        // progress. Equal finishes still preserve DDE:
        // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // https://github.com/FlareZ123/pokemon-sims/issues/2238
        const bool attached = attach_manual_tate_blender_original();
        if (attached) issue_1912_replay_blender_after_attachment();
        return attached;
      }
    }
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
  // Full Prize inspection establishes the same K1 state as a resolved deck
  // search, so the route must depend on known zones rather than provenance:
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Correction precedent: https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1932
  const bool issue_1552_public_t2_state =
      !secret_box_combo_enabled() &&
      scenario_.dci == DciProfile::StrictJit &&
      scenario_.locks == LockMode::None && scenario_.going_first &&
      state_.turn == 2 && prizes_known() &&
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
    // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1552 https://github.com/FlareZ123/pokemon-sims/issues/1932
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
      if (pays_apex_energy_cost(*target)) {
        // Preserve the once-per-turn manual attachment when the selected VSTAR
        // already pays Apex through DDE plus a Basic Energy.
        // DDE: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
        // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
        // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2421
        return false;
      }
      const auto preferred = preferred_manual_energy_from_hand(*target);
      if (!preferred.has_value()) {
        return attach_manual_tate_blender_original();
      }
      const Card energy = *preferred;
      if (!remove_one(state_.hand, energy) || !attach_energy_card(*target, energy)) {
        throw std::logic_error("Tate promotion Energy projection disappeared");
      }
      state_.manual_energy_used = true;
      trace("ATTACH", "R-GAME-ENERGY; R-TATE-01; R-RVS-01; P-DCI-01",
            std::string(name(energy)) +
                " manually to the Benched Regidrago VSTAR selected for Tate & Liza promotion.");
      return true;
    }
  }

  const Pokemon* before_attachment = target_regi();
  const bool attachment_can_complete_ggf =
      !state_.manual_energy_used && before_attachment != nullptr &&
      !pays_apex_energy_cost(*before_attachment);
  const bool attached = attach_manual_tate_blender_original();
  if (attached && attachment_can_complete_ggf) {
    // Reopen only the deterministic held Blender action unlocked by this
    // attachment. This preserves unrelated pre-attachment Item ordering:
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1912
    issue_1912_replay_blender_after_attachment();
  }
  if (retreat_banked_tapu_to_regidrago()) return true;
  return attached;
}