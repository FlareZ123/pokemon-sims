bool Engine::forretress_combo_enabled() const {
  return total_copies(Card::Pineco) > 0 && total_copies(Card::ForretressEx) > 0;
}

bool Engine::needs_pineco_connector() const {
  return forretress_combo_enabled() && has_any_regi() && need_energy() &&
         bench_space() > 0 && hand_count(Card::Pineco) == 0 &&
         in_play_count(Card::Pineco) == 0 &&
         in_play_count(Card::ForretressEx) == 0 &&
         ability_available_for_pokemon(Card::ForretressEx) &&
         might_be_unseen(Card::Pineco);
}

bool Engine::needs_forretress_connector() const {
  return forretress_combo_enabled() && need_energy() &&
         in_play_count(Card::Pineco) > 0 &&
         in_play_count(Card::ForretressEx) == 0 &&
         hand_count(Card::ForretressEx) == 0 &&
         ability_available_for_pokemon(Card::ForretressEx) &&
         might_be_unseen(Card::ForretressEx);
}

bool Engine::bench_pineco_if_useful() {
  if (!forretress_combo_enabled() || !has_any_regi() || !need_energy() ||
      in_play_count(Card::Pineco) > 0 ||
      in_play_count(Card::ForretressEx) > 0) {
    return false;
  }
  // Pineco is a Basic Pokémon and may be played from hand to the Bench. The
  // custom 2-2 fixture keeps this connector outside the canonical sample deck:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  return bench_from_hand(Card::Pineco, false);
}

bool Engine::evolve_forretress_ex() {
  if (!forretress_combo_enabled() || state_.turn <= 1 ||
      hand_count(Card::ForretressEx) == 0 ||
      in_play_count(Card::ForretressEx) > 0) {
    return false;
  }

  const bool forest_active = state_.stadium == Stadium::ForestOfVitality;
  const auto can_evolve = [this, forest_active](const Pokemon& pokemon) {
    return pokemon.card == Card::Pineco &&
           (pokemon.entered_turn < state_.turn || forest_active);
  };
  Pokemon* pineco = nullptr;
  if (state_.active && can_evolve(*state_.active)) {
    pineco = &*state_.active;
  }
  if (pineco == nullptr) {
    for (Pokemon& pokemon : state_.bench) {
      if (can_evolve(pokemon)) {
        pineco = &pokemon;
        break;
      }
    }
  }
  if (pineco == nullptr ||
      !remove_one(state_.hand, Card::ForretressEx)) {
    return false;
  }

  const bool evolved_on_entry_turn = pineco->entered_turn == state_.turn;
  // Forest of Vitality lets a Grass Pokémon evolve into another Grass Pokémon on
  // its entry turn after the player's first turn. Without it, ordinary prior-turn
  // evolution timing remains mandatory:
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Core evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1096
  pineco->card = Card::ForretressEx;
  trace("EVOLVE", "R-FORRETRESS-01; R-GAME-EVOLUTION; R-FOREST-VITALITY-01",
        evolved_on_entry_turn
            ? "Forest of Vitality allowed the newly played Pineco to evolve into Forretress ex."
            : "Pineco evolved into Forretress ex under normal prior-turn timing.");
  return true;
}

Pokemon* Engine::pokemon_at_board_index(const std::size_t index) {
  if (index == 0U) {
    return state_.active ? &*state_.active : nullptr;
  }
  const std::size_t bench_index = index - 1U;
  return bench_index < state_.bench.size() ? &state_.bench[bench_index] : nullptr;
}

