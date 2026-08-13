#pragma once

#include <array>

#include "card_definition.hpp"
#include "trainers/evolution_incense.hpp"
#include "trainers/professors_letter.hpp"
#include "trainers/quick_ball.hpp"

namespace sim::cards {

constexpr std::array<const CardDefinition*, 3> kRegisteredCards{
    &EvolutionIncense::definition, // Exact print: https://api.pokemontcg.io/v2/cards/swsh1-163
    &QuickBall::definition,
    &ProfessorsLetter::definition, // Exact XY 123 metadata: https://api.pokemontcg.io/v2/cards/xy1-123
};

constexpr const CardDefinition* find_definition(const Card card) {
  for (const CardDefinition* definition : kRegisteredCards) {
    if (definition->id == card) return definition;
  }
  return nullptr;
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
