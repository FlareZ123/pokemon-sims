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