std::optional<std::size_t> Engine::exploding_energy_regidrago_target_index() const {
  const Pokemon* target = target_regi();
  if (target == nullptr) return std::nullopt;
  if (state_.active && target == &*state_.active) return 0U;
  for (std::size_t index = 0; index < state_.bench.size(); ++index) {
    if (target == &state_.bench[index]) return index + 1U;
  }
  return std::nullopt;
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
    const std::vector<std::size_t>& attachment_destinations) {
  if (!forretress_combo_enabled() ||
      !ability_available_for_pokemon(Card::ForretressEx) ||
      attachment_destinations.size() > 5U) {
    return false;
  }

  const bool source_is_active =
      state_.active && state_.active->card == Card::ForretressEx;
  std::optional<std::size_t> source_bench_index;
  if (!source_is_active) {
    for (std::size_t index = 0; index < state_.bench.size(); ++index) {
      if (state_.bench[index].card == Card::ForretressEx) {
        source_bench_index = index;
        break;
      }
    }
  }
  if (!source_is_active && !source_bench_index) return false;

  for (const std::size_t destination : attachment_destinations) {
    if (pokemon_at_board_index(destination) == nullptr) return false;
  }
  if (deck_count_after_search_started(Card::Grass) <
      static_cast<int>(attachment_destinations.size())) {
    return false;
  }

  // Exploding Energy searches for up to five Basic Grass Energy, attaches them
  // in any distribution, shuffles, then Knocks Out Forretress ex. The search is
  // optional, while using it with zero selected cards still performs the deck
  // search and the printed self-Knock-Out instruction:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  record_deck_search_knowledge("Exploding Energy");
  outcome_.used_exploding_energy = true;
  for (const std::size_t destination : attachment_destinations) {
    if (!remove_one(state_.deck, Card::Grass)) {
      throw std::logic_error(
          "Exploding Energy preflight diverged from deck state");
    }
    Pokemon* target = pokemon_at_board_index(destination);
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
    knocked_out = state_.bench[*source_bench_index];
    state_.bench.erase(state_.bench.begin() +
                       static_cast<std::ptrdiff_t>(*source_bench_index));
  }
  discard_forretress_ex_stack(knocked_out);

  trace("USE ABILITY", "R-FORRETRESS-01; R-GAME-ABILITY; R-GAME-KO",
        "Exploding Energy attached " +
            std::to_string(attachment_destinations.size()) +
            " Basic Grass Energy, shuffled the deck, and Knocked Out "
            "Forretress ex. Its Pokémon ex rule would award the opponent 2 "
            "Prize cards in a complete two-player game.");

  if (!state_.active && !state_.bench.empty()) {
    const auto promotion_rank = [](const Pokemon& pokemon) {
      const int role = pokemon.card == Card::RegidragoVstar ? 3 :
          (pokemon.card == Card::RegidragoV ? 2 : 1);
      return std::tuple{role, pokemon.grass + pokemon.fire,
                        -pokemon.entered_turn};
    };
    const auto promoted = std::max_element(
        state_.bench.begin(), state_.bench.end(),
        [&promotion_rank](const Pokemon& left, const Pokemon& right) {
          return promotion_rank(left) < promotion_rank(right);
        });
    state_.active = *promoted;
    state_.bench.erase(promoted);
    trace("PROMOTE", "R-GAME-KO; R-FORRETRESS-01",
          std::string(name(state_.active->card)) +
              " was selected from the Bench after the Active Forretress ex "
              "was Knocked Out.");
  }
  return true;
}

bool Engine::use_exploding_energy_for_setup() {
  if (!forretress_combo_enabled() || !need_energy() ||
      in_play_count(Card::ForretressEx) == 0 ||
      !ability_available_for_pokemon(Card::ForretressEx)) {
    return false;
  }
  const auto target_index = exploding_energy_regidrago_target_index();
  if (!target_index || !might_be_unseen(Card::Grass)) return false;

  std::vector<std::size_t> destinations;
  const bool source_is_active =
      state_.active && state_.active->card == Card::ForretressEx;
  const bool active_blocks_regi = !source_is_active && state_.active &&
      state_.active->card != Card::RegidragoV &&
      state_.active->card != Card::RegidragoVstar;
  const bool retreat_available = active_blocks_regi && !state_.retreat_used;
  // Skyliner gives every Basic Pokémon in play no Retreat Cost while Latias ex's
  // Rule Box Ability is available. Apply that effective cost before distributing
  // Exploding Energy so a free retreat never consumes a searched Grass Energy:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  const bool skyliner_free_retreat = retreat_available &&
      can_free_retreat_with_latias();
  const int active_retreat_cost = retreat_available && !skyliner_free_retreat
      ? retreat_cost(state_.active->card) : 0;
  const int target_grass_needed = std::max(0, grass_needed());
  const bool paid_retreat_route = retreat_available && active_retreat_cost > 0;
  if (target_grass_needed <= 0 && !paid_retreat_route) return false;

  // Choosing to use the Ability at K0 relies only on fixed-list public-zone
  // accounting. The exact remaining Grass count becomes available only after
  // the printed deck search begins:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/1118
  record_deck_search_knowledge("Exploding Energy");
  const int available = std::min(5, deck_count_after_search_started(Card::Grass));
  if (available <= 0) return false;

  // Exploding Energy searches for up to five Grass Energy, so select only the
  // copies that advance the current Grass and paid-Retreat axes. Extra copies do
  // not improve the earliest ready turn and remain in deck for later attackers:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  const bool can_complete_retreat = retreat_available &&
      (active_retreat_cost == 0 ||
       available >= active_retreat_cost + target_grass_needed);
  const int retreat_grass = can_complete_retreat ? active_retreat_cost : 0;
  const int target_grass = std::min(
      target_grass_needed, available - retreat_grass);
  if (retreat_grass + target_grass <= 0) return false;
  if (retreat_grass > 0) {
    destinations.insert(destinations.end(),
                        static_cast<std::size_t>(retreat_grass), 0U);
  }
  destinations.insert(destinations.end(),
                      static_cast<std::size_t>(target_grass), *target_index);

  if (!resolve_exploding_energy(destinations)) return false;
  if (!can_complete_retreat || !state_.active) return true;

  int remaining_cost = active_retreat_cost;
  while (remaining_cost > 0 && state_.active->grass > 0) {
    --state_.active->grass;
    state_.discard.push_back(Card::Grass);
    --remaining_cost;
  }
  while (remaining_cost > 0 && state_.active->fire > 0) {
    --state_.active->fire;
    state_.discard.push_back(Card::Fire);
    --remaining_cost;
  }
  if (remaining_cost != 0) {
    throw std::logic_error("Exploding Energy retreat payment disappeared");
  }
  const auto target = std::max_element(
      state_.bench.begin(), state_.bench.end(), [](const Pokemon& left,
                                                   const Pokemon& right) {
        const auto rank = [](const Pokemon& pokemon) {
          return std::tuple{
              pokemon.card == Card::RegidragoVstar ? 2 :
                  (pokemon.card == Card::RegidragoV ? 1 : 0),
              pokemon.grass + pokemon.fire,
              -pokemon.entered_turn};
        };
        return rank(left) < rank(right);
      });
  if (target == state_.bench.end() ||
      (target->card != Card::RegidragoV &&
       target->card != Card::RegidragoVstar)) {
    throw std::logic_error("Exploding Energy retreat target disappeared");
  }
  std::swap(*state_.active, *target);
  state_.retreat_used = true;
  trace("RETREAT", "R-GAME-RETREAT; R-FORRETRESS-01",
        skyliner_free_retreat
            ? "Skyliner removed the Basic Active's Retreat Cost and promoted Regidrago without discarding Energy."
            : "Used Grass Energy attached by Exploding Energy to pay the Active Pokémon's Retreat Cost and promote Regidrago.");
  return true;
}

