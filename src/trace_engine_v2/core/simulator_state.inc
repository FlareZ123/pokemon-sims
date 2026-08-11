    case Card::Lusamine:
    case Card::TeamYellsCheer:
    case Card::RoseannesBackup:
    case Card::Klara:
    case Card::ProfessorTuro:
      return true;
    default:
      return false;
  }
}

constexpr bool is_item(const Card card) {
  switch (card) {
    case Card::BrilliantBlender:
    case Card::SecretBox:
    case Card::BattleVipPass:
    case Card::MysteriousTreasure:
    case Card::QuickBall:
    case Card::UltraBall:
    case Card::EvolutionIncense:
    case Card::PokemonCommunication:
    case Card::EarthenVessel:
    case Card::ProfessorsLetter:  // https://api.pokemontcg.io/v2/cards/xy1-123 ; https://github.com/FlareZ123/pokemon-sims/issues/2509
    case Card::FieldBlower:
    case Card::HisuianHeavyBall:
      return true;
    default:
      return false;
  }
}

constexpr bool is_tool(const Card card) {
  return card == Card::ForestSealStone || card == Card::WishfulBaton ||
         card == Card::Powerglass;
}

constexpr bool is_stadium(const Card card) {
  return card == Card::ChaoticSwell || card == Card::ForestOfVitality ||
         card == Card::PathToPeak;
}

constexpr bool is_ace_spec(const Card card) {
  return card == Card::BrilliantBlender || card == Card::SecretBox;
}

// Scenario, public game state, and modeled attachment state live together.
// Direct source registry: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/game_state_types.inc
#include "game_state_types.inc"

// Per-trial and aggregate counters are reporting state rather than gameplay state.
// Source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulation_metrics.inc
#include "simulation_metrics.inc"

// Trace ownership and state-fingerprint suppression are isolated from state storage.
// Source: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/trace_log.inc
#include "trace_log.inc"

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