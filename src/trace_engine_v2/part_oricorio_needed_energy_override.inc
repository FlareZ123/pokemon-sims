  bool needed_oricorio_energy_might_be_in_deck() const {
    // Vital Dance can search Basic Energy, while Apex Dragon specifically requires
    // two Grass and one Fire. At K1, an off-type Energy cannot advance the missing
    // component; K0 keeps using public fixed-list plausibility through might_be_unseen():
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    return (grass_needed() > 0 && might_be_unseen(Card::Grass)) ||
           (fire_needed() > 0 && might_be_unseen(Card::Fire));
  }

  bool needs_oricorio_connector() const {
    return needed_oricorio_energy_might_be_in_deck() &&
           needs_oricorio_connector_original();
  }

  bool bench_oricorio_if_useful() {
    if (hand_count(Card::Oricorio) == 0 || in_play(Card::Oricorio) ||
        bench_space() <= 0 || !needs_oricorio_connector()) {
      return false;
    }
    // Playing Oricorio from hand to the Bench triggers the optional Vital Dance
    // search only after the target-aware preflight has found a needed type:
    // https://api.pokemontcg.io/v2/cards/sm2-55
    return bench_from_hand(Card::Oricorio, true);
  }