bool Engine::advance_forretress_combo() {
  if (!forretress_combo_enabled()) return false;
  bool changed = false;

  const auto prior_turn_pineco = [this](const Pokemon& pokemon) {
    return pokemon.card == Card::Pineco &&
        pokemon.entered_turn < state_.turn;
  };
  const bool pineco_can_evolve_normally =
      (state_.active && prior_turn_pineco(*state_.active)) ||
      std::any_of(state_.bench.begin(), state_.bench.end(), prior_turn_pineco);
  const bool current_turn_pineco =
      (state_.active && state_.active->card == Card::Pineco &&
       state_.active->entered_turn == state_.turn) ||
      std::any_of(state_.bench.begin(), state_.bench.end(),
                  [this](const Pokemon& pokemon) {
                    return pokemon.card == Card::Pineco &&
                        pokemon.entered_turn == state_.turn;
                  });
  const bool pineco_can_enter_this_turn = in_play_count(Card::Pineco) == 0 &&
      bench_space() > 0 &&
      (hand_count(Card::Pineco) > 0 || might_be_unseen(Card::Pineco));
  const bool forretress_available = hand_count(Card::ForretressEx) > 0 ||
      might_be_unseen(Card::ForretressEx);
  // Forest of Vitality changes only entry-turn Grass evolution. Preserve it when
  // a Pineco is already eligible under ordinary prior-turn timing, and spend it
  // only for a current-turn or still-to-be-Benched Pineco with a live Forretress:
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Core evolution and Stadium procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1377
  // Playing Forest into Chaotic Swell still resolves the established Stadium
  // replacement interaction and clears both Stadiums:
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Stadium procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Repository contract: https://github.com/FlareZ123/pokemon-sims/issues/1096
  const bool chaotic_swell_replacement =
      state_.stadium == Stadium::ChaoticSwell;
  const bool combo_stadium_is_useful = !state_.stadium_used && need_energy() &&
      hand_count(Card::ForestOfVitality) > 0 &&
      state_.stadium != Stadium::ForestOfVitality &&
      (chaotic_swell_replacement ||
       (in_play_count(Card::ForretressEx) == 0 &&
        !pineco_can_evolve_normally && forretress_available &&
        (current_turn_pineco || pineco_can_enter_this_turn)));
  if (combo_stadium_is_useful &&
      remove_one(state_.hand, Card::ForestOfVitality)) {
    // Stadiums can be played through Item lock. A player cannot play a Stadium with
    // the same name as the one already in play. Chaotic Swell discards itself and
    // the incoming Stadium instead of allowing that Stadium to remain in play:
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
    // Stadium procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/1096
    if (state_.stadium == Stadium::ChaoticSwell) {
      state_.discard.push_back(Card::ChaoticSwell);
      state_.discard.push_back(Card::ForestOfVitality);
      state_.stadium = Stadium::None;
      trace("PLAY STADIUM", "R-FOREST-VITALITY-01; R-CHAOTIC-SWELL-01",
            "Chaotic Swell discarded itself and the incoming Forest of Vitality.");
    } else {
      state_.stadium = Stadium::ForestOfVitality;
      state_.path_lock_removed = true;
      trace("PLAY STADIUM", "R-FOREST-VITALITY-01; R-GAME-EVOLUTION",
            "Played Forest of Vitality. Grass Pokémon may evolve during their entry turn after the first turn.");
    }
    state_.stadium_used = true;
    changed = true;
  }

  const bool pineco_present = in_play_count(Card::Pineco) > 0 ||
      in_play_count(Card::ForretressEx) > 0 || hand_count(Card::Pineco) > 0;
  const bool pineco_searchable = pineco_present || might_be_unseen(Card::Pineco);
  const bool forretress_searchable = in_play_count(Card::ForretressEx) > 0 ||
      hand_count(Card::ForretressEx) > 0 || might_be_unseen(Card::ForretressEx);
  const bool dawn_advances_combo = supporter_allowed() && need_energy() &&
      ability_available_for_pokemon(Card::ForretressEx) &&
      hand_count(Card::Dawn) > 0 && pineco_searchable && forretress_searchable &&
      (pineco_present || bench_space() > 0);
  if (dawn_advances_combo && remove_one(state_.hand, Card::Dawn)) {
    state_.discard.push_back(Card::Dawn);
    state_.supporter_used = true;
    record_deck_search_knowledge("Dawn");

    std::vector<Card> selected;
    if (in_play_count(Card::Pineco) == 0 &&
        in_play_count(Card::ForretressEx) == 0 &&
        hand_count(Card::Pineco) == 0 && bench_space() > 0 &&
        move_deck_to_hand(Card::Pineco)) {
      selected.push_back(Card::Pineco);
    } else if (need_regi() && move_deck_to_hand(Card::RegidragoV)) {
      selected.push_back(Card::RegidragoV);
    }
    if (in_play_count(Card::ForretressEx) == 0 &&
        hand_count(Card::ForretressEx) == 0 &&
        move_deck_to_hand(Card::ForretressEx)) {
      selected.push_back(Card::ForretressEx);
    }
    const std::array<Card, 2> stage_two_priority =
        secret_box_combo_enabled()
            ? std::array<Card, 2>{Card::Dragapult, Card::MegaDragonite}
            : std::array<Card, 2>{Card::MegaDragonite, Card::Dragapult};
    for (const Card stage_two : stage_two_priority) {
      if (move_deck_to_hand(stage_two)) {
        selected.push_back(stage_two);
        break;
      }
    }
    shuffle(state_.deck);
    // Dawn independently searches one Basic, one Stage 1, and one Stage 2. The
    // route selector takes Pineco, Forretress ex, and a modeled Stage 2 Dragon when
    // those cards remain, while each hidden-information category may legally fail:
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Pineco and Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://github.com/FlareZ123/pokemon-sims/issues/1096
    trace("PLAY SUPPORTER", "R-DAWN-01; R-GAME-SUPPORTER; R-FORRETRESS-01",
          "Dawn searched and revealed: " + join_cards(selected) + ".");
    changed = true;
  }

  if (bench_pineco_if_useful()) changed = true;
  if (evolve_forretress_ex()) changed = true;
  if (use_exploding_energy_for_setup()) changed = true;
  return changed;
}

