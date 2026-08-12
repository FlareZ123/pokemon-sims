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

// Classification code must stop falling back to legacy tables once a card is
// registered. Keep this predicate value-based so it remains usable in constexpr
// classification paths even on compilers that reject static assertions involving
// pointers returned by find_definition().
constexpr bool has_definition(const Card card) {
  switch (card) {
    case Card::QuickBall:
      return true;
    default:
      return false;
  }
}

constexpr bool registered_is_item(const Card card) {
  switch (card) {
    case Card::QuickBall:
      return QuickBall::definition.kind == CardKind::Trainer &&
             QuickBall::definition.trainer_kind == TrainerKind::Item;
    default:
      return false;
  }
}

}  // namespace sim::cards
