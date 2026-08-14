#pragma once

#include <array>

#include "card_definition.hpp"
#include "trainers/arven.hpp"
#include "trainers/battle_vip_pass.hpp"
#include "trainers/brilliant_blender.hpp"
#include "trainers/chaotic_swell.hpp"
#include "trainers/channeler.hpp"
#include "trainers/crispin.hpp"
#include "trainers/dawn.hpp"
#include "trainers/evolution_incense.hpp"
#include "trainers/field_blower.hpp"
#include "trainers/guzma_hala.hpp"
#include "trainers/hisuian_heavy_ball.hpp"
#include "trainers/lusamine.hpp"
#include "trainers/mysterious_treasure.hpp"
#include "trainers/pokemon_communication.hpp"
#include "trainers/powerglass.hpp"
#include "trainers/professor_burnet.hpp"
#include "trainers/professors_letter.hpp"
#include "trainers/quick_ball.hpp"
#include "trainers/secret_box.hpp"

namespace sim::cards {

inline constexpr std::array<const CardDefinition*, 19> kRegisteredCardDefinitions{
    &Arven::definition, // Exact Scarlet & Violet Supporter: https://api.pokemontcg.io/v2/cards/sv1-166
    &BattleVipPass::definition,
    &BrilliantBlender::definition, // Exact ACE SPEC Item: https://api.pokemontcg.io/v2/cards/sv8-164
    &ChaoticSwell::definition, // Exact Cosmic Eclipse Stadium: https://api.pokemontcg.io/v2/cards/sm12-187
    &Channeler::definition, // Exact Unified Minds Supporter: https://api.pokemontcg.io/v2/cards/sm11-190 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3624
    &Crispin::definition, // Exact Stellar Crown Supporter: https://api.pokemontcg.io/v2/cards/sv7-133 ; enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3580
    &Dawn::definition, // Exact Mega Evolution Supporter: https://api.pokemontcg.io/v2/cards/me2-87
    &EvolutionIncense::definition, // Exact print: https://api.pokemontcg.io/v2/cards/swsh1-163
    &FieldBlower::definition, // Exact SM 125 metadata: https://api.pokemontcg.io/v2/cards/sm2-125
    &QuickBall::definition,
    &ProfessorsLetter::definition, // Exact XY 123 metadata: https://api.pokemontcg.io/v2/cards/xy1-123
    &MysteriousTreasure::definition, // Exact SM 113 metadata: https://api.pokemontcg.io/v2/cards/sm6-113
    &HisuianHeavyBall::definition, // Exact SWSH 146 metadata: https://api.pokemontcg.io/v2/cards/swsh10-146
    &GuzmaHala::definition, // Exact Cosmic Eclipse 229 Supporter: https://api.pokemontcg.io/v2/cards/sm12-229
    &PokemonCommunication::definition, // Exact Team Up Item: https://api.pokemontcg.io/v2/cards/sm9-152 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3552
    &Powerglass::definition, // Exact Shrouded Fable Tool: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    &ProfessorBurnet::definition, // Exact Trainer Gallery Supporter: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    &SecretBox::definition, // Exact Twilight Masquerade ACE SPEC Item: https://api.pokemontcg.io/v2/cards/sv6-163 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3541
    &Lusamine::definition, // Exact Crimson Invasion Supporter: https://api.pokemontcg.io/v2/cards/sm4-96 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3619
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

constexpr bool registered_is_ace_spec(const Card card) {
  const CardDefinition* definition = find_definition(card);
  return definition != nullptr && definition->ace_spec;
}

}  // namespace sim::cards
