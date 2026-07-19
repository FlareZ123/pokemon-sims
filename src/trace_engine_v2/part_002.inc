    case Card::Lusamine:
    case Card::TeamYellsCheer:
    case Card::RoseannesBackup:
    case Card::ProfessorTuro:
      return true;
    default:
      return false;
  }
}

enum class DciProfile : std::uint8_t { StrictJit, MatchupFlexJit, NoDiscardControl };
enum class LockMode : std::uint8_t { None, TurnTwoItem, FullItem, FullRuleBoxAbility, FullCombined };
enum class Tool : std::uint8_t { None, ForestSealStone, Powerglass };
enum class Stadium : std::uint8_t { None, ChaoticSwell };
enum class OpponentBenchState : std::uint8_t { Unknown, Unavailable, Available };

struct Scenario {
  std::string label;
  DciProfile dci{DciProfile::StrictJit};
  LockMode locks{LockMode::None};
  bool going_first{true};
  int max_turn{4};
  // Guzma's self-switch is conditional on successfully switching an opposing
  // Benched Pokémon. Existing goldfish scenarios cannot prove that prerequisite:
  // https://api.pokemontcg.io/v2/cards/sm3-115
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#opponent-actions
  // https://github.com/FlareZ123/pokemon-sims/issues/741
  OpponentBenchState opponent_bench{OpponentBenchState::Unknown};
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
  Stadium stadium{Stadium::None};
};

struct TrialOutcome {
  bool ready_by_2{false};
  bool ready_by_3{false};
  bool ready_by_4{false};
  bool ready_by_5{false};
  int first_ready_turn{0};
  bool setup_failed{false};
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
  std::uint64_t by5{0};
  std::uint64_t ready_on5{0};
  std::uint64_t setup_failures{0};
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
  std::map<std::string, std::string> last_policy_state_by_key;

  static std::string policy_state_fingerprint(const State& state) {
    // Fixed-point policy may query the same no-op decision repeatedly. Public trace
    // events are emitted once per equivalent visible state and may reappear after a
    // real state change. Zone order is canonicalized because deck shuffles do not
    // change K1 card availability or any visible policy axis:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
    // https://github.com/FlareZ123/pokemon-sims/issues/1007
    std::ostringstream out;
    const auto append_cards = [&out](const std::vector<Card>& cards) {
      std::vector<int> ordered;
      ordered.reserve(cards.size());
      for (const Card card : cards) ordered.push_back(static_cast<int>(card));
      std::sort(ordered.begin(), ordered.end());
      out << '[';
      for (const int card : ordered) out << card << ',';
      out << ']';
    };
    const auto append_pokemon = [&out](const Pokemon& pokemon) {
      out << static_cast<int>(pokemon.card) << ':' << pokemon.entered_turn << ':'
          << pokemon.grass << ':' << pokemon.fire << ':'
          << static_cast<int>(pokemon.tool) << ';';
    };

    append_cards(state.deck);
    append_cards(state.hand);
    append_cards(state.prizes);
    append_cards(state.discard);
    append_cards(state.discarded_this_turn);
    if (state.active) append_pokemon(*state.active);
    else out << "no-active;";
    out << "bench{";
    for (const Pokemon& pokemon : state.bench) append_pokemon(pokemon);
    out << "}:" << state.turn << ':' << state.supporter_used << ':'
        << state.manual_energy_used << ':' << state.retreat_used << ':'
        << state.vstar_power_used << ':' << state.turn_ended << ':'
        << state.path_lock_removed << ':' << static_cast<int>(state.stadium);
    return out.str();
  }

  void add(const int turn, std::string_view action, std::string_view rules, std::string_view detail) {
    if (!enabled) return;
    std::ostringstream out;
    if (turn > 0) out << "T" << turn << " | ";
    out << action << " | rules: " << rules << " | " << detail;
    lines.push_back(out.str());
  }

  void add_policy_once(const State& state, std::string_view key, const int turn,
                       std::string_view action, std::string_view rules,
                       std::string_view detail) {
    if (!enabled) return;
    const std::string fingerprint = policy_state_fingerprint(state);
    const std::string owned_key{key};
    const auto found = last_policy_state_by_key.find(owned_key);
    if (found != last_policy_state_by_key.end() && found->second == fingerprint) return;
    last_policy_state_by_key[owned_key] = fingerprint;
    add(turn, action, rules, detail);
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