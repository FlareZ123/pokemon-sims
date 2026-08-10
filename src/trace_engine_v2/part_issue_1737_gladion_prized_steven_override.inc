bool issue_1737_prized_steven_route_available() const {
  const bool held_direct_vstar_connector =
      hand_count(Card::RegidragoVstar) > 0 ||
      hand_count(Card::EvolutionIncense) > 0 ||
      hand_count(Card::MysteriousTreasure) > 0 ||
      hand_count(Card::UltraBall) > 0 ||
      hand_count(Card::PokemonCommunication) > 0 ||
      hand_count(Card::ForestSealStone) > 0 ||
      hand_count(Card::Arven) > 0 || hand_count(Card::SecretBox) > 0;
  const bool held_payload_cost = std::any_of(
      state_.hand.begin(), state_.hand.end(), is_payload);
  const bool held_secondary_secret_box_cost = std::any_of(
      state_.hand.begin(), state_.hand.end(), [](const Card card) {
        return card != Card::Gladion && !is_payload(card) &&
            card != Card::RegidragoVstar;
      });

  // Reject every non-target public state before consulting the registered deck
  // recipe. Engine unit fixtures may pass a temporary reduced recipe, while this
  // confirmed route exists only for the registered going-second Pineco seed:
  // First-turn Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Registered Pineco route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#secret-box-and-pineco-route-policy
  // Existing going-first Gladion route preserved: https://github.com/FlareZ123/pokemon-sims/issues/1697
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1737
  if (scenario_.going_first ||
      scenario_.dci != DciProfile::NoDiscardControl ||
      scenario_.locks != LockMode::None ||
      scenario_.opponent_bench == OpponentBenchState::Available ||
      state_.turn + 2 > scenario_.max_turn || !prizes_known() ||
      !supporter_allowed() || hand_count(Card::Gladion) == 0 ||
      prize_count_after_reveal(Card::StevensResolve) == 0 ||
      prize_count_after_reveal(Card::RegidragoVstar) > 0 ||
      !state_.active || state_.active->card != Card::RegidragoV ||
      state_.active->entered_turn >= state_.turn ||
      state_.active->grass < 2 || state_.active->fire != 0 ||
      !need_vstar() || !need_energy() || !need_payload() ||
      hand_count(Card::StevensResolve) > 0 ||
      hand_count(Card::ProfessorBurnet) > 0 ||
      held_direct_vstar_connector || !held_payload_cost ||
      !held_secondary_secret_box_cost ||
      deck_count_after_search_started(Card::Fire) == 0 ||
      deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
      deck_count_after_search_started(Card::SecretBox) == 0 ||
      deck_count_after_search_started(Card::MysteriousTreasure) < 2 ||
      deck_count_after_search_started(Card::Grant) == 0 ||
      deck_count_after_search_started(Card::WishfulBaton) == 0) {
    return false;
  }

  const auto recipe_copies = [this](const Card card) {
    int copies = 0;
    for (const auto& [candidate, count] : recipe_) {
      if (candidate == card) copies += count;
    }
    return copies;
  };
  const bool registered_pineco_recipe =
      recipe_copies(Card::SecretBox) == 1 &&
      recipe_copies(Card::Pineco) == 2 &&
      recipe_copies(Card::ForretressEx) == 2 &&
      recipe_copies(Card::MysteriousTreasure) == 4 &&
      recipe_copies(Card::StevensResolve) == 1;

  // Gladion reveals every Prize before the exchange. In this registered K1
  // Pineco state, prized Steven's Resolve is a live multi-axis connector while
  // Guzma has no modeled switch target unless an opponent Bench is explicitly
  // available. Steven banks Secret Box plus two discardable cards; Secret Box
  // then reaches Mysterious Treasure, which spends the held Dragon payload and
  // searches Regidrago VSTAR. Preserve direct held VSTAR channels ahead of this
  // deferred route:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Guzma: https://api.pokemontcg.io/v2/cards/sm3-115
  // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Prize exchange, Supporter, Item, evolution, attachment, and attack procedure:
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, route priority, and registered Pineco policy:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#secret-box-and-pineco-route-policy
  // Guzma opponent-Bench model:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#guzma-opponent-bench-prerequisite
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1737
  return registered_pineco_recipe && secret_box_combo_enabled();
}

bool issue_2292_gladion_final_prize_energy_finish(const Card energy) const {
  if (scenario_.dci != DciProfile::StrictJit || !prizes_known() ||
      !supporter_allowed() || state_.manual_energy_used ||
      hand_count(Card::Gladion) == 0 || hand_count(energy) > 0 ||
      !state_.active || state_.active->card != Card::RegidragoVstar ||
      pays_apex_energy_cost(*state_.active) ||
      prize_count_after_reveal(energy) == 0 || !payload_ready() ||
      (energy != Card::Grass && energy != Card::Fire)) {
    return false;
  }

  Pokemon projected = *state_.active;
  const bool final_basic_completes_apex =
      attach_energy_card(projected, energy) && pays_apex_energy_cost(projected);

  // Gladion may exchange itself for any known Prize. Evaluate the final Basic
  // attachment by projecting the physical Energy card onto the current Active,
  // rather than summing typed Grass/Fire deficits. A previously attached Double
  // Dragon Energy contributes two flexible units to Apex Dragon, so additive
  // typed-deficit arithmetic can misclassify an otherwise complete GGF payment.
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Prize, Supporter, and manual Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, strict-JIT, and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original Gladion bug: https://github.com/FlareZ123/pokemon-sims/issues/2292
  // Confirmed DDE semantic migration: https://github.com/FlareZ123/pokemon-sims/issues/2368
  return final_basic_completes_apex;
}

bool play_issue_2292_gladion_final_prize_energy_finish() {
  const auto target = final_prized_energy_for_gladion();
  if (!target || !issue_2292_gladion_final_prize_energy_finish(*target)) {
    return false;
  }

  remove_one(state_.hand, Card::Gladion);
  state_.supporter_used = true;
  prizes_revealed_ = true;
  const auto prize = std::find(state_.prizes.begin(), state_.prizes.end(), *target);
  if (prize == state_.prizes.end()) {
    throw std::logic_error("Known prized final Energy disappeared");
  }
  *prize = Card::Gladion;
  state_.hand.push_back(*target);
  shuffle(state_.prizes);
  trace("PLAY SUPPORTER",
        "R-GLADION-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-JIT-01; P-COMPRESS-01",
        "Exchanged Gladion for the known prized final Basic Energy so the unused manual attachment can complete GGF this turn.");
  return true;
}

bool play_gladion() {
  if (play_issue_2292_gladion_final_prize_energy_finish()) return true;

  if (!issue_1737_prized_steven_route_available()) {
    return play_gladion_issue1737_original();
  }

  remove_one(state_.hand, Card::Gladion);
  state_.supporter_used = true;
  prizes_revealed_ = true;
  const auto prize = std::find(
      state_.prizes.begin(), state_.prizes.end(), Card::StevensResolve);
  if (prize == state_.prizes.end()) {
    throw std::logic_error("Known prized Steven's Resolve disappeared");
  }
  *prize = Card::Gladion;
  state_.hand.push_back(Card::StevensResolve);
  shuffle(state_.prizes);
  trace("PLAY SUPPORTER",
        "R-GLADION-01; R-STEVEN-01; R-SECRET-BOX-01; P-KNOWLEDGE-01; P-COMPRESS-01",
        "Exchanged Gladion for Steven's Resolve to preserve the registered Secret Box and VSTAR continuation.");
  return true;
}
