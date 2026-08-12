// Compose the state model, aggregate metrics, and trace ownership before the
// formatting helpers that consume them. Preserve this textual include order:
// https://eel.is/c++draft/cpp.include
// State model: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/game_state_types.inc
// Metrics: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulation_metrics.inc
// Trace log: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/trace_log.inc
#include "game_state_types.inc"
#include "simulation_metrics.inc"
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