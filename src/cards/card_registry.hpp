#pragma once

#include <array>

#include "card_definition.hpp"
#include "trainers/mysterious_treasure.hpp"
#include "trainers/professors_letter.hpp"
#include "trainers/quick_ball.hpp"

namespace sim::cards {

inline constexpr std::array<const CardDefinition*, 3> kRegisteredDefinitions{
    &QuickBall::definition, // Exact card data: https://api.pokemontcg.io/v2/cards/swsh1-179
    &ProfessorsLetter::definition, // Exact card data: https://api.pokemontcg.io/v2/cards/xy1-123
    &MysteriousTreasure::definition, // Exact card data: https://api.pokemontcg.io/v2/cards/sm6-113
};

constexpr const CardDefinition* find_definition(const Card card) {
  for (const CardDefinition* definition : kRegisteredDefinitions) {
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
