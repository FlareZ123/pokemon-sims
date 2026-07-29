  int klara_recoverable_copies(const Card card) const {
    return count_of(state_.discard, card);
  }

  int klara_older_discard_copies(const Card card) const {
    return std::max(0, count_of(state_.discard, card) -
                            count_of(state_.discarded_this_turn, card));
  }

  bool klara_payload_copy_is_safe_to_recover(
      const Card card, const std::vector<Card>& already_selected) const {
    if (!is_payload(card) || klara_recoverable_copies(card) <= count_of(already_selected, card)) {
      return false;
    }

    // A strict-JIT ready payload must enter discard this turn and remain there at the
    // ready check. Klara may select an older duplicate while preserving that current-
    // turn copy, but it must not lift the sole current-turn Dragon out of discard:
    // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/1773
    if (strict_payload_timing() && payload_ready()) {
      return klara_older_discard_copies(card) > count_of(already_selected, card);
    }

    // Under no-discard-control, any Dragon in discard satisfies the payload axis.
    // Preserve at least one such Dragon when Klara is being played for another axis.
    if (!strict_payload_timing() && payload_ready()) {
      const int payloads_in_discard = static_cast<int>(std::count_if(
          state_.discard.begin(), state_.discard.end(), is_payload));
      const int selected_payloads = static_cast<int>(std::count_if(
          already_selected.begin(), already_selected.end(), is_payload));
      return payloads_in_discard - selected_payloads > 1;
    }
    return true;
  }

  bool klara_payload_route_available(const Card payload) const {
    if (!is_payload(payload) || count_of(state_.discard, payload) == 0 ||
        !need_payload() || !can_play_payload_this_turn()) {
      return false;
    }

    // Klara uses the Supporter action, so a recovered Dragon must have a legal Item
    // discard outlet. Project the paid Klara state and require the exact one-discard
    // or Ultra Ball route rather than treating a held Item by name as sufficient:
    // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Official Trainer procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/1773
    Engine projected = *this;
    if (!remove_one(projected.state_.hand, Card::Klara)) return false;
    projected.state_.discard.push_back(Card::Klara);
    projected.state_.supporter_used = true;
    if (!projected.recover_discard_to_hand(payload)) return false;
    return projected.has_live_one_discard_hand_payload_line() ||
           projected.has_live_ultra_ball_hand_payload_line();
  }

  std::vector<Card> klara_pokemon_recovery_targets() const {
    std::vector<Card> targets;
    const auto recover_once = [this, &targets](const Card card) {
      if (targets.size() >= 2U ||
          klara_recoverable_copies(card) <= count_of(targets, card)) {
        return;
      }
      targets.push_back(card);
    };

    // Direct setup-axis recoveries. Klara moves the selected cards straight to hand,
    // so no Item search or shuffle is required before the later Bench/evolution fixed
    // point in the same turn:
    // https://api.pokemontcg.io/v2/cards/swsh6-145
    // https://www.pokemon.com/us/pokemon-tcg/rules
    if (need_vstar() && has_regidrago_v_that_can_evolve_this_turn()) {
      recover_once(Card::RegidragoVstar);
    }
    if (need_regi() && bench_space() > 0) recover_once(Card::RegidragoV);
    if (need_active_vstar() && bench_space() > 0 &&
        ability_available_for_pokemon(Card::LatiasEx)) {
      recover_once(Card::LatiasEx);
    }

    if (need_payload()) {
      for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                                 Card::GoodraVstar, Card::DialgaGX,
                                 Card::Appletun}) {
        if (klara_payload_route_available(payload)) {
          recover_once(payload);
          break;
        }
      }
    }

    return targets;
  }

  std::vector<Card> klara_energy_recovery_targets() const {
    std::vector<Card> targets;
    const auto recover_once = [this, &targets](const Card energy) {
      if (targets.size() < 2U &&
          count_of(state_.discard, energy) > count_of(targets, energy)) {
        targets.push_back(energy);
      }
    };

    // Recover missing types first. When one manual attachment completes GGF, this is
    // an immediate setup route. A second recovered Basic Energy is still legal and
    // improves the next Regidrago or post-attack recovery line without exposing the
    // recovery to Item denial such as Eri:
    // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
    // Eri: https://api.pokemontcg.io/v2/cards/sv5-146
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    if (grass_needed() > 0) recover_once(Card::Grass);
    if (fire_needed() > 0) recover_once(Card::Fire);
    while (targets.size() < 2U &&
           count_of(state_.discard, Card::Grass) > count_of(targets, Card::Grass)) {
      recover_once(Card::Grass);
    }
    while (targets.size() < 2U &&
           count_of(state_.discard, Card::Fire) > count_of(targets, Card::Fire)) {
      recover_once(Card::Fire);
    }
    return targets;
  }

  bool klara_targets_advance_setup(const std::vector<Card>& pokemon,
                                   const std::vector<Card>& energy) const {
    if (std::find(pokemon.begin(), pokemon.end(), Card::RegidragoVstar) != pokemon.end() ||
        std::find(pokemon.begin(), pokemon.end(), Card::RegidragoV) != pokemon.end() ||
        std::find(pokemon.begin(), pokemon.end(), Card::LatiasEx) != pokemon.end() ||
        std::any_of(pokemon.begin(), pokemon.end(), [this](const Card card) {
          return is_payload(card) && klara_payload_route_available(card);
        })) {
      return true;
    }
    return !state_.manual_energy_used && grass_needed() + fire_needed() == 1 &&
           ((grass_needed() == 1 && count_of(energy, Card::Grass) > 0) ||
            (fire_needed() == 1 && count_of(energy, Card::Fire) > 0));
  }

  void klara_fill_discrete_value_targets(std::vector<Card>& pokemon) const {
    if (pokemon.size() >= 2U) return;

    // Once an advancing target makes Klara worth the Supporter action, use the second
    // optional Pokémon selection for a redundant older Dragon when that does not remove
    // the current-turn strict-JIT payload. This models the requested discard-control
    // denial while preserving the attack-ready discard state:
    // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
    // Team Yell's Cheer: https://api.pokemontcg.io/v2/cards/swsh9-149
    // Lusamine: https://api.pokemontcg.io/v2/cards/sm4-96
    // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/1773
    for (const Card payload : {Card::MegaDragonite, Card::Dragapult,
                               Card::GoodraVstar, Card::DialgaGX,
                               Card::Appletun}) {
      while (pokemon.size() < 2U &&
             klara_payload_copy_is_safe_to_recover(payload, pokemon)) {
        pokemon.push_back(payload);
      }
    }
  }

  bool suppress_klara_projection_{false};

  bool non_klara_current_turn_completion_exists() {
    if (suppress_klara_projection_ || hand_count(Card::Klara) == 0) return false;

    // Klara must not consume the Supporter action merely because one recovered card
    // improves an axis when the existing hand and deck connectors already complete
    // every setup axis this turn. Project the rest of the exact current turn with
    // Klara disabled but still physically present in hand, preserving its legal value
    // as discard fodder and preserving every later selector input. The projection uses
    // a copied Engine state and restores the shared RNG immediately afterward, so it
    // cannot perturb the real trial's shuffle or draw stream:
    // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Official Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Regression evidence: https://github.com/FlareZ123/pokemon-sims/issues/1773
    const std::mt19937_64 saved_rng = rng_;
    Engine projected = *this;
    projected.trace_ = nullptr;
    projected.suppress_klara_projection_ = true;
    try {
      projected.run_turn();
    } catch (...) {
      rng_ = saved_rng;
      throw;
    }
    rng_ = saved_rng;
    return projected.active_is_vstar() && projected.state_.active->grass >= 2 &&
           projected.state_.active->fire >= 1 && projected.payload_ready();
  }

  bool play_klara_recovery() {
    if (suppress_klara_projection_ || !supporter_allowed() ||
        hand_count(Card::Klara) == 0 || !setup_axis_missing()) {
      return false;
    }

    std::vector<Card> pokemon = klara_pokemon_recovery_targets();
    const std::vector<Card> energy = klara_energy_recovery_targets();
    if (!klara_targets_advance_setup(pokemon, energy)) return false;
    if (non_klara_current_turn_completion_exists()) {
      trace("HOLD SUPPORTER", "R-KLARA-01; P-CONNECTOR-01",
            "Held Klara because the existing non-Klara connector route completes every setup axis this turn.");
      return false;
    }
    klara_fill_discrete_value_targets(pokemon);

    remove_one(state_.hand, Card::Klara);
    state_.discard.push_back(Card::Klara);
    state_.supporter_used = true;
    for (const Card card : pokemon) {
      if (!recover_discard_to_hand(card)) {
        throw std::logic_error("Klara Pokémon recovery target disappeared");
      }
    }
    for (const Card card : energy) {
      if (!recover_discard_to_hand(card)) {
        throw std::logic_error("Klara Basic Energy recovery target disappeared");
      }
    }

    std::string detail = "Klara recovered";
    if (!pokemon.empty()) detail += " Pokémon: " + join_cards(pokemon);
    if (!pokemon.empty() && !energy.empty()) detail += "; and";
    if (!energy.empty()) detail += " Basic Energy: " + join_cards(energy);
    detail += ".";
    trace("PLAY SUPPORTER", "R-KLARA-01; R-GAME-SUPPORTER; P-DCI-01", detail);
    return true;
  }

  void choose_supporter() {
    if (!supporter_allowed() || !setup_axis_missing()) return;
    if (play_klara_recovery()) return;
    choose_supporter_klara_original();
  }
