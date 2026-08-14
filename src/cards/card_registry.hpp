#pragma once

#include <array>

#include "card_definition.hpp"
#include "trainers/arven.hpp"
#include "trainers/battle_vip_pass.hpp"
#include "trainers/brilliant_blender.hpp"
#include "trainers/chaotic_swell.hpp"
#include "trainers/dawn.hpp"
#include "trainers/evolution_incense.hpp"
#include "trainers/field_blower.hpp"
#include "trainers/guzma_hala.hpp"
#include "trainers/hisuian_heavy_ball.hpp"
#include "trainers/mysterious_treasure.hpp"
#include "trainers/powerglass.hpp"
#include "trainers/professors_letter.hpp"
#include "trainers/quick_ball.hpp"

namespace sim::cards {

inline constexpr std::array<const CardDefinition*, 13> kRegisteredCardDefinitions{
    &Arven::definition, // Exact Scarlet & Violet Supporter: https://api.pokemontcg.io/v2/cards/sv1-166
    &BattleVipPass::definition,
    &BrilliantBlender::definition, // Exact ACE SPEC Item: https://api.pokemontcg.io/v2/cards/sv8-164
    &ChaoticSwell::definition, // Exact Cosmic Eclipse Stadium: https://api.pokemontcg.io/v2/cards/sm12-187
    &Dawn::definition, // Exact Mega Evolution Supporter: https://api.pokemontcg.io/v2/cards/me2-87
    &EvolutionIncense::definition, // Exact print: https://api.pokemontcg.io/v2/cards/swsh1-163
    &FieldBlower::definition, // Exact SM 125 metadata: https://api.pokemontcg.io/v2/cards/sm2-125
    &QuickBall::definition,
    &ProfessorsLetter::definition, // Exact XY 123 metadata: https://api.pokemontcg.io/v2/cards/xy1-123
    &MysteriousTreasure::definition, // Exact SM 113 metadata: https://api.pokemontcg.io/v2/cards/sm6-113
    &HisuianHeavyBall::definition, // Exact SWSH 146 metadata: https://api.pokemontcg.io/v2/cards/swsh10-146
    &GuzmaHala::definition, // Exact Cosmic Eclipse 229 Supporter: https://api.pokemontcg.io/v2/cards/sm12-229
    &Powerglass::definition, // Exact Shrouded Fable Tool: https://api.pokemontcg.io/v2/cards/sv6pt5-63
};

// Keep migrated card metadata tied to the stable legacy Card ids while the trace
// engine still contains legacy switch-based fallbacks. The card registry owns this
// bridge contract so consumers do not need to duplicate metadata assertions.
static_assert(QuickBall::definition.id == Card::QuickBall); // Exact Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
static_assert(QuickBall::definition.canonical_id == "swsh1-179");
static_assert(QuickBall::definition.name == "Quick Ball");
static_assert(GuzmaHala::definition.id == Card::GuzmaHala); // Exact Guzma & Hala: https://api.pokemontcg.io/v2/cards/sm12-229
static_assert(GuzmaHala::definition.canonical_id == "sm12-229");
static_assert(GuzmaHala::definition.name == "Guzma & Hala");
static_assert(Arven::definition.id == Card::Arven); // Exact Arven: https://api.pokemontcg.io/v2/cards/sv1-166
static_assert(Arven::definition.canonical_id == "sv1-166");
static_assert(Arven::definition.name == "Arven");
static_assert(Dawn::definition.id == Card::Dawn); // Exact Dawn: https://api.pokemontcg.io/v2/cards/me2-87
static_assert(Dawn::definition.canonical_id == "me2-87");
static_assert(Dawn::definition.name == "Dawn");
static_assert(Powerglass::definition.id == Card::Powerglass); // Exact Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
static_assert(Powerglass::definition.canonical_id == "sv6pt5-63");
static_assert(Powerglass::definition.name == "Powerglass");

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
