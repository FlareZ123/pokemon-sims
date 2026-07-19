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

    Pokemon* pineco = nullptr;
    if (state_.active && state_.active->card == Card::Pineco &&
        state_.active->entered_turn < state_.turn) {
      pineco = &*state_.active;
    }
    if (pineco == nullptr) {
      for (Pokemon& pokemon : state_.bench) {
        if (pokemon.card == Card::Pineco &&
            pokemon.entered_turn < state_.turn) {
          pineco = &pokemon;
          break;
        }
      }
    }
    if (pineco == nullptr ||
        !remove_one(state_.hand, Card::ForretressEx)) {
      return false;
    }

    // Forretress ex is a Stage 1 that evolves from Pineco. A Pokémon cannot
    // evolve on the first turn or during the turn it entered play:
    // https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/972
    pineco->card = Card::ForretressEx;
    trace("EVOLVE", "R-FORRETRESS-01; R-GAME-EVOLUTION",
          "Pineco evolved into Forretress ex.");
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
      state_.active = state_.bench.front();
      state_.bench.erase(state_.bench.begin());
      trace("PROMOTE", "R-GAME-KO",
            std::string(name(state_.active->card)) +
                " was promoted after the Active Forretress ex was Knocked Out.");
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

    // Choosing to use the Ability at K0 relies only on fixed-list public-zone
    // accounting. The exact remaining Grass count becomes available only after
    // the printed deck search begins:
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/issues/972
    record_deck_search_knowledge("Exploding Energy");
    const int available = deck_count_after_search_started(Card::Grass);

    // Use all five legal attachments when available. Extra Grass Energy remains
    // attached to the selected Regidrago and therefore preserves the modeled GGF
    // requirement while exercising the complete printed combo:
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/972
    const int selected = std::min(5, available);
    return resolve_exploding_energy(
        std::vector<std::size_t>(static_cast<std::size_t>(selected),
                                 *target_index));
  }

bool Engine::advance_forretress_combo() {
    bool changed = bench_pineco_if_useful();
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
    for (const Card target : {Card::ForretressEx, Card::Pineco,
                              Card::RegidragoVstar, Card::RegidragoV,
                              Card::LatiasEx, Card::Oricorio,
                              Card::TapuLeleGX, Card::MawileGX,
                              Card::DialgaGX, Card::MegaDragonite,
                              Card::Dragapult, Card::GoodraVstar,
                              Card::Dipplin}) {
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
      for (const Card fallback : {Card::RegidragoVstar,
                                  Card::MegaDragonite, Card::Dragapult,
                                  Card::GoodraVstar}) {
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
