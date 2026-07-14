  bool attach_manual() {
    const int supporters_in_hand = static_cast<int>(
        std::count_if(state_.hand.begin(), state_.hand.end(), is_supporter));
    const bool only_tate_supporter_held =
        supporters_in_hand == hand_count(Card::TateLiza);
    const bool active_can_evolve_now = state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        hand_count(Card::RegidragoVstar) > 0;

    // Tate & Liza promotes a Benched Pokémon, Brilliant Blender supplies a
    // current-turn Dragon payload, and Apex Dragon still requires GGF. Direct the
    // once-per-turn attachment toward that intended VSTAR before either resource is
    // spent when Tate is the only held Supporter and the Active V cannot evolve now:
    // https://api.pokemontcg.io/v2/cards/sm7-148
    // https://api.pokemontcg.io/v2/cards/sv8-164
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    const bool coordinate_tate_blender_line = !state_.manual_energy_used &&
        strict_payload_timing() && supporter_allowed() && only_tate_supporter_held &&
        hand_count(Card::TateLiza) > 0 && has_live_blender_payload_line() &&
        need_active_vstar() && !can_free_retreat_with_latias() && state_.active &&
        state_.active->card == Card::RegidragoV && !active_can_evolve_now;
    if (coordinate_tate_blender_line) {
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
        trace("ATTACH", "R-GAME-ENERGY; R-TATE-01; R-BLENDER-01; R-RVS-01; P-DCI-01",
              std::string(name(energy)) +
                  " manually to the Benched Regidrago VSTAR selected for Tate & Liza promotion.");
        return true;
      }
    }

    return attach_manual_tate_blender_original();
  }
