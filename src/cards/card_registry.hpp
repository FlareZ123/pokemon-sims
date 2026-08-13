#pragma once

#include <array>

#include "card_definition.hpp"
#include "trainers/quick_ball.hpp"

namespace sim::cards {

inline constexpr std::array registered_cards{
    Card::QuickBall,
};

// Explicit registration avoids static initialization order and linker-retention
// behavior. Add one case for each card migrated by the cleanup workflow.
constexpr const CardDefinition* find_definition(const Card card) {
  switch (card) {
    case Card::QuickBall:
      return &QuickBall::definition;
    default:
      return nullptr;
  }
}

// Keep registry membership in one value list so audits and compatibility seams do
// not need a second hand-maintained registration switch.
constexpr bool has_definition(const Card card) {
  for (const Card registered_card : registered_cards) {
    if (registered_card == card) return true;
  }
  return false;
}

constexpr bool registered_is_item(const Card card) {
  switch (card) {
    case Card::QuickBall:
      return QuickBall::definition.is_item();
    default:
      return false;
  }
}

}  // namespace sim::cards
