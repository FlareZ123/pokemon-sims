bool play_gladion() {
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
