enum class DciProfile : std::uint8_t { StrictJit, MatchupFlexJit, NoDiscardControl };
enum class LockMode : std::uint8_t { None, TurnTwoItem, FullItem, FullRuleBoxAbility, FullCombined, FullSupporter };
enum class Tool : std::uint8_t { None, ForestSealStone, WishfulBaton, Powerglass };
enum class Stadium : std::uint8_t { None, ChaoticSwell, ForestOfVitality };
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
  int double_dragon{0};  // Physical DDE cards; each provides exactly two flexible Energy while attached to Dragon: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ ; https://github.com/FlareZ123/pokemon-sims/issues/2238
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
  bool stadium_used{false};
  bool vstar_power_used{false};
  bool dark_asset_used{false};  // Once per turn: https://api.pokemontcg.io/v2/cards/swsh3-104
  bool turn_ended{false};
  bool path_lock_removed{false};
  Stadium stadium{Stadium::None};
};