Card Engine::fallback_combo_basic_after_search_started() const {
  for (const Card target : {Card::Pineco, Card::RegidragoV,
                            Card::LatiasEx, Card::Oricorio,
                            Card::TapuLeleGX, Card::MawileGX,
                            Card::DialgaGX}) {
    if (deck_count_after_search_started(target) > 0) return target;
  }
  return Card::Pineco;
}

Card Engine::fallback_combo_pokemon_after_search_started() const {
  // Ultra Ball searches any Pokémon, so the post-inspection fallback includes
  // the registered Appletun payload:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/1299
  for (const Card target : {Card::ForretressEx, Card::Pineco,
                            Card::RegidragoVstar, Card::RegidragoV,
                            Card::LatiasEx, Card::Oricorio,
                            Card::TapuLeleGX, Card::MawileGX,
                            Card::DialgaGX, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::Appletun, Card::Dipplin}) {
    if (deck_count_after_search_started(target) > 0) return target;
  }
  return Card::Pineco;
}

bool Engine::play_quick_ball_for_pineco() {
  if (!needs_pineco_connector() || item_locked() ||
      hand_count(Card::QuickBall) == 0) {
    return false;
  }
  const auto cost = choose_discard(false, true, true, Card::QuickBall);
  if (!cost) return false;

  remove_one(state_.hand, Card::QuickBall);
  state_.discard.push_back(Card::QuickBall);
  discard_from_hand(*cost, "Quick Ball cost", "R-QB-01");
  record_deck_search_knowledge("Quick Ball");
  Card target = Card::Pineco;
  if (deck_count_after_search_started(target) == 0) {
    target = fallback_combo_basic_after_search_started();
  }
  const bool found = move_deck_to_hand(target);
  shuffle(state_.deck);
  trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; R-FORRETRESS-01",
        found ? "Searched a Basic Pokémon: " + std::string(name(target)) +
                    "."
              : "No Basic Pokémon remained after search.");
  return true;
}

