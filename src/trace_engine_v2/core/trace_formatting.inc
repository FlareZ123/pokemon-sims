#pragma once

// Canonical trace-formatting helpers shared by the composed simulator runtime.
// Keep these helpers presentation-only: strategy, rules, and state transitions
// remain with their existing policy and rules owners.
// Card naming/catalog owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/card_catalog.inc
// Runtime Pokemon/Tool state owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/simulation_runtime.inc
// C++ textual include semantics: https://eel.is/c++draft/cpp.include

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
  std::ostringstream out;
  out << name(p.card) << " [";
  for (int i = 0; i < p.grass; ++i) out << "G";
  for (int i = 0; i < p.fire; ++i) out << "F";
  // D is one physical Double Dragon Energy card.
  for (int i = 0; i < p.double_dragon; ++i) out << "D";
  if (p.grass == 0 && p.fire == 0 && p.double_dragon == 0) out << "-";
  out << "]";
  if (p.tool == Tool::ForestSealStone) out << " + Forest Seal Stone";
  if (p.tool == Tool::WishfulBaton) out << " + Wishful Baton";
  if (p.tool == Tool::Powerglass) out << " + Powerglass";
  return out.str();
}
