constexpr bool is_dragon_or_psychic(const Card card) {
  switch (card) {
    case Card::RegidragoV:
    case Card::RegidragoVstar:
    case Card::Dragapult:
    case Card::MegaDragonite:
    case Card::DialgaGX:
    case Card::GoodraVstar:
    case Card::TapuLeleGX:
    case Card::LatiasEx:
    case Card::Oricorio:
    case Card::Dipplin:
      return true;
    default:
      return false;
  }
}

constexpr bool is_payload(const Card card) {
  return card == Card::Dragapult || card == Card::MegaDragonite || card == Card::DialgaGX ||
         card == Card::GoodraVstar;
}

constexpr bool is_energy(const Card card) { return card == Card::Grass || card == Card::Fire; }

constexpr bool is_item(const Card card) {
  switch (card) {
    case Card::BrilliantBlender:
    case Card::MysteriousTreasure:
    case Card::QuickBall:
    case Card::UltraBall:
    case Card::EvolutionIncense:
    case Card::EarthenVessel:
    case Card::ForestSealStone:
    case Card::Powerglass:
    case Card::FieldBlower:
    case Card::HisuianHeavyBall:
      return true;
    default:
      return false;
  }
}

enum class DciProfile : std::uint8_t { StrictJit, MatchupFlexJit, NoDiscardControl };
enum class LockMode : std::uint8_t { None, TurnTwoItem, FullItem, FullRuleBoxAbility, FullCombined };

struct Scenario {
  std::string label;
  DciProfile dci;
  LockMode locks;
  bool going_first{true};
  int max_turn{4};
};

struct Pokemon {
  Card card{};
  int entered_turn{0};
  int grass{0};
  int fire{0};
  bool forest_seal{false};
  bool powerglass{false};
};

struct State {
  std::vector<Card> deck;
  std::vector<Card> hand;
  std::vector<Card> prizes;
  std::vector<Card> discard;
  std::vector<Card> discarded_this_turn;
  std::optional<Pokemon> active;
  std::vector<Pokemon> bench;
  int turn{0};
  bool supporter_used{false};
  bool manual_attachment_used{false};
  bool vstar_power_used{false};
  bool turn_ended{false};
};

struct TrialOutcome {
  bool ready_by_2{false};
  bool ready_by_3{false};
  bool ready_by_4{false};
  int first_ready_turn{0};
  int mulligans{0};
  bool opening_regidrago{false};
  bool used_fss{false};
  bool used_steven{false};
  bool used_blender{false};
};

struct Aggregate {
  std::uint64_t trials{0};
  std::uint64_t by2{0};
  std::uint64_t by3{0};
  std::uint64_t by4{0};
  std::uint64_t opening_regidrago{0};
  std::uint64_t fss{0};
  std::uint64_t steven{0};
  std::uint64_t blender{0};
  double mulligans{0.0};
};

class Engine {
 public:
  Engine(const Scenario& scenario, const DeckRecipe& recipe, std::mt19937_64& rng)
      : scenario_(scenario), recipe_(recipe), rng_(rng) {}

  TrialOutcome run() {
    setup();
    for (int turn = 1; turn <= scenario_.max_turn; ++turn) {
      state_.turn = turn;
      state_.supporter_used = false;
      state_.manual_attachment_used = false;
      state_.turn_ended = false;
      state_.discarded_this_turn.clear();
      draw();
      run_turn();
      record_ready();
    }
    return outcome_;
  }

 private:
  const Scenario& scenario_;
  const DeckRecipe& recipe_;
  std::mt19937_64& rng_;
  State state_;
  TrialOutcome outcome_;

  static int count_of(const std::vector<Card>& zone, const Card card) {
    return static_cast<int>(std::count(zone.begin(), zone.end(), card));
  }

  int hand_count(const Card card) const { return count_of(state_.hand, card); }
  int deck_count(const Card card) const { return count_of(state_.deck, card); }
  int prize_count(const Card card) const { return count_of(state_.prizes, card); }

  static bool remove_one(std::vector<Card>& zone, const Card card) {
    const auto it = std::find(zone.begin(), zone.end(), card);
    if (it == zone.end()) return false;
    zone.erase(it);
    return true;
  }

  void shuffle(std::vector<Card>& zone) { std::shuffle(zone.begin(), zone.end(), rng_); }

  bool first_turn_going_first() const { return scenario_.going_first && state_.turn == 1; }
  bool supporter_allowed() const { return !first_turn_going_first() && !state_.supporter_used && !state_.turn_ended; }
  bool item_locked() const {
    if (scenario_.locks == LockMode::FullItem || scenario_.locks == LockMode::FullCombined) return true;
    return scenario_.locks == LockMode::TurnTwoItem && state_.turn >= 2;
  }
  bool rule_box_locked() const {
    return scenario_.locks == LockMode::FullRuleBoxAbility || scenario_.locks == LockMode::FullCombined;
  }
  bool ability_active(const Card card) const { return !is_rule_box_pokemon(card) || !rule_box_locked(); }
  bool payload_window_open() const {
    return scenario_.dci == DciProfile::NoDiscardControl || state_.turn >= 2;
  }
