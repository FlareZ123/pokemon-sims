  std::ostringstream out;
  out << name(p.card) << " [";
  for (int i = 0; i < p.grass; ++i) out << "G";
  for (int i = 0; i < p.fire; ++i) out << "F";
  if (p.grass == 0 && p.fire == 0) out << "-";
  out << "]";
  if (p.tool == Tool::ForestSealStone) out << " + Forest Seal Stone";
  if (p.tool == Tool::WishfulBaton) out << " + Wishful Baton";
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
      // A turn cannot proceed after the start-of-turn draw loss.
      if (state_.turn_ended) break;
      run_turn();
      // Powerglass resolves at the end of the turn after the attack timing point, so
      // its Energy cannot make that already-passed attack ready this turn:
      // https://api.pokemontcg.io/v2/cards/sv6pt5-63
      record_ready();
      if (outcome_.first_ready_turn != 0) break;
      resolve_powerglass_end_turn();
    }
    if (scenario_.max_turn >= 5) {
      outcome_.setup_failed = !outcome_.ready_by_4;
      if (outcome_.setup_failed) {
        const std::string detail = outcome_.first_ready_turn == 5
            ? "First reached readiness on T5. T5 is diagnostic, so the game remains a setup loss."
            : "Did not reach readiness by the end of T4. The game is classified as a setup loss.";
        trace("SETUP FAILURE", "P-T5-FAIL-01", detail);
      }
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
    for (const auto& [candidate, copies] : recipe_) {
      if (candidate == card && copies > 0) return copies;
    }
    // Unit fixtures may install a physically consistent reduced state whose card is
    // intentionally absent from the canonical recipe. Derive that synthetic state's
    // copy count from its zones without changing any real simulation, where every
    // physical card is represented by recipe_. This keeps K1 legality tests independent
    // of retired main-list singletons such as Mawile-GX.
    int observed = count_of(state_.hand, card) + count_of(state_.deck, card) +
                   count_of(state_.prizes, card) + count_of(state_.discard, card);
    if (state_.active && state_.active->card == card) ++observed;
    observed += static_cast<int>(std::count_if(
        state_.bench.begin(), state_.bench.end(),
        [card](const Pokemon& pokemon) { return pokemon.card == card; }));
    return observed;
  }

  int attached_public_count(const Card card) const {
    const auto count_on_pokemon = [card](const Pokemon& pokemon) {
      switch (card) {
        case Card::Grass: return pokemon.grass;
        case Card::Fire: return pokemon.fire;
        case Card::ForestSealStone: return pokemon.tool == Tool::ForestSealStone ? 1 : 0;
        case Card::WishfulBaton: return pokemon.tool == Tool::WishfulBaton ? 1 : 0;
        case Card::Powerglass: return pokemon.tool == Tool::Powerglass ? 1 : 0;
        default: return 0;
      }
    };
    int count = state_.active ? count_on_pokemon(*state_.active) : 0;
    for (const Pokemon& pokemon : state_.bench) count += count_on_pokemon(pokemon);
    return count;
  }

  // P-HIDDEN-01: before a legal deck or Prize inspection, policy uses only public zones and
  // fixed-list copy counts. A deck search sees the remaining deck directly.
  // Attached Energy and Tools are public board information and cannot remain in
  // the deck. Core procedure source: https://www.pokemon.com/us/pokemon-tcg/rules
  // Heavy Ball reveals the full Prize set, so the fixed list plus known zones
  // also determines the remaining deck count. Source:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  bool might_be_unseen(const Card card) const {
    if (deck_seen_ || prizes_revealed_) return deck_count_after_search_started(card) > 0;
    // An evolved Regidrago VSTAR remains a public Evolution stack containing the
    // Regidrago V it evolved from. Count that underlying Basic for fixed-list K0
    // copy accounting without changing strategic counts of unevolved Pokémon:
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    const int evolved_regidrago_bases =
        card == Card::RegidragoV ? in_play_count(Card::RegidragoVstar) : 0;
    // Forretress ex is a Stage 1 that evolves from Pineco. Count the public
    // underlying Pineco in the same fixed-list K0 accounting used for Regidrago's
    // Evolution stack: https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // https://www.pokemon.com/us/pokemon-tcg/rules
    // https://github.com/FlareZ123/pokemon-sims/issues/972
    const int evolved_pineco_bases =
        card == Card::Pineco ? in_play_count(Card::ForretressEx) : 0;
    const int known = hand_count(card) + count_of(state_.discard, card) + in_play_count(card) +
                      evolved_regidrago_bases + evolved_pineco_bases +
                      attached_public_count(card);
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
  bool supporter_allowed() const { return scenario_.locks != LockMode::FullSupporter && !first_turn_going_first() && !state_.supporter_used && !state_.turn_ended; }
  bool item_locked() const {
    return scenario_.locks == LockMode::FullItem || scenario_.locks == LockMode::FullCombined ||
           (scenario_.locks == LockMode::TurnTwoItem && state_.turn >= 2);
  }
  bool rule_box_abilities_locked() const {
    const bool path_style_lock = scenario_.locks == LockMode::FullRuleBoxAbility ||
                                 scenario_.locks == LockMode::FullCombined;
    // Path to the Peak suppresses Rule Box Pokémon Abilities only while that Stadium
    // remains in play: https://api.pokemontcg.io/v2/cards/swsh6-148
    return path_style_lock && !state_.path_lock_removed;
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
