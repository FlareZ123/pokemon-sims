  std::uint64_t trials{0};
  std::uint64_t by2{0};
  std::uint64_t by3{0};
  std::uint64_t by4{0};
  std::uint64_t started_regi{0};
  std::uint64_t fss{0};
  std::uint64_t legacy{0};
  std::uint64_t steven{0};
  std::uint64_t blender{0};
  double total_mulligans{0.0};
};

struct TraceLog {
  bool enabled{false};
  std::vector<std::string> lines;

  void add(const int turn, std::string_view action, std::string_view rules, std::string_view detail) {
    if (!enabled) return;
    std::ostringstream out;
    if (turn > 0) out << "T" << turn << " | ";
    out << action << " | rules: " << rules << " | " << detail;
    lines.push_back(out.str());
  }

  void note(std::string_view detail) {
    if (enabled) lines.emplace_back(detail);
  }
};

std::string join_cards(const std::vector<Card>& cards) {
  if (cards.empty()) return "(none)";
  std::ostringstream out;
  for (std::size_t i = 0; i < cards.size(); ++i) {
    if (i != 0) out << ", ";
    out << name(cards[i]);
  }
  return out.str();
}

std::string pokemon_summary(const Pokemon& p) {
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

  static int count_of(const std::vector<Card>& zone, const Card card) {
    return static_cast<int>(std::count(zone.begin(), zone.end(), card));
  }

  int hand_count(const Card card) const { return count_of(state_.hand, card); }
  int deck_count_after_search_started(const Card card) const { return count_of(state_.deck, card); }
  int prize_count_after_reveal(const Card card) const { return count_of(state_.prizes, card); }

  int total_copies(const Card card) const {
    for (const auto& [candidate, copies] : recipe_) if (candidate == card) return copies;
    return 0;
  }

  // P-HIDDEN-01: decision code may use known zones and the deck list, but not deck or Prize identities.
  bool might_be_unseen(const Card card) const {
    const int known = hand_count(card) + count_of(state_.discard, card) + in_play_count(card);
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
    state_.retreat_used = false;
    state_.turn_ended = false;
    state_.discarded_this_turn.clear();
    if (first_turn_going_first()) {
      trace("TURN START", "R-GAME-FIRST-TURN", "Going first: no opening-turn draw and no Supporter or attack.");
    } else {
      draw_card("start-of-turn");
    }
  }

  bool in_play(const Card card) const {
    if (state_.active && state_.active->card == card) return true;
    return std::any_of(state_.bench.begin(), state_.bench.end(), [card](const Pokemon& p) { return p.card == card; });
  }

  int in_play_count(const Card card) const {
    int count = state_.active && state_.active->card == card ? 1 : 0;
    count += static_cast<int>(std::count_if(state_.bench.begin(), state_.bench.end(), [card](const Pokemon& p) { return p.card == card; }));
    return count;
  }

  bool has_regi_v() const { return in_play(Card::RegidragoV); }
  bool has_vstar() const { return in_play(Card::RegidragoVstar); }
  bool has_any_regi() const { return has_regi_v() || has_vstar(); }
  bool active_is_vstar() const { return state_.active && state_.active->card == Card::RegidragoVstar; }
  int bench_space() const { return 5 - static_cast<int>(state_.bench.size()); }

  int score_regi(const Pokemon& p, const bool active) const {
    if (p.card != Card::RegidragoV && p.card != Card::RegidragoVstar) return -100000;
    return 100 * std::min(2, p.grass) + 100 * std::min(1, p.fire) +
           (p.card == Card::RegidragoVstar ? 15 : 0) + (active ? 3 : 0);
  }

  Pokemon* target_regi() {
    Pokemon* best = nullptr;
    int best_score = -100000;
    if (state_.active) {
      const int score = score_regi(*state_.active, true);
      if (score > best_score) { best = &*state_.active; best_score = score; }
    }
    for (Pokemon& p : state_.bench) {
      const int score = score_regi(p, false);
      if (score > best_score) { best = &p; best_score = score; }
    }
    return best;
  }

  const Pokemon* target_regi() const {
    const Pokemon* best = nullptr;
    int best_score = -100000;
    if (state_.active) {
      const int score = score_regi(*state_.active, true);
      if (score > best_score) { best = &*state_.active; best_score = score; }
    }
    for (const Pokemon& p : state_.bench) {
      const int score = score_regi(p, false);
      if (score > best_score) { best = &p; best_score = score; }
    }
    return best;
  }

  int grass_needed() const {
    const Pokemon* target = target_regi();
    return target == nullptr ? 2 : std::max(0, 2 - target->grass);
  }
  int fire_needed() const {
    const Pokemon* target = target_regi();
    return target == nullptr ? 1 : std::max(0, 1 - target->fire);
  }
  bool need_energy() const { return grass_needed() > 0 || fire_needed() > 0; }

  bool payload_ready() const {
    if (!strict_payload_timing()) return std::any_of(state_.discard.begin(), state_.discard.end(), is_payload);
    return std::any_of(state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), is_payload);
  }
  bool need_payload() const { return !payload_ready(); }

  bool can_free_retreat_with_latias() const {
    return state_.active && is_basic(state_.active->card) && in_play(Card::LatiasEx) &&
           ability_available_for_pokemon(Card::LatiasEx) && !state_.retreat_used;
  }
  bool need_active_vstar() const { return has_vstar() && !active_is_vstar(); }

  std::string state_line() const {
    std::ostringstream out;
    out << "Active=";
    if (state_.active) out << pokemon_summary(*state_.active); else out << "(none)";
    out << " | Bench=";
    if (state_.bench.empty()) out << "(none)";
    for (std::size_t i = 0; i < state_.bench.size(); ++i) {
      if (i != 0) out << "; ";
      out << pokemon_summary(state_.bench[i]);
    }
    out << " | Hand=" << join_cards(state_.hand);
    out << " | Discard=" << join_cards(state_.discard);
    return out.str();
  }

  void setup() {
    for (const auto& [card, copies] : recipe_) {
      for (int copy = 0; copy < copies; ++copy) state_.deck.push_back(card);
    }
    shuffle(state_.deck);
    while (true) {
      state_.hand.clear();
      for (int card = 0; card < 7; ++card) {
        const Card drawn = state_.deck.back();
        state_.deck.pop_back();
        state_.hand.push_back(drawn);
      }
      if (std::any_of(state_.hand.begin(), state_.hand.end(), is_basic)) break;
      state_.deck.insert(state_.deck.end(), state_.hand.begin(), state_.hand.end());
      state_.hand.clear();
      shuffle(state_.deck);
      ++outcome_.mulligans;
    }

    choose_opening_active();
    choose_opening_bench();
    shuffle(state_.deck);
    for (int prize = 0; prize < 6; ++prize) {
