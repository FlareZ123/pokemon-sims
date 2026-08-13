#pragma once

#include "card_definition.hpp"
#include "trainers/earthen_vessel.hpp"
#include "trainers/professors_letter.hpp"
#include "trainers/quick_ball.hpp"

namespace sim::cards {

constexpr const CardDefinition* find_definition(const Card card) {
  switch (card) {
    case Card::QuickBall:
      return &QuickBall::definition;
    case Card::ProfessorsLetter:
      return &ProfessorsLetter::definition; // Exact XY 123 metadata: https://api.pokemontcg.io/v2/cards/xy1-123
    case Card::EarthenVessel:
      return &EarthenVessel::definition; // Exact PAR 163 metadata: https://api.pokemontcg.io/v2/cards/sv4-163
    default:
      return nullptr;
  }
}

constexpr bool has_definition(const Card card) {
  return find_definition(card) != nullptr;
}

constexpr bool registered_is_item(const Card card) {
  const CardDefinition* definition = find_definition(card);
  return definition != nullptr && definition->kind == CardKind::Trainer &&
         definition->trainer_kind == TrainerKind::Item;
}

}  // namespace sim::cards
