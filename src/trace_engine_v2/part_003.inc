  std::ostringstream out;
  out << name(p.card) << " [";
  for (int i = 0; i < p.grass; ++i) out << "G";
  for (int i = 0; i < p.fire; ++i) out << "F";
  if (p.grass == 0 && p.fire == 0) out << "-";
  out << "]";
  if (p.tool == Tool::ForestSealStone) out << " + Forest Seal Stone";
  if (p.tool == Tool::Powerglass) out << " + Powerglass";
  return out.str();
}

class Engine {
 public:
  friend struct EngineTestAccess;
  Engine(const Scenario& scenario, const DeckRecipe& recipe, std::mt19937_64& rng, TraceLog* trace = nullptr)
      : scenario_(scenario), recipe_(recipe), rng_(rng), trace_(trace) {}

  TrialOutcome run() {
    setup();
    for (int next_turn = 1; next_turn <= scenario_.max_turn; ++next_turn) {
      begin_turn(next_turn);
      if (!state_.turn_ended) run_turn();
      record_ready();
      if (outcome_.first_ready_turn != 0) break;
    }
    return outcome_;
  }

  const State& state() const { return state_; }

 private:
  const Scenario& scenario_;
  const DeckRecipe& recipe_;
  std::mt19937_64& rng_;
  TraceLog* trace_{nullptr};
  State state_;
  TrialOutcome outcome_;
  // K0: no deck/Prize inspection. K1: a legal inspection established Prize knowledge.
  bool deck_seen_{false};
  bool prizes_revealed_{false};

  static int count_of(const std::vector<Card>& zone, const Card card) {
    return static_cast<int>(std::count(zone.begin(), zone.end(), card));
  }

  int hand_count(const Card card) const { return count_of(state_.hand, card); }
  int deck_count_after_search_started(const Card card) const { return count_of(state_.deck, card); }
  int prize_count_after_reveal(const Card card) const { return count_of(state_.prizes, card); }
  bool prizes_known() const { return deck_seen_ || prizes_revealed_; }
  bool known_prize(const Card card) const { return prizes_known() && prize_count_after_reveal(card) > 0; }

  // Call only while resolving a legal search effect. The deck is physically seen,
  // so later policy can deduce the Prize multiset from the fixed deck list and zones.
  void record_deck_search_knowledge(std::string_view source) {
    if (deck_seen_) return;
    deck_seen_ = true;
    trace("DECK KNOWLEDGE", "P-KNOWLEDGE-01", std::string(source) + ": deck inspected; Prize identities are now deducible for later decisions.");
  }

  int total_copies(const Card card) const {
    for (const auto& [candidate, copies] : recipe_) if (candidate == card) return copies;
    return 0;
  }

  // P-HIDDEN-01: before a legal inspection, policy uses only public zones and
  // fixed-list copy counts. After a deck search OR a full Prize reveal, the
  // remaining deck is deducible from the fixed list and known zones.
  // Heavy Ball source: https://api.pokemontcg.io/v2/cards/swsh10-146
  bool might_be_unseen(const Card card) const {
    if (deck_seen_) return deck_count_after_search_started(card) > 0;
    int known = hand_count(card) + count_of(state_.discard, card) + in_play_count(card);
    if (prizes_revealed_) known += prize_count_after_reveal(card);
    return total_copies(card) > known;
  }

  static bool remove_one(std::vector<Card>& zone, const Card card) {
    const auto it = std::find(zone.begin(), zone.end(), card);
    if (it == zone.end()) return false;
    zone.erase(it);
    return true;
  }

  void shuffle(std::vector<Card>& zone) { std::shuffle(zone.begin(), zone.end(), rng_); }

  void trace(std::string_view action, std::string_view rules, std::string_view detail) {
    if (trace_ != nullptr) trace_->add(state_.turn, action, rules, detail);
  }

  bool first_turn_going_first() const { return state_.turn == 1 && scenario_.going_first; }
  bool supporter_allowed() const { return !first_turn_going_first() && !state_.supporter_used && !state_.turn_ended; }
  bool item_locked() const {
    return scenario_.locks == LockMode::FullItem || scenario_.locks == LockMode::FullCombined ||
           (scenario_.locks == LockMode::TurnTwoItem && state_.turn >= 2);
  }
  bool rule_box_abilities_locked() const {
    return scenario_.locks == LockMode::FullRuleBoxAbility || scenario_.locks == LockMode::FullCombined;
  }
  bool ability_available_for_pokemon(const Card card) const {
    return !is_rule_box_pokemon(card) || !rule_box_abilities_locked();
  }
  bool strict_payload_timing() const { return scenario_.dci != DciProfile::NoDiscardControl; }
  bool payload_window_open() const { return !strict_payload_timing() || state_.turn >= 2; }

  void draw_card(std::string_view source) {
    if (state_.deck.empty()) {
      trace("DRAW FAILED", "R-GAME-DRAW", std::string(source) + ": deck empty");
      return;
    }
    const Card drawn = state_.deck.back();
    state_.deck.pop_back();
    state_.hand.push_back(drawn);
    trace("DRAW", "R-GAME-DRAW", std::string(source) + ": " + std::string(name(drawn)));
  }

  void begin_turn(const int turn) {
    state_.turn = turn;
    state_.supporter_used = false;
    state_.manual_energy_used = false;