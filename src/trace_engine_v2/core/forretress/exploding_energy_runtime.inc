// Forretress ex / Exploding Energy runtime ownership:
// https://api.pokemontcg.io/v2/cards/sv4pt5-2
// Core Ability, search, attachment, shuffle, Knock Out, and promotion procedure:
// https://www.pokemon.com/us/pokemon-tcg/rules
// Canonical board-state seam:
// https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc

Engine::OptionalBoardIndex Engine::exploding_energy_regidrago_target_index() const {
  // Preserve the strategy-selected Regidrago target while translating its board
  // pointer through the shared Active-first board-index contract:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  return board_index_for_pointer(target_regi());
}

void Engine::discard_forretress_ex_stack(const Pokemon& knocked_out) {
  state_.discard.push_back(Card::ForretressEx);
  state_.discard.push_back(Card::Pineco);
  for (int count = 0; count < knocked_out.grass; ++count) {
    state_.discard.push_back(Card::Grass);
  }
  for (int count = 0; count < knocked_out.fire; ++count) {
    state_.discard.push_back(Card::Fire);
  }
  if (knocked_out.tool == Tool::ForestSealStone) {
    state_.discard.push_back(Card::ForestSealStone);
  } else if (knocked_out.tool == Tool::Powerglass) {
    state_.discard.push_back(Card::Powerglass);
  }
}

bool Engine::resolve_exploding_energy(
    const AttachmentDestinations& attachment_destinations) {
  // Ability effects using "up to X" require a choice from 1 through X.
  // Reject an empty destination list before mutating knowledge or the board:
  // Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085
  // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1587
  if (!forretress_combo_enabled() ||
      !ability_available_for_pokemon(Card::ForretressEx) ||
      attachment_destinations.empty() ||
      attachment_destinations.size() > 5U) {
    return false;
  }

  const OptionalBoardIndex source_index = board_index_for_card(Card::ForretressEx);
  if (!source_index) return false;
  const bool source_is_active = board_index_is_active(*source_index);

  for (const BoardIndex destination : attachment_destinations) {
    if (board_pokemon_at_index(destination) == nullptr) return false;
  }
  if (deck_count_after_search_started(Card::Grass) <
      static_cast<int>(attachment_destinations.size())) {
    return false;
  }

  // Exploding Energy searches for one through five Basic Grass Energy,
  // attaches them in any distribution, shuffles, then Knocks Out Forretress ex.
  // The February 2026 rule update permits zero only for attack effects:
  // Official February 2026 ruling: https://professorprogram.pokemon.com/news/11473085
  // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Core Ability, search, attachment, shuffle, and Knock Out procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1587
  record_deck_search_knowledge("Exploding Energy");
  outcome_.used_exploding_energy = true;
  for (const BoardIndex destination : attachment_destinations) {
    if (!remove_one(state_.deck, Card::Grass)) {
      throw std::logic_error(
          "Exploding Energy preflight diverged from deck state");
    }
    Pokemon* target = board_pokemon_at_index(destination);
    if (target == nullptr) {
      throw std::logic_error(
          "Exploding Energy destination disappeared during resolution");
    }
    ++target->grass;
  }
  shuffle(state_.deck);

  Pokemon knocked_out;
  if (source_is_active) {
    knocked_out = *state_.active;
    state_.active.reset();
  } else {
    const BoardIndex bench_position =
        bench_position_from_board_index(*source_index);
    knocked_out = state_.bench[bench_position];
    state_.bench.erase(state_.bench.begin() +
                       static_cast<std::ptrdiff_t>(bench_position));
  }
  discard_forretress_ex_stack(knocked_out);

  trace("USE ABILITY", "R-FORRETRESS-01; R-GAME-ABILITY; R-GAME-KO",
        "Exploding Energy attached " +
            std::to_string(attachment_destinations.size()) +
            " Basic Grass Energy, shuffled the deck, and Knocked Out "
            "Forretress ex. Its Pokémon ex rule would award the opponent 2 "
            "Prize cards in a complete two-player game.");

  if (!state_.active && !state_.bench.empty()) {
    const auto promotion_rank = [this](const Pokemon& pokemon) {
      const int role = pokemon.card == Card::RegidragoVstar ? 3 :
          (pokemon.card == Card::RegidragoV ? 2 : 1);
      const bool regidrago = pokemon.card == Card::RegidragoVstar ||
          pokemon.card == Card::RegidragoV;
      const bool apex_paid = regidrago && pays_apex_energy_cost(pokemon);
      bool one_basic_from_apex = false;
      if (regidrago && !apex_paid) {
        for (const Card basic : {Card::Grass, Card::Fire}) {
          Pokemon projected = pokemon;
          if (attach_energy_card(projected, basic) &&
              pays_apex_energy_cost(projected)) {
            one_basic_from_apex = true;
            break;
          }
        }
      }
      // Preserve evolution-stage priority, then prefer the same-stage attacker
      // closest to an immediate Apex Dragon. DDE supplies two Energy of every type:
      // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
      // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Knock Out / replacement Active: https://www.pokemon.com/us/pokemon-tcg/rules
      // DDE semantic contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
      // Policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2447
      const int semantic_energy_class = apex_paid ? 2 :
          (one_basic_from_apex ? 1 : 0);
      const int attached_energy_units = pokemon.grass + pokemon.fire +
          2 * pokemon.double_dragon;
      return std::tuple{role, semantic_energy_class, attached_energy_units,
                        -pokemon.entered_turn};
    };
    // Keep Forretress strategy ranking local while reusing the canonical Bench
    // query and first-equal ordering contract:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/board_state_policy.inc
    const BoardIndex promoted_index =
        *best_benched_pokemon_index(promotion_rank);
    const BoardIndex promoted_bench_position =
        bench_position_from_board_index(promoted_index);
    state_.active = state_.bench[promoted_bench_position];
    state_.bench.erase(
        state_.bench.begin() +
        static_cast<std::ptrdiff_t>(promoted_bench_position));
    trace("PROMOTE", "R-GAME-KO; R-FORRETRESS-01",
          std::string(name(state_.active->card)) +
              " was selected from the Bench after the Active Forretress ex "
              "was Knocked Out.");
  }
  return true;
}
