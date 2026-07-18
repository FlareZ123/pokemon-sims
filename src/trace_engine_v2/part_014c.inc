  bool attach_live_fss() {
    // Forest Seal Stone grants a VSTAR Power, and a player may use only one VSTAR
    // Power during a game. Once that resource is spent, attaching the Stone cannot
    // advance setup and would consume the holder's one Tool slot:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    if (state_.vstar_power_used) return false;
    return attach_fss();
  }

  bool attach_powerglass() {
    // Powerglass is a Pokémon Tool, so it can be attached during the turn even
    // through Item lock. Its delayed effect matters only on the Active holder:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
    if (hand_count(Card::Powerglass) == 0 || !state_.active || state_.active->tool != Tool::None ||
        (state_.active->card != Card::RegidragoV && state_.active->card != Card::RegidragoVstar)) return false;

    // Powerglass attaches Energy to its own Active holder, not to the policy's
    // separately selected Regidrago line: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed == 0 && active_fire_needed == 0) return false;
    const bool needed_grass_in_discard = active_grass_needed > 0 && count_of(state_.discard, Card::Grass) > 0;
    const bool needed_fire_in_discard = active_fire_needed > 0 && count_of(state_.discard, Card::Fire) > 0;
    if (!needed_grass_in_discard && !needed_fire_in_discard) return false;

    remove_one(state_.hand, Card::Powerglass);
    state_.active->tool = Tool::Powerglass;
    trace("ATTACH TOOL", "R-POWERGLASS-01; R-GAME-TOOL",
          "Attached Powerglass to the Active " + std::string(name(state_.active->card)) + ".");
    return true;
  }

  bool resolve_powerglass_end_turn() {
    // Powerglass resolves at the end of the turn after the attack timing point and
    // only while its holder is Active: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    if (!state_.active || state_.active->tool != Tool::Powerglass) return false;

    // The effect can attach only to the attached Active Pokémon. Determine the
    // Energy type from that holder's own GGF deficit: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    const int active_grass_needed = std::max(0, 2 - state_.active->grass);
    const int active_fire_needed = std::max(0, 1 - state_.active->fire);
    if (active_grass_needed == 0 && active_fire_needed == 0) return false;

    Card energy = Card::Grass;
    if (active_grass_needed > 0 && count_of(state_.discard, Card::Grass) > 0) {
      energy = Card::Grass;
    } else if (active_fire_needed > 0 && count_of(state_.discard, Card::Fire) > 0) {
      energy = Card::Fire;
    } else {
      return false;
    }

    if (!remove_one(state_.discard, energy)) throw std::logic_error("Powerglass Energy target disappeared");
    if (energy == Card::Grass) ++state_.active->grass; else ++state_.active->fire;
    trace("POWERGLASS", "R-POWERGLASS-01",
          "Attached " + std::string(name(energy)) + " from discard at end of turn.");
    return true;
  }

  bool play_chaotic_swell() {
    // Chaotic Swell is a Stadium, so Item lock does not prevent it from being played.
    // A different Stadium coming into play discards the Stadium already in play:
    // https://api.pokemontcg.io/v2/cards/sm12-187
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // Path to the Peak suppresses Rule Box Pokémon Abilities only while it remains
    // in play: https://api.pokemontcg.io/v2/cards/swsh6-148
    if (!rule_box_abilities_locked() || hand_count(Card::ChaoticSwell) == 0 ||
        state_.stadium == Stadium::ChaoticSwell) return false;

    remove_one(state_.hand, Card::ChaoticSwell);
    state_.stadium = Stadium::ChaoticSwell;
    state_.path_lock_removed = true;
    trace("PLAY STADIUM", "R-CHAOTIC-SWELL-01; R-PATH-01",
          "Played Chaotic Swell, replacing the modeled Path to the Peak and restoring Rule Box Pokémon Abilities.");
    return true;
  }

  bool play_field_blower() {
    if (item_locked() || hand_count(Card::FieldBlower) == 0 || !rule_box_abilities_locked()) return false;

    // Field Blower may choose a Stadium in play and discard it. The Rule Box lock
    // scenario represents Path to the Peak, whose suppression ends when it leaves play:
    // https://api.pokemontcg.io/v2/cards/sm2-125
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    remove_one(state_.hand, Card::FieldBlower);
    state_.discard.push_back(Card::FieldBlower);
    state_.path_lock_removed = true;
    trace("PLAY ITEM", "R-GAME-ITEM",
          "Field Blower discarded the modeled Path to the Peak; Rule Box Pokémon Abilities are available again.");
    return true;
  }

  void run_turn() {
    trace("POLICY", "P-AXIS-01", "Start: " + state_line());
    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());
    attach_live_fss();
    use_fss();
    play_chaotic_swell();
    play_field_blower();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());

    attach_manual();
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
    attach_manual();
    retreat_to_benched_vstar_with_latias();
    if (need_active_vstar() && !state_.supporter_used) play_tate_switch();
    // Attach only after the final modeled switch decision, while the intended holder
    // is known to remain Active for Powerglass's end-of-turn condition:
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    attach_powerglass();
    use_celestial_roar();
    trace("POLICY", "P-AXIS-01", "End: " + state_line());
  }

  void record_ready() {
    const bool ready = state_.turn >= 2 && active_is_vstar() && state_.active->grass >= 2 && state_.active->fire >= 1 && payload_ready();
    if (!ready) return;
    if (outcome_.first_ready_turn == 0) {
      outcome_.first_ready_turn = state_.turn;
      trace("READY", "R-RVS-01; P-JIT-01", "Active Regidrago VSTAR has GGF and a permitted Dragon payload in discard.");
    }
    if (state_.turn <= 2) outcome_.ready_by_2 = true;
    if (state_.turn <= 3) outcome_.ready_by_3 = true;
    if (state_.turn <= 4) outcome_.ready_by_4 = true;
    if (state_.turn <= 5) outcome_.ready_by_5 = true;
  }

  // The original run_turn() above is macro-renamed by the active composition. End
  // that rename and compose the Latias helper and replacement while Engine is open:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/regidrago_sim.cpp#L165-L169
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_014c_latias_bench_override.inc#L1-L105
#undef run_turn
#include "part_014c_latias_bench_override.inc"
};

DeckRecipe baseline_recipe() { return DeckRecipe(kDeckRecipe.begin(), kDeckRecipe.end()); }

std::vector<Scenario> all_scenarios() {
  std::vector<Scenario> result;
  for (const bool first : {true, false}) {
    const std::string seat = first ? "go-first" : "go-second";
    result.push_back({"strict-jit/" + seat, DciProfile::StrictJit, LockMode::None, first, 5});
    result.push_back({"matchup-flex-jit/" + seat, DciProfile::MatchupFlexJit, LockMode::None, first, 5});
    result.push_back({"no-discard-control/" + seat, DciProfile::NoDiscardControl, LockMode::None, first, 5});
    result.push_back({"strict-jit-turn2-item-lock/" + seat, DciProfile::StrictJit, LockMode::TurnTwoItem, first, 5});
    result.push_back({"strict-jit-full-item-lock/" + seat, DciProfile::StrictJit, LockMode::FullItem, first, 5});
    result.push_back({"strict-jit-rulebox-ability-lock/" + seat, DciProfile::StrictJit, LockMode::FullRuleBoxAbility, first, 5});
    result.push_back({"strict-jit-combined-lock/" + seat, DciProfile::StrictJit, LockMode::FullCombined, first, 5});
  }
  return result;
}

std::optional<Scenario> scenario_by_label(const std::string& label) {
  for (const Scenario& scenario : all_scenarios()) if (scenario.label == label) return scenario;