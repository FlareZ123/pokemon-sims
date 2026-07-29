bool issue_1697_held_next_turn_manual_energy_route() const {
  // In no-discard-control, a held Basic Energy can finish the one remaining
  // GGF component through next turn's legal manual attachment. Taking the known
  // prized Forest Seal Stone now evolves the prior-turn Regidrago V one turn
  // earlier, preserves the live Crispin, and leaves next turn's Supporter unused:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Supporter, Tool, VSTAR Power, manual attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1, earliest-completion, and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1697
  return scenario_.dci == DciProfile::NoDiscardControl &&
      state_.turn < scenario_.max_turn && state_.manual_energy_used &&
      state_.active &&
      std::max(0, 2 - state_.active->grass) +
              std::max(0, 1 - state_.active->fire) ==
          1 &&
      ((state_.active->grass < 2 && hand_count(Card::Grass) > 0) ||
       (state_.active->fire < 1 && hand_count(Card::Fire) > 0)) &&
      hand_count(Card::Crispin) > 0 &&
      deck_count_after_search_started(Card::Grass) > 0 &&
      deck_count_after_search_started(Card::Fire) > 0 &&
      crispin_can_advance_energy_axis();
}

bool play_gladion_issue1737_original() {
  const bool issue_1644_has_held_vstar_route =
      hand_count(Card::ForestSealStone) > 0 || hand_count(Card::Arven) > 0 ||
      hand_count(Card::EvolutionIncense) > 0 ||
      hand_count(Card::MysteriousTreasure) > 0 ||
      hand_count(Card::UltraBall) > 0 ||
      hand_count(Card::PokemonCommunication) > 0;
  const bool issue_1644_prized_fss_vstar_route =
      prizes_known() && supporter_allowed() && !rule_box_abilities_locked() &&
      !state_.vstar_power_used && state_.active &&
      state_.active->card == Card::RegidragoV &&
      state_.active->entered_turn < state_.turn &&
      state_.active->tool == Tool::None && need_vstar() &&
      (!need_energy() || issue_1697_held_next_turn_manual_energy_route()) &&
      !need_payload() && hand_count(Card::Gladion) > 0 &&
      prize_count_after_reveal(Card::ForestSealStone) > 0 &&
      deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
      !issue_1644_has_held_vstar_route;
  if (issue_1644_prized_fss_vstar_route) {
    // Gladion exchanges itself for the known prized Forest Seal Stone. The Tool
    // attaches to the prior-turn Active Regidrago V, and Star Alchemy searches
    // the inspected deck for Regidrago VSTAR before the legal same-turn evolution.
    // This advances the only unresolved setup axis while the GGF Energy and
    // payload axes remain complete:
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Prize exchange, Tool, VSTAR Power, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and earliest-completion policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1644
    remove_one(state_.hand, Card::Gladion);
    state_.supporter_used = true;
    prizes_revealed_ = true;
    const auto prize = std::find(
        state_.prizes.begin(), state_.prizes.end(), Card::ForestSealStone);
    if (prize == state_.prizes.end()) {
      throw std::logic_error("Known prized Forest Seal Stone disappeared");
    }
    *prize = Card::Gladion;
    state_.hand.push_back(Card::ForestSealStone);
    shuffle(state_.prizes);
    trace("PLAY SUPPORTER",
          "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Exchanged Gladion for Forest Seal Stone to complete the same-turn Regidrago VSTAR route.");
    return true;
  }

  const bool issue_1633_crispin_before_gladion =
      scenario_.dci == DciProfile::NoDiscardControl && prizes_known() &&
      supporter_allowed() && state_.manual_energy_used && !need_payload() &&
      state_.active && state_.active->card == Card::RegidragoV &&
      grass_needed() + fire_needed() == 1 && hand_count(Card::Gladion) > 0 &&
      hand_count(Card::Crispin) > 0 &&
      deck_count_after_search_started(Card::Grass) > 0 &&
      deck_count_after_search_started(Card::Fire) > 0 &&
      deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
      prize_count_after_reveal(Card::RegidragoVstar) == 0 &&
      crispin_can_advance_energy_axis();
  if (issue_1633_crispin_before_gladion) {
    // Crispin deterministically attaches the one missing Basic Energy while
    // putting the other type into hand. The no-control payload is already
    // banked, and K1 proves Regidrago VSTAR remains in deck rather than Prizes.
    // Spending Gladion cannot improve a current setup axis, while preserving it
    // retains the deck's one Prize-exchange resource and avoids a random
    // Celestial Roar that can discard the unresolved VSTAR copies:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, attachment, search, evolution, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, no-control DCI, and direct-completion priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1633
    if (trace_ != nullptr) {
      trace_->add_policy_once(
          state_, "issue-1633-crispin-before-gladion", state_.turn,
          "HOLD SUPPORTER",
          "R-CRISPIN-01; R-GLADION-01; R-RV-01; R-RVS-01; P-KNOWLEDGE-01; P-DCI-01; P-COMPRESS-01",
          "Held Gladion because Crispin deterministically completes GGF while the unresolved VSTAR remains in the inspected deck.");
    }
    return false;
  }

  const bool complete_benched_regidrago_v = std::any_of(
      state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
        return pokemon.card == Card::RegidragoV && pokemon.grass >= 2 &&
            pokemon.fire >= 1;
      });
  const bool issue_1608_burnet_before_dead_crispin =
      scenario_.dci == DciProfile::NoDiscardControl &&
      scenario_.locks == LockMode::None && state_.turn == 3 &&
      prizes_known() && supporter_allowed() && !need_energy() &&
      need_payload() && state_.active &&
      state_.active->card == Card::TapuLeleGX &&
      complete_benched_regidrago_v &&
      hand_count(Card::Gladion) > 0 &&
      hand_count(Card::ProfessorBurnet) > 0 &&
      prize_count_after_reveal(Card::Crispin) > 0 &&
      prize_count_after_reveal(Card::RegidragoV) == 0 &&
      prize_count_after_reveal(Card::RegidragoVstar) == 0 &&
      professor_burnet_has_live_ready_turn_route();
  if (!issue_1608_burnet_before_dead_crispin) {
    return play_gladion_issue1608_original();
  }

  // The Benched Regidrago V already has GGF, so the revealed prized Crispin
  // advances no unresolved setup axis. Held Professor Burnet can legally
  // search the deck for Dragon payloads, discard them, and shuffle. Preserve
  // the Supporter permission for that immediate public payload progress. Do
  // not project a specific next draw through Burnet's required shuffle:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Supporter, deck-search, discard, and shuffle procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1, no-discard-control, dynamic DCI, and earliest-axis policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Refined confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1608
  if (trace_ != nullptr) {
    trace_->add_policy_once(
        state_, "issue-1608-burnet-before-dead-crispin", state_.turn,
        "HOLD SUPPORTER",
        "R-GLADION-01; R-BURNET-01; R-CRISPIN-01; P-DCI-01; P-COMPRESS-01; P-KNOWLEDGE-01",
        "Held Gladion because prized Crispin cannot improve complete GGF and Professor Burnet immediately advances payload.");
  }
  return false;
}

#include "part_issue_1737_gladion_prized_steven_override.inc"
