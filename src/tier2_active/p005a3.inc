
  void choose_opening_active() {
    // R-GAME-SETUP: only Basics can be chosen; this policy preserves entry-Abilities where possible.
    for (const Card candidate : {Card::RegidragoV, Card::LatiasEx, Card::Oricorio, Card::MawileGX, Card::DialgaGX, Card::TapuLeleGX}) {
      if (remove_one(state_.hand, candidate)) {
        state_.active = Pokemon{candidate, 0};
        outcome_.started_regi = candidate == Card::RegidragoV;
        return;
      }
    }
    throw std::runtime_error("opening hand has no Basic after mulligan validation");
  }

  void choose_opening_bench() {
    // R-TAPU-01/R-ORICORIO-01: their on-Bench effects require a play from hand during a turn.
    if (bench_space() > 0 && hand_count(Card::RegidragoV) > 0 && in_play_count(Card::RegidragoV) < 2) {
      remove_one(state_.hand, Card::RegidragoV);
      state_.bench.push_back(Pokemon{Card::RegidragoV, 0});
    }
    // R-LATIAS-01 is continuous. Setup Bench placement is useful if the Active is not Regidrago V.
