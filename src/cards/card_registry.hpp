#pragma once

#include "card_definition.hpp"
#include "trainers/quick_ball.hpp"

namespace sim::cards {

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

}  // namespace sim::cards