bool Engine::play_ultra_ball_for_forretress_combo() {
  if (item_locked() || hand_count(Card::UltraBall) == 0 ||
      (!needs_pineco_connector() && !needs_forretress_connector())) {
    return false;
  }
  const Card preferred =
      needs_pineco_connector() ? Card::Pineco : Card::ForretressEx;
  if (!might_be_unseen(preferred)) return false;

  const auto first = choose_discard(false, true, true, Card::UltraBall);
  if (!first) return false;
  if (!remove_one(state_.hand, *first)) return false;
  const auto second = choose_discard(false, true, true, Card::UltraBall);
  state_.hand.push_back(*first);
  if (!second) return false;

  remove_one(state_.hand, Card::UltraBall);
  state_.discard.push_back(Card::UltraBall);
  discard_from_hand(*first, "Ultra Ball first cost", "R-UB-01");
  discard_from_hand(*second, "Ultra Ball second cost", "R-UB-01");
  record_deck_search_knowledge("Ultra Ball");
  Card target = preferred;
  if (deck_count_after_search_started(target) == 0) {
    target = fallback_combo_pokemon_after_search_started();
  }
  const bool found = move_deck_to_hand(target);
  shuffle(state_.deck);
  trace("PLAY ITEM", "R-UB-01; R-GAME-ITEM; R-FORRETRESS-01",
        found ? "Searched a Pokémon: " + std::string(name(target)) + "."
              : "No Pokémon remained after search.");
  return true;
}

bool Engine::play_evolution_incense_for_forretress() {
  if (!needs_forretress_connector() || item_locked() ||
      hand_count(Card::EvolutionIncense) == 0) {
    return false;
  }
  remove_one(state_.hand, Card::EvolutionIncense);
  state_.discard.push_back(Card::EvolutionIncense);
  record_deck_search_knowledge("Evolution Incense");
  Card target = Card::ForretressEx;
  if (deck_count_after_search_started(target) == 0) {
    // Evolution Incense searches any Evolution Pokémon, including Appletun:
    // https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://github.com/FlareZ123/pokemon-sims/issues/1255
    for (const Card fallback : {Card::RegidragoVstar,
                                Card::MegaDragonite, Card::Dragapult,
                                Card::GoodraVstar, Card::Appletun}) {
      if (deck_count_after_search_started(fallback) > 0) {
        target = fallback;
        break;
      }
    }
  }
  const bool found = move_deck_to_hand(target);
  shuffle(state_.deck);
  trace("PLAY ITEM", "R-EVOINCENSE-01; R-GAME-ITEM; R-FORRETRESS-01",
        found ? "Searched an Evolution Pokémon: " +
                    std::string(name(target)) + "."
              : "No Evolution Pokémon remained after search.");
  return true;
}

bool Engine::run_forretress_combo_search_step(const bool permit_payload) {
  if (!forretress_combo_enabled() || item_locked()) return false;
  if (needs_pineco_connector()) {
    if (play_pokemon_communication(permit_payload)) return true;
    if (play_quick_ball_for_pineco()) return true;
    if (play_ultra_ball_for_forretress_combo()) return true;
    if (play_heavy_ball()) return true;
  }
  if (needs_forretress_connector()) {
    if (play_evolution_incense_for_forretress()) return true;
    if (play_pokemon_communication(permit_payload)) return true;
    if (play_ultra_ball_for_forretress_combo()) return true;
  }
  return false;
}
