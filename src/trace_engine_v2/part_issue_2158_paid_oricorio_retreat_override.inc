  bool issue_2158_paid_oricorio_retreat_burnet_available() const {
    if (!strict_payload_timing() || !prizes_known() || !supporter_allowed() ||
        state_.supporter_used || state_.manual_energy_used ||
        state_.retreat_used || state_.turn > scenario_.max_turn ||
        !state_.active || state_.active->card != Card::Oricorio ||
        state_.active->grass != 0 || state_.active->fire != 0 ||
        hand_count(Card::ProfessorBurnet) == 0 ||
        (hand_count(Card::Grass) == 0 && hand_count(Card::Fire) == 0) ||
        !need_active_vstar() || need_energy() || !need_payload() ||
        can_free_retreat_with_latias()) {
      return false;
    }

    // Project only the current public K1 state. A manual Basic Energy attachment
    // pays Oricorio's one-Colorless Retreat Cost, promotes the already-GGF Benched
    // Regidrago VSTAR, and leaves the unused Supporter action for Professor Burnet
    // to establish a permitted Dragon payload during this same ready turn:
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Latias ex / superior free-retreat route: https://api.pokemontcg.io/v2/cards/sv8-76
    // Official attachment, Retreat, Supporter, search, and discard procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, current-turn JIT, DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2158
    std::mt19937_64 shadow_rng = rng_;
    Engine projected(scenario_, recipe_, shadow_rng);
    projected.state_ = state_;
    projected.deck_seen_ = deck_seen_;
    projected.prizes_revealed_ = prizes_revealed_;
    projected.trace_ = nullptr;

    Pokemon* target = projected.best_benched_vstar_for_promotion();
    if (target == nullptr || target->grass < 2 || target->fire < 1) {
      return false;
    }

    const Card payment = projected.hand_count(Card::Grass) > 0
        ? Card::Grass
        : Card::Fire;
    if (!remove_one(projected.state_.hand, payment)) return false;
    if (payment == Card::Grass) {
      ++projected.state_.active->grass;
      --projected.state_.active->grass;
    } else {
      ++projected.state_.active->fire;
      --projected.state_.active->fire;
    }
    projected.state_.manual_energy_used = true;
    projected.state_.discard.push_back(payment);
    std::swap(*projected.state_.active, *target);
    projected.state_.retreat_used = true;
    return projected.professor_burnet_has_live_ready_turn_route();
  }

  bool play_issue_2158_paid_oricorio_retreat_burnet() {
    if (!issue_2158_paid_oricorio_retreat_burnet_available()) return false;

    Pokemon* target = best_benched_vstar_for_promotion();
    if (target == nullptr || target->grass < 2 || target->fire < 1) {
      throw std::logic_error(
          "Issue-2158 projected GGF promotion target disappeared");
    }
    const Card payment = hand_count(Card::Grass) > 0 ? Card::Grass : Card::Fire;
    if (!remove_one(state_.hand, payment)) {
      throw std::logic_error("Issue-2158 projected Retreat Energy disappeared");
    }

    // The attachment and Retreat are separate legal actions. Record the temporary
    // Energy on Oricorio, then discard that same Energy to pay its printed cost:
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Official attachment and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2158
    if (payment == Card::Grass) {
      ++state_.active->grass;
    } else {
      ++state_.active->fire;
    }
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; R-ORICORIO-01; P-COMPRESS-01",
          std::string(name(payment)) +
              " manually to Oricorio for its immediate Retreat Cost.");

    if (payment == Card::Grass) {
      --state_.active->grass;
    } else {
      --state_.active->fire;
    }
    state_.discard.push_back(payment);
    std::swap(*state_.active, *target);
    state_.retreat_used = true;
    trace("RETREAT", "R-GAME-RETREAT; R-ORICORIO-01; P-COMPRESS-01",
          "Paid Oricorio's one-Energy Retreat Cost and promoted the GGF Regidrago VSTAR before Professor Burnet.");
    return true;
  }

  bool attach_manual() {
    if (play_issue_2158_paid_oricorio_retreat_burnet()) return true;
    return attach_manual_issue2158_original();
  }
