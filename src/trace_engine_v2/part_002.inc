    case Card::Lusamine:
    case Card::TeamYellsCheer:
    case Card::RoseannesBackup:
    case Card::ProfessorTuro:
    case Card::Carmine:
    case Card::ProfessorsResearch:
    case Card::Lillie:
    case Card::AdventurersDiscovery:
      return true;
    default:
      return false;
  }
}

enum class DciProfile : std::uint8_t { StrictJit, MatchupFlexJit, NoDiscardControl };
enum class LockMode : std::uint8_t { None, TurnTwoItem, FullItem, FullRuleBoxAbility, FullCombined };
enum class Tool : std::uint8_t { None, ForestSealStone, Powerglass };
enum class Stadium : std::uint8_t { None, ChaoticSwell, PokeStop };

struct Scenario {
  std::string label;
  DciProfile dci{DciProfile::StrictJit};
  LockMode locks{LockMode::None};
  bool going_first{true};
  int max_turn{4};
};

struct Pokemon {
  Card card{};
  int entered_turn{0};
  int grass{0};
  int fire{0};
  Tool tool{Tool::None};
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
  bool manual_energy_used{false};
  bool retreat_used{false};
  bool vstar_power_used{false};
  bool turn_ended{false};
  bool path_lock_removed{false};
  bool stadium_ability_used{false};
  Stadium stadium{Stadium::None};
};

struct TrialOutcome {
  bool ready_by_2{false};
  bool ready_by_3{false};
  bool ready_by_4{false};
  int first_ready_turn{0};
  int mulligans{0};
  bool started_regi{false};
  bool used_fss{false};
  bool used_legacy_star{false};
  bool used_steven{false};
  bool used_blender{false};
};

struct Aggregate {
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