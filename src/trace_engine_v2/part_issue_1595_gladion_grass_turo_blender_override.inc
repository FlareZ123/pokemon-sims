
bool play_gladion() {
  const auto powered_benched_vstar = std::find_if(
      state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
        return pokemon.card == Card::RegidragoVstar &&
               pokemon.grass == 1 && pokemon.fire == 1;
      });
  const bool issue_1595_known_grass_route =
      supporter_allowed() && prizes_known() && !item_locked() &&
      scenario_.dci == DciProfile::MatchupFlexJit &&
      scenario_.locks == LockMode::None && state_.turn == 3 &&
      !state_.manual_energy_used && state_.active &&
      state_.active->card == Card::TapuLeleGX &&
      powered_benched_vstar != state_.bench.end() &&
      hand_count(Card::Gladion) > 0 &&
      hand_count(Card::BrilliantBlender) > 0 &&
      hand_count(Card::ProfessorTuro) > 0 &&
      prize_count_after_reveal(Card::Grass) > 0 &&
      payload_might_be_in_deck();
  if (!issue_1595_known_grass_route) {
    return play_gladion_issue1595_original();
  }

  // K1 proves the final Grass Energy is in the Prizes. Gladion recovers it and
  // the unused manual attachment completes GGF on the Benched VSTAR. Held
  // Brilliant Blender and Professor Turo then complete payload plus Active axes
  // next turn without Supporter contention:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Prize exchange, Supporter, attachment, Item, discard, return, and promotion procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1, dynamic DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1595
  remove_one(state_.hand, Card::Gladion);
  state_.supporter_used = true;
  prizes_revealed_ = true;
  const auto prize = std::find(state_.prizes.begin(), state_.prizes.end(),
                               Card::Grass);
  if (prize == state_.prizes.end()) {
    throw std::logic_error("Issue-1595 known prized Grass Energy disappeared");
  }
  *prize = Card::Gladion;
  state_.hand.push_back(Card::Grass);
  shuffle(state_.prizes);
  trace("PLAY SUPPORTER",
        "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
        "Exchanged Gladion for the known prized Grass Energy to complete the next-turn Blender-Turo route.");
  if (!attach_manual()) {
    throw std::logic_error("Issue-1595 recovered Grass attachment disappeared");
  }
  return true;
}
