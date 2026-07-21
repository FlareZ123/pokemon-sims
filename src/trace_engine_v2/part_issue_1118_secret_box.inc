
  bool secret_box_combo_enabled() const {
    return total_copies(Card::SecretBox) > 0 &&
           total_copies(Card::Pineco) > 0 &&
           total_copies(Card::ForretressEx) > 0;
  }

  bool secret_box_has_live_fss_holder() const {
    const auto live = [](const Pokemon& pokemon) {
      return pokemon.tool == Tool::ForestSealStone && is_pokemon_v(pokemon.card);
    };
    return !state_.vstar_power_used &&
        ((state_.active && live(*state_.active)) ||
         std::any_of(state_.bench.begin(), state_.bench.end(), live));
  }

  bool secret_box_regi_can_evolve_now() const {
    if (has_vstar()) return true;
    const auto eligible = [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV &&
             pokemon.entered_turn < state_.turn;
    };
    return (state_.active && eligible(*state_.active)) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), eligible);
  }

  bool secret_box_full_combo_axes_visible() const {
    if (!secret_box_combo_enabled() || item_locked() || state_.turn <= 1 ||
        !ability_available_for_pokemon(Card::ForretressEx) ||
        !secret_box_regi_can_evolve_now()) {
      return false;
    }

    const bool prior_turn_pineco =
        (state_.active && state_.active->card == Card::Pineco &&
         state_.active->entered_turn < state_.turn) ||
        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [this](const Pokemon& pokemon) {
          return pokemon.card == Card::Pineco &&
                 pokemon.entered_turn < state_.turn;
        });
    const bool pineco_in_play = in_play_count(Card::Pineco) > 0;
    const bool forretress_in_play = in_play_count(Card::ForretressEx) > 0;
    const bool forest_available = state_.stadium == Stadium::ForestOfVitality ||
        hand_count(Card::ForestOfVitality) > 0 ||
        might_be_unseen(Card::ForestOfVitality);
    const bool pineco_available = pineco_in_play ||
        hand_count(Card::Pineco) > 0 || might_be_unseen(Card::Pineco);
    const bool forretress_available = forretress_in_play ||
        hand_count(Card::ForretressEx) > 0 ||
        might_be_unseen(Card::ForretressEx);
    const bool dawn_available = hand_count(Card::Dawn) > 0 ||
        might_be_unseen(Card::Dawn);

    const bool existing_line = forretress_in_play ||
        ((prior_turn_pineco || (pineco_in_play && forest_available)) &&
         hand_count(Card::ForretressEx) > 0);
    const bool held_entry_line = !pineco_in_play && bench_space() > 0 &&
        hand_count(Card::Pineco) > 0 &&
        hand_count(Card::ForretressEx) > 0 && forest_available;
    const bool dawn_line = supporter_allowed() && dawn_available &&
        pineco_available && forretress_available && forest_available &&
        (pineco_in_play || bench_space() > 0);
    if (!existing_line && !held_entry_line && !dawn_line) return false;

    const bool vstar_held_or_play = has_vstar() ||
        hand_count(Card::RegidragoVstar) > 0;
    const bool vstar_available = vstar_held_or_play ||
        might_be_unseen(Card::RegidragoVstar);
    const bool payload_already = payload_ready();
    const bool payload_available = payload_already ||
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        payload_might_be_in_deck();
    const bool treasure_available = hand_count(Card::MysteriousTreasure) > 0 ||
        might_be_unseen(Card::MysteriousTreasure);
    const bool needs_treasure = !vstar_held_or_play || !payload_already;
    if (!vstar_available || !payload_available ||
        (needs_treasure && !treasure_available)) {
      return false;
    }

    const bool fire_held = hand_count(Card::Fire) > 0;
    const bool fire_searchable = might_be_unseen(Card::Fire);
    const bool fss_available = secret_box_has_live_fss_holder() ||
        hand_count(Card::ForestSealStone) > 0 ||
        might_be_unseen(Card::ForestSealStone);
    const bool fss_fire = !state_.vstar_power_used && fss_available &&
        fire_searchable;
    const bool crispin_fire = supporter_allowed() &&
        !state_.manual_energy_used && fire_searchable &&
        (hand_count(Card::Crispin) > 0 || might_be_unseen(Card::Crispin));
    const bool vessel_fire = !needs_treasure && !state_.manual_energy_used &&
        fire_searchable && (hand_count(Card::EarthenVessel) > 0 ||
                            might_be_unseen(Card::EarthenVessel));
    if (!fire_held && !fss_fire && !crispin_fire && !vessel_fire) return false;

    const bool grass_available = might_be_unseen(Card::Grass) ||
        grass_needed() <= 0;
    return grass_available;
  }

  void record_secret_box_missing_axes() {
    const bool eligible_regi = secret_box_regi_can_evolve_now();
    const bool ability = state_.turn > 1 &&
        ability_available_for_pokemon(Card::ForretressEx);

    const bool prior_turn_pineco =
        (state_.active && state_.active->card == Card::Pineco &&
         state_.active->entered_turn < state_.turn) ||
        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [this](const Pokemon& pokemon) {
          return pokemon.card == Card::Pineco &&
                 pokemon.entered_turn < state_.turn;
        });
    const bool pineco_in_play = in_play_count(Card::Pineco) > 0;
    const bool forretress_in_play = in_play_count(Card::ForretressEx) > 0;
    const bool forest_available = state_.stadium == Stadium::ForestOfVitality ||
        hand_count(Card::ForestOfVitality) > 0 ||
        might_be_unseen(Card::ForestOfVitality);
    const bool pineco_available = pineco_in_play ||
        hand_count(Card::Pineco) > 0 || might_be_unseen(Card::Pineco);
    const bool forretress_available = forretress_in_play ||
        hand_count(Card::ForretressEx) > 0 ||
        might_be_unseen(Card::ForretressEx);
    const bool dawn_available = hand_count(Card::Dawn) > 0 ||
        might_be_unseen(Card::Dawn);
    const bool existing_line = forretress_in_play ||
        ((prior_turn_pineco || (pineco_in_play && forest_available)) &&
         hand_count(Card::ForretressEx) > 0);
    const bool held_entry_line = !pineco_in_play && bench_space() > 0 &&
        hand_count(Card::Pineco) > 0 &&
        hand_count(Card::ForretressEx) > 0 && forest_available;
    const bool dawn_line = supporter_allowed() && dawn_available &&
        pineco_available && forretress_available && forest_available &&
        (pineco_in_play || bench_space() > 0);
    const bool line = existing_line || held_entry_line || dawn_line;

    const bool vstar_held_or_play = has_vstar() ||
        hand_count(Card::RegidragoVstar) > 0;
    const bool vstar = vstar_held_or_play ||
        might_be_unseen(Card::RegidragoVstar);
    const bool payload_already = payload_ready();
    const bool payload = payload_already ||
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        payload_might_be_in_deck();
    const bool needs_treasure = !vstar_held_or_play || !payload_already;
    const bool treasure = !needs_treasure ||
        hand_count(Card::MysteriousTreasure) > 0 ||
        might_be_unseen(Card::MysteriousTreasure);

    const bool fire_held = hand_count(Card::Fire) > 0;
    const bool fire_searchable = might_be_unseen(Card::Fire);
    const bool fss_available = secret_box_has_live_fss_holder() ||
        hand_count(Card::ForestSealStone) > 0 ||
        might_be_unseen(Card::ForestSealStone);
    const bool fss_fire = !state_.vstar_power_used && fss_available &&
        fire_searchable;
    const bool crispin_fire = supporter_allowed() &&
        !state_.manual_energy_used && fire_searchable &&
        (hand_count(Card::Crispin) > 0 || might_be_unseen(Card::Crispin));
    const bool vessel_fire = !needs_treasure && !state_.manual_energy_used &&
        fire_searchable && (hand_count(Card::EarthenVessel) > 0 ||
                            might_be_unseen(Card::EarthenVessel));
    const bool fire = fire_needed() <= 0 || fire_held || fss_fire ||
        crispin_fire || vessel_fire;
    const bool grass = grass_needed() <= 0 || might_be_unseen(Card::Grass);

    if (!eligible_regi) ++outcome_.secret_box_missing_regi_axis;
    if (!line) ++outcome_.secret_box_missing_line_axis;
    if (!vstar) ++outcome_.secret_box_missing_vstar_axis;
    if (!payload) ++outcome_.secret_box_missing_payload_axis;
    if (!treasure) ++outcome_.secret_box_missing_treasure_axis;
    if (!fire) ++outcome_.secret_box_missing_fire_axis;
    if (!grass) ++outcome_.secret_box_missing_grass_axis;
    if (!ability) ++outcome_.secret_box_missing_ability_axis;
    if ((!supporter_allowed() && !existing_line && !held_entry_line) ||
        (!supporter_allowed() && !fire_held && !fss_fire && !vessel_fire)) {
      ++outcome_.secret_box_missing_supporter_axis;
    }

    if (prizes_known()) {
      bool known_prize_block = false;
      if (!eligible_regi) known_prize_block |= known_prize(Card::RegidragoV);
      if (!line) {
        known_prize_block |= known_prize(Card::Pineco) ||
            known_prize(Card::ForretressEx) || known_prize(Card::Dawn) ||
            known_prize(Card::ForestOfVitality);
      }
      if (!vstar) known_prize_block |= known_prize(Card::RegidragoVstar);
      if (!payload) {
        known_prize_block |= std::any_of(state_.prizes.begin(),
                                          state_.prizes.end(), is_payload);
      }
      if (!treasure) {
        known_prize_block |= known_prize(Card::MysteriousTreasure);
      }
      if (!fire) {
        known_prize_block |= known_prize(Card::Fire) ||
            known_prize(Card::ForestSealStone) || known_prize(Card::Crispin) ||
            known_prize(Card::EarthenVessel);
      }
      if (!grass) known_prize_block |= known_prize(Card::Grass);
      if (known_prize_block) ++outcome_.secret_box_missing_known_prize_zone;
    }

    const auto discarded = [this](const Card card) {
      return std::find(state_.discard.begin(), state_.discard.end(), card) !=
          state_.discard.end();
    };
    bool discard_block = false;
    if (!eligible_regi) discard_block |= discarded(Card::RegidragoV);
    if (!line) {
      discard_block |= discarded(Card::Pineco) ||
          discarded(Card::ForretressEx) || discarded(Card::Dawn) ||
          discarded(Card::ForestOfVitality);
    }
    if (!vstar) discard_block |= discarded(Card::RegidragoVstar);
    if (!payload) {
      discard_block |= std::any_of(state_.discard.begin(), state_.discard.end(),
                                   is_payload);
    }
    if (!treasure) discard_block |= discarded(Card::MysteriousTreasure);
    if (!fire) {
      discard_block |= discarded(Card::Fire) ||
          discarded(Card::ForestSealStone) || discarded(Card::Crispin) ||
          discarded(Card::EarthenVessel);
    }
    if (!grass) discard_block |= discarded(Card::Grass);
    if (discard_block) ++outcome_.secret_box_missing_discard_zone;

    const bool stranded_line_hand = !line &&
        (hand_count(Card::Pineco) > 0 || hand_count(Card::ForretressEx) > 0 ||
         hand_count(Card::Dawn) > 0 ||
         hand_count(Card::ForestOfVitality) > 0);
    const bool stranded_fire_hand = !fire &&
        (hand_count(Card::Fire) > 0 || hand_count(Card::ForestSealStone) > 0 ||
         hand_count(Card::Crispin) > 0 ||
         hand_count(Card::EarthenVessel) > 0);
    const bool stranded_search_hand = (!vstar || !payload || !treasure) &&
        (hand_count(Card::RegidragoVstar) > 0 ||
         hand_count(Card::MysteriousTreasure) > 0 ||
         std::any_of(state_.hand.begin(), state_.hand.end(), is_payload));
    if (stranded_line_hand || stranded_fire_hand || stranded_search_hand) {
      ++outcome_.secret_box_missing_stranded_hand_zone;
    }
  }

  int secret_box_cost_score(const Card card,
                            const std::map<Card, int>& remaining) const {
    if (card == Card::SecretBox) return -1;
    const auto copies = [&remaining](const Card target) {
      const auto found = remaining.find(target);
      return found == remaining.end() ? 0 : found->second;
    };

    // Grant and Wishful Baton are the preferred source-bound costs. Grant can
    // recover itself from discard by paying its separate printed effect, while
    // Wishful Baton has no modeled goldfish setup value:
    // https://api.pokemontcg.io/v2/cards/swsh10-144
    // https://api.pokemontcg.io/v2/cards/sm3-128
    if (card == Card::Grant) return 0;
    if (card == Card::WishfulBaton) return 1;
    if (card == Card::ErikasInvitation) return 2;

    // Physical duplicates retain the same connector after one copy is spent.
    if (copies(card) >= 2 &&
        (card == Card::TapuLeleGX || card == Card::Dawn ||
         card == Card::ForestOfVitality || card == Card::Pineco ||
         card == Card::ForretressEx || card == Card::MysteriousTreasure ||
         card == Card::QuickBall || card == Card::EarthenVessel ||
         card == Card::Arven || card == Card::Gladion ||
         card == Card::RegidragoVstar || card == Card::Grass ||
         card == Card::Fire || is_payload(card))) {
      return 3;
    }

    // Route pieces become dynamically discardable when the public state or the
    // same Secret Box/Dawn chain replaces their role. This is the DCI distinction
    // between a protected singleton and a physically redundant or already-used
    // axis:
    // https://github.com/FlareZ123/pokemon-sims/issues/1118
    if (card == Card::Pineco &&
        (in_play_count(Card::Pineco) > 0 ||
         in_play_count(Card::ForretressEx) > 0)) return 4;
    if (card == Card::ForretressEx &&
        in_play_count(Card::ForretressEx) > 0) return 4;
    if (is_payload(card) && can_play_payload_this_turn()) return 4;
    if (card == Card::ForestOfVitality &&
        (state_.stadium == Stadium::ForestOfVitality ||
         in_play_count(Card::Pineco) > 0)) return 4;
    if (card == Card::Dawn &&
        in_play_count(Card::Pineco) > 0 &&
        hand_count(Card::ForretressEx) > 0) return 4;

    if (card == Card::FieldBlower && !rule_box_abilities_locked()) return 4;
    if (card == Card::Guzma || card == Card::ProfessorTuro) return 5;
    if (card == Card::ProfessorBurnet && payload_might_be_in_deck()) return 6;

    // A route-complete strict-JIT hand may spend a connector whose only setup
    // role is already supplied by another public axis. In particular, Earthen
    // Vessel is dynamically dead when Fire is held or the live FSS channel can
    // search Fire. Quick Ball is likewise redundant after Regidrago is already
    // established and the Pineco line is visible. These are replacement proofs,
    // not generic permission to spend protected singletons:
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://github.com/FlareZ123/pokemon-sims/issues/1118
    if (secret_box_full_combo_axes_visible()) {
      const bool fire_already_covered = hand_count(Card::Fire) > 0 ||
          (!state_.vstar_power_used && might_be_unseen(Card::Fire) &&
           (secret_box_has_live_fss_holder() ||
            hand_count(Card::ForestSealStone) > 0 ||
            might_be_unseen(Card::ForestSealStone)));
      if (card == Card::EarthenVessel && fire_already_covered) return 7;
      if (card == Card::QuickBall && has_any_regi()) return 7;
      if (card == Card::ForestOfVitality &&
          ((state_.active && state_.active->card == Card::Pineco &&
            state_.active->entered_turn < state_.turn) ||
           std::any_of(state_.bench.begin(), state_.bench.end(),
                       [this](const Pokemon& pokemon) {
             return pokemon.card == Card::Pineco &&
                    pokemon.entered_turn < state_.turn;
           }))) return 7;
      const bool prior_turn_pineco =
          (state_.active && state_.active->card == Card::Pineco &&
           state_.active->entered_turn < state_.turn) ||
          std::any_of(state_.bench.begin(), state_.bench.end(),
                      [this](const Pokemon& pokemon) {
            return pokemon.card == Card::Pineco &&
                   pokemon.entered_turn < state_.turn;
          });
      const bool existing_line = in_play_count(Card::ForretressEx) > 0 ||
          ((prior_turn_pineco ||
            (in_play_count(Card::Pineco) > 0 &&
             state_.stadium == Stadium::ForestOfVitality)) &&
           hand_count(Card::ForretressEx) > 0);
      const bool held_entry_line = in_play_count(Card::Pineco) == 0 &&
          hand_count(Card::Pineco) > 0 &&
          hand_count(Card::ForretressEx) > 0 &&
          (state_.stadium == Stadium::ForestOfVitality ||
           hand_count(Card::ForestOfVitality) > 0 ||
           might_be_unseen(Card::ForestOfVitality));
      const bool payload_without_dawn = payload_ready() ||
          std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
      const bool dawn_required = !(existing_line || held_entry_line) ||
          !payload_without_dawn;
      const bool fire_without_crispin = hand_count(Card::Fire) > 0 ||
          (!state_.vstar_power_used && might_be_unseen(Card::Fire) &&
           (secret_box_has_live_fss_holder() ||
            hand_count(Card::ForestSealStone) > 0 ||
            might_be_unseen(Card::ForestSealStone)));

      // Dawn owns the one-Supporter slot whenever it supplies a missing line or
      // strict-JIT payload. Other Supporters are then physically dead costs. If
      // Dawn is unnecessary, any Supporter except the uniquely required Crispin
      // is likewise unused by the proven current-turn route.
      if (is_supporter(card)) {
        if (card != Card::Dawn && card != Card::Crispin) return 7;
        if (card == Card::Dawn && !dawn_required) return 7;
        if (card == Card::Crispin && fire_without_crispin) return 7;
      }

      // Exploding Energy is the route's Grass source. A hand Grass is surplus
      // once the full line, Bench access, and Ability legality are proven.
      if (card == Card::Grass) return 7;

      // With VSTAR already held, the Box's independent Item category can take
      // Quick Ball to discard the Dawn-supplied Dragon. A held Treasure is then
      // a replaceable cost rather than an irreplaceable connector.
      if (card == Card::MysteriousTreasure &&
          (has_vstar() || hand_count(Card::RegidragoVstar) > 0) &&
          (hand_count(Card::QuickBall) > 0 || might_be_unseen(Card::QuickBall)) &&
          (payload_without_dawn || hand_count(Card::Dawn) > 0 ||
           might_be_unseen(Card::Dawn))) return 7;
    }

    // In the no-discard-control profile, a complete shadow-proven Secret Box
    // route may spend an otherwise protected physical axis only when the route
    // has a public replacement channel. The Item search replaces Treasure, FSS
    // replaces Fire, Treasure replaces VSTAR, Dawn replaces its three Pokémon
    // categories, and the Supporter/Stadium searches replace Dawn and Forest.
    // A sole irreplaceable copy remains protected. Strict-JIT therefore keeps
    // the exact-state regression in which VSTAR is the third physical card:
    // https://api.pokemontcg.io/v2/cards/sv6-163
    // https://api.pokemontcg.io/v2/cards/me2-87
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://github.com/FlareZ123/pokemon-sims/issues/1118
    if (scenario_.dci == DciProfile::NoDiscardControl &&
        secret_box_full_combo_axes_visible()) {
      if (card == Card::MysteriousTreasure &&
          (copies(card) >= 2 || might_be_unseen(card))) return 8;
      if (card == Card::RegidragoVstar &&
          (copies(card) >= 2 || might_be_unseen(card))) return 8;
      if (card == Card::Fire &&
          (copies(card) >= 2 || might_be_unseen(card))) return 8;
      if (card == Card::Dawn &&
          (copies(card) >= 2 || might_be_unseen(card))) return 8;
      if (card == Card::ForestOfVitality &&
          (state_.stadium == Stadium::ForestOfVitality ||
           copies(card) >= 2 || might_be_unseen(card))) return 8;
      const bool fire_without_supporter = hand_count(Card::Fire) > 0 ||
          (!state_.vstar_power_used &&
           (secret_box_has_live_fss_holder() ||
            hand_count(Card::ForestSealStone) > 0 ||
            might_be_unseen(Card::ForestSealStone)) &&
           might_be_unseen(Card::Fire));
      if (card == Card::Pineco &&
          (in_play_count(Card::Pineco) > 0 ||
           in_play_count(Card::ForretressEx) > 0 || copies(card) >= 2 ||
           (might_be_unseen(card) && fire_without_supporter))) return 8;
      if (card == Card::ForretressEx &&
          (in_play_count(Card::ForretressEx) > 0 || copies(card) >= 2 ||
           (might_be_unseen(card) && fire_without_supporter))) return 8;
      if (is_payload(card) && payload_might_be_in_deck()) return 8;
    }

    // Energy above the current public GGF requirement is dynamically discardable.
    if (card == Card::Grass && copies(Card::Grass) > grass_needed() + 1) return 7;
    if (card == Card::Fire && copies(Card::Fire) > fire_needed() + 1) return 7;

    // Matchup-flex and no-control profiles admit additional inert goldfish roles.
    if (scenario_.dci != DciProfile::StrictJit &&
        (card == Card::FieldBlower || card == Card::Guzma ||
         card == Card::ProfessorTuro || card == Card::ProfessorBurnet ||
         card == Card::HisuianHeavyBall)) {
      return 10;
    }

    if (scenario_.dci == DciProfile::NoDiscardControl) {
      if (is_payload(card)) return 20;
      if (card != Card::RegidragoVstar && card != Card::ForestSealStone &&
          card != Card::Dawn && card != Card::ForestOfVitality &&
          card != Card::Pineco && card != Card::ForretressEx) {
        return 25;
      }
    }
    return -1;
  }

  std::optional<std::array<Card, 3>> secret_box_cost_plan() const {
    std::map<Card, int> remaining;
    for (const Card card : state_.hand) ++remaining[card];
    if (remaining[Card::SecretBox] == 0) return std::nullopt;
    --remaining[Card::SecretBox];

    std::array<Card, 3> result{};
    for (std::size_t slot = 0; slot < result.size(); ++slot) {
      int best_score = 1000000;
      std::optional<Card> best;
      for (const auto& [card, copies] : remaining) {
        if (copies <= 0) continue;
        const int score = secret_box_cost_score(card, remaining);
        if (score >= 0 && score < best_score) {
          best_score = score;
          best = card;
        }
      }
      if (!best) return std::nullopt;
      result[slot] = *best;
      --remaining[*best];
    }
    return result;
  }

  bool play_secret_box() {
    if (!secret_box_combo_enabled() || item_locked() ||
        hand_count(Card::SecretBox) == 0 || !setup_axis_missing()) {
      return false;
    }

    ++outcome_.secret_box_combo_attempted;
    if (bench_space() == 0 && in_play_count(Card::Pineco) == 0 &&
        in_play_count(Card::ForretressEx) == 0) {
      ++outcome_.secret_box_bench_blocked;
      return false;
    }
    if (!secret_box_full_combo_axes_visible()) {
      ++outcome_.secret_box_missing_axis;
      record_secret_box_missing_axes();
      return false;
    }
    const auto costs = secret_box_cost_plan();
    if (!costs) {
      ++outcome_.secret_box_cost_blocked;
      trace("HOLD ITEM", "R-SECRET-BOX-01; P-DCI-01",
            "Secret Box was held because DCI did not admit three other costs.");
      return false;
    }

    remove_one(state_.hand, Card::SecretBox);
    for (const Card cost : *costs) {
      if (!discard_from_hand(cost, "Secret Box cost",
                             "R-SECRET-BOX-01; P-DCI-01")) {
        throw std::logic_error("Secret Box cost plan diverged from hand");
      }
    }
    state_.discard.push_back(Card::SecretBox);
    outcome_.used_secret_box = true;
    record_deck_search_knowledge("Secret Box");

    std::vector<Card> selected;
    const auto take = [this, &selected](const Card card) {
      if (move_deck_to_hand(card)) selected.push_back(card);
    };

    // Secret Box independently searches up to one Item, Tool, Supporter,
    // and Stadium. Select only the missing channels in the observable route.
    // A held VSTAR plus an existing payload can use Vessel instead of Treasure;
    // a prior-turn Pineco line can use Crispin instead of Dawn; held Fire makes
    // FSS optional. The printed searches themselves are optional:
    // https://api.pokemontcg.io/v2/cards/sv6-163
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/me2-87
    // https://api.pokemontcg.io/v2/cards/me1-117
    const bool prior_turn_pineco =
        (state_.active && state_.active->card == Card::Pineco &&
         state_.active->entered_turn < state_.turn) ||
        std::any_of(state_.bench.begin(), state_.bench.end(),
                    [this](const Pokemon& pokemon) {
          return pokemon.card == Card::Pineco &&
                 pokemon.entered_turn < state_.turn;
        });
    const bool existing_line = in_play_count(Card::ForretressEx) > 0 ||
        ((prior_turn_pineco ||
          (in_play_count(Card::Pineco) > 0 &&
           state_.stadium == Stadium::ForestOfVitality)) &&
         hand_count(Card::ForretressEx) > 0);
    const bool held_entry_line = in_play_count(Card::Pineco) == 0 &&
        hand_count(Card::Pineco) > 0 && hand_count(Card::ForretressEx) > 0 &&
        (state_.stadium == Stadium::ForestOfVitality ||
         hand_count(Card::ForestOfVitality) > 0);
    const bool needs_dawn = !existing_line && !held_entry_line;
    const bool needs_forest = !prior_turn_pineco &&
        in_play_count(Card::ForretressEx) == 0 &&
        state_.stadium != Stadium::ForestOfVitality &&
        hand_count(Card::ForestOfVitality) == 0;
    const bool vstar_held_or_play = has_vstar() ||
        hand_count(Card::RegidragoVstar) > 0;
    const bool needs_treasure = !vstar_held_or_play || !payload_ready();
    const bool needs_fire = fire_needed() > 0 && hand_count(Card::Fire) == 0;
    const bool can_use_fss_for_fire = needs_fire && !state_.vstar_power_used &&
        (secret_box_has_live_fss_holder() ||
         hand_count(Card::ForestSealStone) > 0 ||
         might_be_unseen(Card::ForestSealStone));

    if (needs_treasure && hand_count(Card::MysteriousTreasure) == 0) {
      take(Card::MysteriousTreasure);
    } else if (needs_fire && hand_count(Card::EarthenVessel) == 0) {
      take(Card::EarthenVessel);
    } else if (hand_count(Card::MysteriousTreasure) == 0) {
      take(Card::MysteriousTreasure);
    } else {
      take(Card::QuickBall);
    }

    if (can_use_fss_for_fire && hand_count(Card::ForestSealStone) == 0) {
      take(Card::ForestSealStone);
    } else {
      take(Card::WishfulBaton);
    }

    if (needs_dawn && hand_count(Card::Dawn) == 0) {
      take(Card::Dawn);
    } else if (needs_fire && !can_use_fss_for_fire &&
               hand_count(Card::Crispin) == 0) {
      take(Card::Crispin);
    } else if (hand_count(Card::Dawn) == 0) {
      take(Card::Dawn);
    } else {
      take(Card::Arven);
    }

    if (needs_forest) take(Card::ForestOfVitality);

    shuffle(state_.deck);
    trace("PLAY ITEM", "R-SECRET-BOX-01; R-GAME-ITEM",
          "Secret Box discarded three other cards and searched: " +
              join_cards(selected) + ".");
    return true;
  }

  bool issue_1304_t1_treasure_erika_cost_available() const {
    // This exception is limited to the source-bound, observable T1 route. Erika's
    // Invitation has no setup effect in this opponent-free model, while the paid
    // Treasure search establishes Regidrago V one turn before it must evolve:
    // https://api.pokemontcg.io/v2/cards/sv3pt5-160
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://api.pokemontcg.io/v2/cards/me1-117
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/1304
    const bool pineco_in_play = in_play_count(Card::Pineco) > 0;
    const bool payload_held = std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    return secret_box_combo_enabled() && state_.turn == 1 && scenario_.going_first &&
        strict_payload_timing() && scenario_.max_turn >= 2 && !item_locked() &&
        !rule_box_abilities_locked() && !state_.vstar_power_used && pineco_in_play &&
        bench_space() > 0 && !has_any_regi() && hand_count(Card::RegidragoV) == 0 &&
        hand_count(Card::MysteriousTreasure) > 0 &&
        hand_count(Card::ErikasInvitation) > 0 &&
        hand_count(Card::ForestSealStone) > 0 &&
        hand_count(Card::ForretressEx) > 0 &&
        (state_.stadium == Stadium::ForestOfVitality ||
         hand_count(Card::ForestOfVitality) > 0) &&
        payload_held && might_be_unseen(Card::RegidragoV) &&
        might_be_unseen(Card::RegidragoVstar) && might_be_unseen(Card::Fire) &&
        might_be_unseen(Card::Grass);
  }

  bool issue_1304_t1_treasure_fss_route_established() const {
    // After Treasure resolves, K1 may verify the remaining route pieces without
    // reading the post-shuffle top card. Star Alchemy is held as the unique direct
    // Fire channel; the remaining Treasure is only known to exist somewhere in deck:
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // https://github.com/FlareZ123/pokemon-sims/issues/1304
    const bool payload_held = std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
    return state_.turn == 1 && scenario_.going_first && strict_payload_timing() &&
        scenario_.max_turn >= 2 && !rule_box_abilities_locked() && deck_seen_ &&
        count_of(state_.discard, Card::MysteriousTreasure) > 0 &&
        count_of(state_.discard, Card::ErikasInvitation) > 0 &&
        has_any_regi() && in_play_count(Card::Pineco) > 0 &&
        hand_count(Card::ForretressEx) > 0 && payload_held &&
        (state_.stadium == Stadium::ForestOfVitality ||
         hand_count(Card::ForestOfVitality) > 0) &&
        might_be_unseen(Card::MysteriousTreasure) &&
        might_be_unseen(Card::RegidragoVstar) && might_be_unseen(Card::Fire) &&
        might_be_unseen(Card::Grass);
  }

  bool hold_fss_for_next_turn_secret_box() const {
    const bool issue_1304_route = issue_1304_t1_treasure_fss_route_established();
    if (!secret_box_combo_enabled() || state_.turn != 1 ||
        !secret_box_has_live_fss_holder() || fire_needed() == 0 ||
        (hand_count(Card::SecretBox) == 0 && !issue_1304_route)) {
      return false;
    }

    const bool regi_established = has_any_regi();
    const bool vstar_channel = hand_count(Card::RegidragoVstar) > 0 ||
        hand_count(Card::MysteriousTreasure) > 0 ||
        might_be_unseen(Card::MysteriousTreasure);
    const bool pineco_channel = in_play_count(Card::Pineco) > 0 ||
        in_play_count(Card::ForretressEx) > 0 ||
        hand_count(Card::Pineco) > 0 || might_be_unseen(Card::Pineco);
    const bool forretress_channel = in_play_count(Card::ForretressEx) > 0 ||
        hand_count(Card::ForretressEx) > 0 ||
        might_be_unseen(Card::ForretressEx);
    const bool dawn_or_held_line =
        (in_play_count(Card::Pineco) > 0 &&
         (hand_count(Card::ForretressEx) > 0 ||
          might_be_unseen(Card::ForretressEx))) ||
        hand_count(Card::Dawn) > 0 || might_be_unseen(Card::Dawn);

    // Secret Box can fetch Mysterious Treasure on T2, while the attached Forest
    // Seal Stone is the route's unique direct Fire channel. Spending Star Alchemy
    // on the VSTAR card during T1 destroys that division of labor. Hold the Power
    // only when the observable next-turn route still has Regidrago, Pineco,
    // Forretress, and VSTAR connectors:
    // https://api.pokemontcg.io/v2/cards/sv6-163
    // https://api.pokemontcg.io/v2/cards/swsh12-156
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://github.com/FlareZ123/pokemon-sims/issues/1118
    return regi_established && vstar_channel && pineco_channel &&
        forretress_channel && dawn_or_held_line;
  }

  bool use_fss_for_secret_box_bank() {
    if (!secret_box_combo_enabled() || !secret_box_has_live_fss_holder() ||
        hand_count(Card::SecretBox) > 0 || !might_be_unseen(Card::SecretBox) ||
        state_.turn >= scenario_.max_turn) {
      return false;
    }

    record_deck_search_knowledge("Star Alchemy");
    if (!move_deck_to_hand(Card::SecretBox)) {
      shuffle(state_.deck);
      return false;
    }
    shuffle(state_.deck);
    state_.vstar_power_used = true;
    outcome_.used_fss = true;
    ++outcome_.secret_box_fss_bank;
    trace("USE VSTAR POWER", "R-FSS-01; R-SECRET-BOX-01",
          "Star Alchemy searched Secret Box for the next payable combo turn.");
    return true;
  }

  bool use_fss_for_secret_box_fire() {
    if (!secret_box_combo_enabled() || !secret_box_has_live_fss_holder() ||
        fire_needed() == 0 || !might_be_unseen(Card::Fire)) {
      return false;
    }
    record_deck_search_knowledge("Star Alchemy");
    if (!move_deck_to_hand(Card::Fire)) {
      shuffle(state_.deck);
      return false;
    }
    shuffle(state_.deck);
    state_.vstar_power_used = true;
    outcome_.used_fss = true;
    trace("USE VSTAR POWER", "R-FSS-01; R-GAME-ENERGY",
          "Star Alchemy searched Basic Fire Energy for the Secret Box route.");
    return true;
  }

  bool play_arven_for_secret_box_bank() {
    if (!secret_box_combo_enabled() || !supporter_allowed() ||
        hand_count(Card::Arven) == 0 || hand_count(Card::SecretBox) > 0 ||
        !might_be_unseen(Card::SecretBox)) {
      return false;
    }

    remove_one(state_.hand, Card::Arven);
    state_.discard.push_back(Card::Arven);
    state_.supporter_used = true;
    record_deck_search_knowledge("Arven");
    std::vector<Card> selected;
    if (move_deck_to_hand(Card::SecretBox)) selected.push_back(Card::SecretBox);
    if (!state_.vstar_power_used && hand_count(Card::ForestSealStone) == 0 &&
        move_deck_to_hand(Card::ForestSealStone)) {
      selected.push_back(Card::ForestSealStone);
    }
    shuffle(state_.deck);
    ++outcome_.secret_box_arven_bank;
    trace("PLAY SUPPORTER", "R-ARVEN-01; R-SECRET-BOX-01",
          "Arven banked the next-turn Secret Box route: " +
              join_cards(selected) + ".");
    return true;
  }

  bool play_known_prized_secret_box_gladion() {
    if (!secret_box_combo_enabled() || !supporter_allowed() ||
        hand_count(Card::Gladion) == 0 || !known_prize(Card::SecretBox)) {
      return false;
    }
    remove_one(state_.hand, Card::Gladion);
    if (!remove_one(state_.prizes, Card::SecretBox)) {
      throw std::logic_error("Known prized Secret Box disappeared");
    }
    state_.hand.push_back(Card::SecretBox);
    state_.prizes.push_back(Card::Gladion);
    shuffle(state_.prizes);
    state_.supporter_used = true;
    ++outcome_.secret_box_gladion_bank;
    trace("PLAY SUPPORTER", "R-GLADION-01; R-SECRET-BOX-01",
          "Gladion recovered the legally known prized Secret Box.");
    return true;
  }

  bool play_steven_for_secret_box_bank() {
    if (!secret_box_combo_enabled() || !supporter_allowed() ||
        hand_count(Card::StevensResolve) == 0 ||
        state_.turn >= scenario_.max_turn ||
        (!has_any_regi() && hand_count(Card::RegidragoV) == 0)) {
      return false;
    }

    const bool secret_box_available = hand_count(Card::SecretBox) > 0 ||
        might_be_unseen(Card::SecretBox);
    if (!secret_box_available) return false;

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    state_.turn_ended = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve");

    std::vector<Card> selected;
    const auto take = [this, &selected](const Card card) {
      if (selected.size() < 3 && move_deck_to_hand(card)) selected.push_back(card);
    };
    if (hand_count(Card::SecretBox) == 0) take(Card::SecretBox);

    // Search low-DCI physical costs first, then missing next-turn axes.
    for (const Card card : {Card::Grant, Card::WishfulBaton,
                            Card::ErikasInvitation, Card::ForestOfVitality,
                            Card::Dawn, Card::MysteriousTreasure}) {
      if (selected.size() >= 3) break;
      take(card);
    }
    shuffle(state_.deck);
    ++outcome_.secret_box_steven_bank;
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-SECRET-BOX-01; P-DCI-01",
          "Steven's Resolve banked: " + join_cards(selected) +
              "; the turn ended.");
    return true;
  }

  bool bench_tapu_for_secret_box_connector() {
    if (!secret_box_combo_enabled() || bench_space() <= 0 ||
        hand_count(Card::TapuLeleGX) == 0 ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        state_.supporter_used || hand_count(Card::SecretBox) > 0) {
      return false;
    }

    const bool secret_box_in_deck = might_be_unseen(Card::SecretBox);
    const bool known_prized_box = known_prize(Card::SecretBox);
    if (!secret_box_in_deck && !known_prized_box) return false;

    if (!bench_from_hand(Card::TapuLeleGX, false)) return false;
    record_deck_search_knowledge("Wonder Tag");
    Card target = Card::Arven;
    if (known_prized_box && deck_count_after_search_started(Card::Gladion) > 0) {
      target = Card::Gladion;
    } else if (deck_count_after_search_started(Card::Arven) == 0 &&
               deck_count_after_search_started(Card::StevensResolve) > 0) {
      target = Card::StevensResolve;
    }
    const bool found = move_deck_to_hand(target);
    shuffle(state_.deck);
    trace("USE ABILITY", "R-TAPU-01; R-SECRET-BOX-01",
          found ? "Wonder Tag searched " + std::string(name(target)) +
                      " for the Secret Box assembly chain."
                : "Wonder Tag found no Secret Box assembly Supporter.");
    return true;
  }

  bool proactive_tapu_retreat_for_pineco() {
    if (!secret_box_combo_enabled() || state_.turn != 1 ||
        state_.retreat_used || state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX || hand_count(Card::Grass) == 0) {
      return false;
    }
    auto target = std::find_if(state_.bench.begin(), state_.bench.end(),
                               [](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV;
    });
    if (target == state_.bench.end()) return false;

    remove_one(state_.hand, Card::Grass);
    ++state_.active->grass;
    state_.manual_energy_used = true;
    --state_.active->grass;
    state_.discard.push_back(Card::Grass);
    std::swap(*state_.active, *target);
    state_.retreat_used = true;
    trace("RETREAT", "R-GAME-RETREAT; R-TAPU-01; R-FORRETRESS-01",
          "Attached Grass to Active Tapu Lele-GX, paid its one-Energy Retreat "
          "Cost, and promoted Regidrago V for the next-turn Forretress route.");
    return true;
  }

  bool play_secret_box_treasure_vstar_completion() {
    if (!secret_box_combo_enabled() || item_locked() || need_energy() ||
        !need_vstar() || hand_count(Card::MysteriousTreasure) == 0 ||
        !secret_box_regi_can_evolve_now() ||
        !might_be_unseen(Card::RegidragoVstar)) {
      return false;
    }

    std::optional<Card> cost;
    if (need_payload()) {
      const auto payload = std::find_if(state_.hand.begin(), state_.hand.end(),
                                        is_payload);
      if (payload != state_.hand.end()) cost = *payload;
    }
    if (!cost && state_.supporter_used) {
      const auto dead_supporter = std::find_if(
          state_.hand.begin(), state_.hand.end(), is_supporter);
      if (dead_supporter != state_.hand.end()) cost = *dead_supporter;
    }
    if (!cost && grass_needed() == 0 && hand_count(Card::Grass) > 0) {
      cost = Card::Grass;
    }
    if (!cost && fire_needed() == 0 && hand_count(Card::Fire) > 0) {
      cost = Card::Fire;
    }
    if (!cost && payload_ready()) {
      const auto redundant_payload = std::find_if(
          state_.hand.begin(), state_.hand.end(), is_payload);
      if (redundant_payload != state_.hand.end()) cost = *redundant_payload;
    }
    if (!cost) {
      for (const Card candidate : {Card::FieldBlower, Card::Guzma,
                                   Card::ProfessorTuro, Card::ErikasInvitation,
                                   Card::Grant, Card::WishfulBaton}) {
        if (hand_count(candidate) > 0) {
          cost = candidate;
          break;
        }
      }
    }
    if (!cost) return false;

    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    if (!discard_from_hand(*cost, "Mysterious Treasure completion cost",
                           "R-MT-01; R-SECRET-BOX-01; P-DCI-01")) {
      throw std::logic_error("Secret Box Treasure completion cost vanished");
    }
    record_deck_search_knowledge("Mysterious Treasure");
    const bool found = move_deck_to_hand(Card::RegidragoVstar);
    shuffle(state_.deck);
    if (!found) return false;
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; R-SECRET-BOX-01",
          "Mysterious Treasure used the proven dead or payload cost and "
          "searched Regidrago VSTAR after GGF became complete.");
    return true;
  }

  bool play_secret_box_planned_supporter() {
    if (play_known_prized_secret_box_gladion()) return true;
    if (play_arven_for_secret_box_bank()) return true;
    if (play_steven_for_secret_box_bank()) return true;
    return false;
  }

  void play_secret_box_t1_search_items() {
    if (state_.turn_ended) return;
    for (int guard = 0; guard < 32; ++guard) {
      if (item_locked() || !setup_axis_missing()) break;
      if (play_heavy_ball()) continue;
      if (!run_search_items_one_step(false)) break;
      play_basics_from_hand();
    }
  }

  void run_secret_box_turn() {
    if (state_.turn == 2) {
      const auto eligible_regi = [this](const Pokemon& pokemon) {
        return pokemon.card == Card::RegidragoV &&
            pokemon.entered_turn < state_.turn;
      };
      outcome_.t2_eligible_regi_start =
          (state_.active && eligible_regi(*state_.active)) ||
          std::any_of(state_.bench.begin(), state_.bench.end(), eligible_regi);
      outcome_.t2_active_regi_start = state_.active &&
          (state_.active->card == Card::RegidragoV ||
           state_.active->card == Card::RegidragoVstar);
      outcome_.t2_pineco_line_start =
          in_play_count(Card::Pineco) > 0 ||
          in_play_count(Card::ForretressEx) > 0;
      outcome_.t2_vstar_held_start = hand_count(Card::RegidragoVstar) > 0;
    }
    trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01",
          "Start Pineco policy: " + state_line());
    play_chaotic_swell();
    play_field_blower();

    if (state_.turn == 1) {
      // Establish Regidrago before spending the Supporter. In particular, a held
      // Mysterious Treasure may find Regidrago V so Steven can bank the complete
      // T2 Secret Box package instead of Dawn consuming the T1 Supporter slot:
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://api.pokemontcg.io/v2/cards/sm7-145
      // https://github.com/FlareZ123/pokemon-sims/issues/1118
      play_basics_from_hand();
      play_secret_box_t1_search_items();
      play_basics_from_hand();
      proactive_tapu_retreat_for_pineco();
      attach_manual();

      if (!state_.supporter_used && !play_secret_box_planned_supporter()) {
        advance_forretress_combo();
        if (!state_.supporter_used) choose_supporter();
      }
      if (state_.turn_ended) {
        trace("TURN END", "R-STEVEN-01", state_line());
        return;
      }

      play_basics_from_hand();
      play_secret_box_t1_search_items();
      attach_live_fss();
      if (hold_fss_for_next_turn_secret_box()) {
        trace("HOLD VSTAR POWER", "R-FSS-01; R-SECRET-BOX-01; P-AXIS-01",
              "Held Star Alchemy as the next-turn Secret Box Fire channel.");
      } else if (!use_fss_for_secret_box_bank()) {
        use_fss();
      }
      advance_forretress_combo();
      use_celestial_roar();
      trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01",
            "End Pineco policy: " + state_line());
      return;
    }

    // Secret Box must be evaluated before Dawn/Forretress consumes or removes the
    // physical cards that can legally pay its three-card cost. The selected chain
    // then replenishes Pineco, Forretress, payload, and search axes as required.
    // https://api.pokemontcg.io/v2/cards/sv6-163
    // https://github.com/FlareZ123/pokemon-sims/issues/1118
    if (play_secret_box()) advance_forretress_combo();
    advance_forretress_combo();
    play_secret_box_treasure_vstar_completion();
    bench_tapu_for_secret_box_connector();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());

    attach_live_fss();
    if (!use_fss_for_secret_box_fire()) {
      if (!use_fss_for_secret_box_bank()) use_fss();
    }

    advance_forretress_combo();
    play_secret_box_treasure_vstar_completion();
    bench_tapu_for_secret_box_connector();
    play_basics_from_hand();
    if (play_secret_box()) advance_forretress_combo();
    play_secret_box_treasure_vstar_completion();
    play_items_until_stable(!strict_payload_timing());

    proactive_tapu_retreat_for_pineco();
    attach_manual();
    play_secret_box_treasure_vstar_completion();

    // A complete direct Regidrago route outranks banking a future Pineco route.
    // This is especially important when a prior-turn Regidrago already has GGF,
    // its VSTAR is held, and a no-discard-control payload is already in discard.
    // Steven's Resolve ends the turn, so playing it before the legal evolution
    // would convert a T2 ready state into a T3 result:
    // https://api.pokemontcg.io/v2/cards/sm7-145
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/1118
    evolve_best_regi();
    resolve_final_promotion_and_attachment();
    if (active_is_vstar() && !need_energy() && !need_payload()) {
      trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01",
            "Direct Regidrago route completed before future-route banking: " +
                state_line());
      return;
    }

    if (!state_.supporter_used && need_energy() &&
        hand_count(Card::Crispin) > 0 && play_crispin()) {
      trace("POLICY", "R-CRISPIN-01; R-SECRET-BOX-01; P-AXIS-01",
            "Used the Secret Box searched Crispin before lower-impact Prize "
            "or future-route Supporters.");
    } else if (!state_.supporter_used && !play_secret_box_planned_supporter()) {
      choose_supporter();
    }
    if (state_.turn_ended) {
      trace("TURN END", "R-STEVEN-01", state_line());
      return;
    }

    advance_forretress_combo();
    play_secret_box_treasure_vstar_completion();
    bench_tapu_for_secret_box_connector();
    play_basics_from_hand();
    attach_live_fss();
    if (!use_fss_for_secret_box_fire()) use_fss();
    if (play_secret_box()) advance_forretress_combo();
    play_items_until_stable(!strict_payload_timing());

    attach_manual();
    play_secret_box_treasure_vstar_completion();
    evolve_best_regi();
    advance_forretress_combo();
    play_items_until_stable(true);
    evolve_best_regi();

    if (!state_.supporter_used && setup_axis_missing() &&
        !play_secret_box_planned_supporter()) {
      choose_supporter();
    }

    if (use_legacy_star()) {
      advance_forretress_combo();
      play_items_until_stable(true);
      if (!state_.supporter_used && setup_axis_missing() &&
          !play_secret_box_planned_supporter()) {
        choose_supporter();
      }
    }

    resolve_final_promotion_and_attachment();
    if (need_active_vstar() && !state_.supporter_used) play_tate_switch();
    use_celestial_roar();
    trace("POLICY", "P-AXIS-01; R-SECRET-BOX-01",
          "End Pineco policy: " + state_line());
  }
