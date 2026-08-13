#pragma once

#include "card_definition.hpp"
#include "trainers/professors_letter.hpp"
#include "trainers/quick_ball.hpp"

namespace sim::cards {

constexpr const CardDefinition* find_definition(const Card card) {
  switch (card) {
    case Card::QuickBall:
      return &QuickBall::definition;
    case Card::ProfessorsLetter:
      return &ProfessorsLetter::definition; // Exact XY 123 metadata: https://api.pokemontcg.io/v2/cards/xy1-123
    default:
      return nullptr;
  }
}

constexpr bool has_definition(const Card card) {
  switch (card) {
    case Card::QuickBall:
    case Card::ProfessorsLetter: // Printed Item: https://api.pokemontcg.io/v2/cards/xy1-123
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
    case Card::ProfessorsLetter:
      return ProfessorsLetter::definition.kind == CardKind::Trainer &&
             ProfessorsLetter::definition.trainer_kind == TrainerKind::Item; // https://api.pokemontcg.io/v2/cards/xy1-123
    default:
      return false;
  }
}

}  // namespace sim::cards
