
  bool issue_1595_known_grass_route_available() const {
    const auto powered_benched_vstar = std::find_if(
        state_.bench.begin(), state_.bench.end(), [](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoVstar &&
                 pokemon.grass == 1 && pokemon.fire == 1;
        });
    const bool next_turn_supporter_available =
        !uses_full_supporter_lock(scenario_.locks);
    const bool next_turn_item_available =
        !item_locked_on_turn(state_.turn + 1);

    // Gladion recovers the known prized Grass now. The following turn uses held
    // Professor Turo plus Brilliant Blender, so admission follows the live
    // Supporter/Item lock channels and requires one remaining scenario turn.
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Advanced Supporter, Item, and Energy procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // Same-ready-turn JIT and scenario locks: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
    // Original route / relative-state regression: https://github.com/FlareZ123/pokemon-sims/issues/1595 https://github.com/FlareZ123/pokemon-sims/issues/3408
    return supporter_allowed() && prizes_known() && !item_locked() &&
           strict_payload_timing() &&
           state_.turn + 1 <= scenario_.max_turn &&
           next_turn_supporter_available && next_turn_item_available &&
           !state_.manual_energy_used && state_.active &&
           state_.active->card == Card::TapuLeleGX &&
           powered_benched_vstar != state_.bench.end() &&
           hand_count(Card::Gladion) > 0 &&
           hand_count(Card::BrilliantBlender) > 0 &&
           hand_count(Card::ProfessorTuro) > 0 &&
           prize_count_after_reveal(Card::Grass) > 0 &&
           payload_might_be_in_deck();
  }

bool play_gladion() {
  const bool issue_2368_powered_evolvable_active =
      state_.active && state_.active->card == Card::RegidragoV &&
      state_.active->entered_turn < state_.turn &&
      state_.active->double_dragon > 0 &&
      pays_apex_energy_cost(*state_.active);
  const bool issue_2368_public_no_vstar_route =
      need_vstar() && hand_count(Card::RegidragoVstar) == 0 &&
      hand_count(Card::EvolutionIncense) == 0 &&
      hand_count(Card::MysteriousTreasure) == 0 &&
      hand_count(Card::UltraBall) == 0 &&
      (item_locked() || hand_count(Card::Arven) == 0) &&
      (state_.vstar_power_used || hand_count(Card::ForestSealStone) == 0);
  const bool issue_2368_gladion_is_already_committed =
      supporter_allowed() && hand_count(Card::Gladion) > 0 &&
      strict_payload_timing() && !item_locked() &&
      issue_2368_powered_evolvable_active && issue_2368_public_no_vstar_route &&
      need_payload() && can_play_payload_this_turn() &&
      std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
  const bool issue_2368_reveals_treasure_finish =
      issue_2368_gladion_is_already_committed &&
      count_of(state_.prizes, Card::MysteriousTreasure) > 0 &&
      count_of(state_.deck, Card::RegidragoVstar) > 0;
  if (issue_2368_reveals_treasure_finish) {
    // The public state already commits Gladion because no VSTAR connector is
    // available before its mandatory Prize inspection. After Gladion legally
    // reveals all Prizes, a prized Mysterious Treasure is the two-axis choice when
    // an actually attached DDE has already completed Energy: Treasure discards the
    // held Dragon payload and searches the deck-resident Regidrago VSTAR this turn.
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Prize inspection/exchange, Item, discard, search, evolution, and Energy procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 / earliest-route / strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2368
    resolve_gladion_prize_exchange(Card::MysteriousTreasure);
    trace("PLAY SUPPORTER",
          "R-GLADION-01; R-MT-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-JIT-01; P-COMPRESS-01",
          "Gladion revealed and exchanged for Mysterious Treasure because attached DDE had already completed Energy and Treasure immediately covers VSTAR plus payload.");
    return true;
  }

  if (!issue_1595_known_grass_route_available()) {
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
  // Confirmed bugs: https://github.com/FlareZ123/pokemon-sims/issues/1595 https://github.com/FlareZ123/pokemon-sims/issues/3408
  resolve_gladion_prize_exchange(Card::Grass);
  trace("PLAY SUPPORTER",
        "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
        "Exchanged Gladion for the known prized Grass Energy to complete the next-turn Blender-Turo route.");
  if (!attach_manual()) {
    throw std::logic_error("Issue-1595 recovered Grass attachment disappeared");
  }
  return true;
}