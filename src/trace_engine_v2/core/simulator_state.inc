// Compose simulator state, aggregate metrics, and trace ownership before the
// formatting helpers that consume them. Preserve this textual include boundary:
// https://eel.is/c++draft/cpp.include
// Runtime state + metrics + trace: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulation_runtime.inc
#include "simulation_runtime.inc"

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