#pragma once

#include <array>

#include "card_definition.hpp"
#include "trainers/brilliant_blender.hpp"
#include "trainers/evolution_incense.hpp"
#include "trainers/forest_seal_stone.hpp"
#include "trainers/mysterious_treasure.hpp"
#include "trainers/professors_letter.hpp"
#include "trainers/quick_ball.hpp"

namespace sim::cards {

inline constexpr std::array<const CardDefinition*, 6> kRegisteredCardDefinitions{
    &BrilliantBlender::definition, // Exact ACE SPEC Item: https://api.pokemontcg.io/v2/cards/sv8-164
    &EvolutionIncense::definition, // Exact print: https://api.pokemontcg.io/v2/cards/swsh1-163
    &ForestSealStone::definition, // Exact Pokémon Tool: https://api.pokemontcg.io/v2/cards/swsh12-156
    &QuickBall::definition,
    &ProfessorsLetter::definition, // Exact XY 123 metadata: https://api.pokemontcg.io/v2/cards/xy1-123
    &MysteriousTreasure::definition, // Exact SM 113 metadata: https://api.pokemontcg.io/v2/cards/sm6-113
};

constexpr const CardDefinition* find_definition(const Card card) {
  for (const CardDefinition* definition : kRegisteredCardDefinitions) {
    if (definition->id == card) return definition;
  }
  return nullptr;
}

constexpr bool has_definition(const Card card) {
  return find_definition(card) != nullptr;
}

constexpr bool registered_is_trainer_kind(const Card card,
                                          const TrainerKind trainer_kind) {
  const CardDefinition* definition = find_definition(card);
  return definition != nullptr && is_trainer_kind(*definition, trainer_kind);
}

constexpr bool registered_is_item(const Card card) {
  return registered_is_trainer_kind(card, TrainerKind::Item);
}

constexpr bool registered_is_tool(const Card card) {
  return registered_is_trainer_kind(card, TrainerKind::Tool);
}

}  // namespace sim::cards
