#pragma once

#include <array>

#include "card_definition.hpp"
#include "trainers/battle_vip_pass.hpp"
#include "trainers/brilliant_blender.hpp"
#include "trainers/chaotic_swell.hpp"
#include "trainers/evolution_incense.hpp"
#include "trainers/field_blower.hpp"
#include "trainers/guzma_hala.hpp"
#include "trainers/hisuian_heavy_ball.hpp"
#include "trainers/mysterious_treasure.hpp"
#include "trainers/powerglass.hpp"
#include "trainers/professors_letter.hpp"
#include "trainers/quick_ball.hpp"

namespace sim::cards {

inline constexpr std::array<const CardDefinition*, 11> kRegisteredCardDefinitions{
    &BattleVipPass::definition,
    &BrilliantBlender::definition, // Exact ACE SPEC Item: https://api.pokemontcg.io/v2/cards/sv8-164
    &ChaoticSwell::definition, // Exact Cosmic Eclipse Stadium: https://api.pokemontcg.io/v2/cards/sm12-187
    &EvolutionIncense::definition, // Exact print: https://api.pokemontcg.io/v2/cards/swsh1-163
    &FieldBlower::definition, // Exact SM 125 metadata: https://api.pokemontcg.io/v2/cards/sm2-125
    &QuickBall::definition,
    &ProfessorsLetter::definition, // Exact XY 123 metadata: https://api.pokemontcg.io/v2/cards/xy1-123
    &MysteriousTreasure::definition, // Exact SM 113 metadata: https://api.pokemontcg.io/v2/cards/sm6-113
    &HisuianHeavyBall::definition, // Exact SWSH 146 metadata: https://api.pokemontcg.io/v2/cards/swsh10-146
    &GuzmaHala::definition, // Exact Cosmic Eclipse 229 Supporter: https://api.pokemontcg.io/v2/cards/sm12-229
    &Powerglass::definition, // Exact Shrouded Fable Tool: https://api.pokemontcg.io/v2/cards/sv6pt5-63
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

constexpr bool registered_is_supporter(const Card card) {
  return registered_is_trainer_kind(card, TrainerKind::Supporter);
}

}  // namespace sim::cards
