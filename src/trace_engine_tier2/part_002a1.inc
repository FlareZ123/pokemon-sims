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
