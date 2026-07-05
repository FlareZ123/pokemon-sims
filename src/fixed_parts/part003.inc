  }

  void draw() {
    if (state_.deck.empty()) return;
    state_.hand.push_back(state_.deck.back());
    state_.deck.pop_back();
  }

  bool in_play(const Card card) const {
    if (state_.active && state_.active->card == card) return true;
    return std::any_of(state_.bench.begin(), state_.bench.end(), [card](const Pokemon& p) { return p.card == card; });
  }

  int in_play_count(const Card card) const {
    int result = state_.active && state_.active->card == card ? 1 : 0;
    result += static_cast<int>(std::count_if(state_.bench.begin(), state_.bench.end(), [card](const Pokemon& p) { return p.card == card; }));
    return result;
  }

  bool has_regi() const { return in_play(Card::RegidragoV) || in_play(Card::RegidragoVstar); }
  bool has_regi_v() const { return in_play(Card::RegidragoV); }
  bool has_vstar() const { return in_play(Card::RegidragoVstar); }
  bool active_is_vstar() const { return state_.active && state_.active->card == Card::RegidragoVstar; }

  Pokemon* primary_regi() {
    if (state_.active && (state_.active->card == Card::RegidragoV || state_.active->card == Card::RegidragoVstar)) return &*state_.active;
    for (Pokemon& p : state_.bench) {
      if (p.card == Card::RegidragoV || p.card == Card::RegidragoVstar) return &p;
    }
    return nullptr;
  }

  const Pokemon* primary_regi() const {
    if (state_.active && (state_.active->card == Card::RegidragoV || state_.active->card == Card::RegidragoVstar)) return &*state_.active;
    for (const Pokemon& p : state_.bench) {
      if (p.card == Card::RegidragoV || p.card == Card::RegidragoVstar) return &p;
    }
    return nullptr;
  }

  int bench_space() const { return 5 - static_cast<int>(state_.bench.size()); }

  bool has_payload_in_discard() const {
    if (scenario_.dci == DciProfile::NoDiscardControl) {
      return std::any_of(state_.discard.begin(), state_.discard.end(), is_payload);
    }
    return std::any_of(state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), is_payload);
  }

  bool need_payload() const { return !has_payload_in_discard(); }
  int grass_needed() const { const Pokemon* p = primary_regi(); return p == nullptr ? 2 : std::max(0, 2 - p->grass); }
  int fire_needed() const { const Pokemon* p = primary_regi(); return p == nullptr ? 1 : std::max(0, 1 - p->fire); }
  bool need_energy() const { return grass_needed() > 0 || fire_needed() > 0; }
  bool needs_active_vstar() const { return has_vstar() && !active_is_vstar(); }

  bool possibly_unseen(const Card card) const {
    int total = 0;
    for (const auto& [candidate, copies] : recipe_) if (candidate == card) total = copies;
    int known = hand_count(card) + count_of(state_.discard, card) + in_play_count(card);
    return total > known;
  }

  void setup() {
    for (const auto& [card, copies] : recipe_) {
      for (int i = 0; i < copies; ++i) state_.deck.push_back(card);
    }
    shuffle(state_.deck);
    for (;;) {
      state_.hand.clear();
      for (int i = 0; i < 7; ++i) draw();
      if (std::any_of(state_.hand.begin(), state_.hand.end(), is_basic)) break;
      state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
      shuffle(state_.deck);
      ++outcome_.mulligans;
    }

    choose_opening_active();
    choose_opening_bench();
    shuffle(state_.deck);
    for (int i = 0; i < 6; ++i) {
      state_.prizes.push_back(state_.deck.back());
      state_.deck.pop_back();
    }
  }

  void choose_opening_active() {
    for (const Card candidate : {Card::RegidragoV, Card::LatiasEx, Card::TapuLeleGX, Card::Oricorio, Card::MawileGX, Card::DialgaGX}) {
      if (remove_one(state_.hand, candidate)) {
        state_.active = Pokemon{candidate, 0};
        outcome_.opening_regidrago = candidate == Card::RegidragoV;
        return;
      }
    }
    throw std::runtime_error("opening hand had no Basic despite mulligan check");
  }

  void choose_opening_bench() {
    // Wonder Tag and Vital Dance only trigger when played from hand during a turn.
    // Keep Tapu and Oricorio in hand rather than silently consuming their entry effects.
    if (bench_space() > 0 && hand_count(Card::RegidragoV) > 0 && in_play_count(Card::RegidragoV) < 2) {
      remove_one(state_.hand, Card::RegidragoV);
      state_.bench.push_back(Pokemon{Card::RegidragoV, 0});
    }
    if (bench_space() > 0 && hand_count(Card::LatiasEx) > 0 && !in_play(Card::LatiasEx)) {
      remove_one(state_.hand, Card::LatiasEx);
      state_.bench.push_back(Pokemon{Card::LatiasEx, 0});
    }
  }

  bool move_from_deck_to_hand(const Card card) {
    if (!remove_one(state_.deck, card)) return false;
    state_.hand.push_back(card);
    return true;
  }

  bool move_from_deck_to_discard(const Card card) {
    if (!remove_one(state_.deck, card)) return false;
    state_.discard.push_back(card);
    state_.discarded_this_turn.push_back(card);
    return true;
  }

  bool bench_basic(const Card card, const bool trigger_ability) {
    if (bench_space() <= 0 || !is_basic(card) || !remove_one(state_.hand, card)) return false;
    state_.bench.push_back(Pokemon{card, state_.turn});
    if (trigger_ability) resolve_entry_ability(state_.bench.back().card);
    return true;
  }

  std::vector<Card> energy_targets(const int grass, const int fire) const {
    std::vector<Card> targets;
    if (grass > 0 && fire > 0) {
      targets.push_back(Card::Grass);
      targets.push_back(Card::Fire);
      return targets;
    }
    if (grass >= 2) return {Card::Grass, Card::Grass};
    if (fire >= 2) return {Card::Fire, Card::Fire};
    if (grass > 0) return {Card::Grass};
    if (fire > 0) return {Card::Fire};
    return {};
  }

  void resolve_energy_search(const int desired_grass, const int desired_fire, const int max_cards) {
    int remaining_grass = desired_grass;
    int remaining_fire = desired_fire;
    std::vector<Card> picked;
    while (static_cast<int>(picked.size()) < max_cards) {
      Card choice = Card::Grass;
      if (remaining_grass > 0 && remaining_fire > 0) {
        // A G/F split is normally best for Apex's mixed cost.
        choice = picked.empty() ? Card::Grass : Card::Fire;
        if (deck_count(choice) == 0) choice = choice == Card::Grass ? Card::Fire : Card::Grass;
      } else if (remaining_grass > 0) {
        choice = Card::Grass;
      } else if (remaining_fire > 0) {
        choice = Card::Fire;
      } else {
        break;
      }
