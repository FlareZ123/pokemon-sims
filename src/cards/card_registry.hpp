#pragma once

#include <array>

#include "card_definition.hpp"
#include "pokemon/appletun.hpp"
#include "trainers/arven.hpp"
#include "trainers/battle_vip_pass.hpp"
#include "trainers/brilliant_blender.hpp"
#include "trainers/chaotic_swell.hpp"
#include "trainers/channeler.hpp"
#include "trainers/crispin.hpp"
#include "trainers/dawn.hpp"
#include "trainers/evolution_incense.hpp"
#include "trainers/field_blower.hpp"
#include "trainers/forest_of_vitality.hpp"
#include "trainers/forest_seal_stone.hpp"
#include "trainers/guzma_hala.hpp"
#include "trainers/hisuian_heavy_ball.hpp"
#include "trainers/klara.hpp"
#include "trainers/lusamine.hpp"
#include "trainers/mysterious_treasure.hpp"
#include "trainers/pokemon_communication.hpp"
#include "trainers/powerglass.hpp"
#include "trainers/professor_burnet.hpp"
#include "trainers/professor_turo_scenario.hpp"
#include "trainers/professors_letter.hpp"
#include "trainers/quick_ball.hpp"
#include "trainers/secret_box.hpp"
#include "trainers/stevens_resolve.hpp"
#include "trainers/wishful_baton.hpp"

namespace sim::cards {

inline constexpr std::array<const CardDefinition*, 26> kRegisteredCardDefinitions{
    &Appletun::definition, // Exact Surging Sparks Stage 1 Dragon: https://api.pokemontcg.io/v2/cards/sv8-140 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3642
    &Arven::definition, // Exact Scarlet & Violet Supporter: https://api.pokemontcg.io/v2/cards/sv1-166
    &BattleVipPass::definition,
    &BrilliantBlender::definition, // Exact ACE SPEC Item: https://api.pokemontcg.io/v2/cards/sv8-164
    &ChaoticSwell::definition, // Exact Cosmic Eclipse Stadium: https://api.pokemontcg.io/v2/cards/sm12-187
    &Channeler::definition, // Exact Unified Minds Supporter: https://api.pokemontcg.io/v2/cards/sm11-190 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3624
    &Crispin::definition, // Exact Stellar Crown Supporter: https://api.pokemontcg.io/v2/cards/sv7-133 ; enhancement: https://github.com/FlareZ123/pokemon-sims/issues/3580
    &Dawn::definition, // Exact Mega Evolution Supporter: https://api.pokemontcg.io/v2/cards/me2-87
    &EvolutionIncense::definition, // Exact print: https://api.pokemontcg.io/v2/cards/swsh1-163
    &FieldBlower::definition, // Exact SM 125 metadata: https://api.pokemontcg.io/v2/cards/sm2-125
    &ForestOfVitality::definition, // Exact Mega Evolution Stadium: https://api.pokemontcg.io/v2/cards/me1-117 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3639
    &ForestSealStone::definition, // Exact Silver Tempest Pokémon Tool: https://api.pokemontcg.io/v2/cards/swsh12-156 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3612
    &QuickBall::definition,
    &ProfessorsLetter::definition, // Exact XY 123 metadata: https://api.pokemontcg.io/v2/cards/xy1-123
    &MysteriousTreasure::definition, // Exact SM 113 metadata: https://api.pokemontcg.io/v2/cards/sm6-113
    &HisuianHeavyBall::definition, // Exact SWSH 146 metadata: https://api.pokemontcg.io/v2/cards/swsh10-146
    &GuzmaHala::definition, // Exact Cosmic Eclipse 229 Supporter: https://api.pokemontcg.io/v2/cards/sm12-229
    &PokemonCommunication::definition, // Exact Team Up Item: https://api.pokemontcg.io/v2/cards/sm9-152 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3552
    &Powerglass::definition, // Exact Shrouded Fable Tool: https://api.pokemontcg.io/v2/cards/sv6pt5-63
    &ProfessorBurnet::definition, // Exact Trainer Gallery Supporter: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
    &ProfessorTuroScenario::definition, // Exact Paradox Rift Supporter: https://api.pokemontcg.io/v2/cards/sv4-171 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3632
    &SecretBox::definition, // Exact Twilight Masquerade ACE SPEC Item: https://api.pokemontcg.io/v2/cards/sv6-163 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3541
    &StevensResolve::definition, // Exact Celestial Storm Supporter: https://api.pokemontcg.io/v2/cards/sm7-145 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3595
    &Lusamine::definition, // Exact Crimson Invasion Supporter: https://api.pokemontcg.io/v2/cards/sm4-96 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3619
    &Klara::definition, // Exact Chilling Reign Supporter: https://api.pokemontcg.io/v2/cards/swsh6-145 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3625
    &WishfulBaton::definition, // Exact Burning Shadows Pokémon Tool: https://api.pokemontcg.io/v2/cards/sm3-128 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3631
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

constexpr bool definition_matches_registration(
    const CardDefinition& definition, const Card id,
    const std::string_view canonical_id, const std::string_view canonical_name) {
  return definition.id == id && definition.canonical_id == canonical_id &&
         definition.name == canonical_name;
}

// Registry-owned compile-time contracts stay beside the definitions they verify.
// Architecture plan: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
static_assert(definition_matches_registration(QuickBall::definition, Card::QuickBall,
                                              "swsh1-179", "Quick Ball"));
static_assert(definition_matches_registration(GuzmaHala::definition, Card::GuzmaHala,
                                              "sm12-229", "Guzma & Hala"));
static_assert(definition_matches_registration(Arven::definition, Card::Arven,
                                              "sv1-166", "Arven")); // Exact Supporter: https://api.pokemontcg.io/v2/cards/sv1-166
static_assert(definition_matches_registration(Crispin::definition, Card::Crispin,
                                              "sv7-133", "Crispin")); // Exact Supporter: https://api.pokemontcg.io/v2/cards/sv7-133 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3580
static_assert(definition_matches_registration(Dawn::definition, Card::Dawn,
                                              "me2-87", "Dawn")); // Exact Supporter: https://api.pokemontcg.io/v2/cards/me2-87
static_assert(definition_matches_registration(Powerglass::definition, Card::Powerglass,
                                              "sv6pt5-63", "Powerglass")); // Exact Tool: https://api.pokemontcg.io/v2/cards/sv6pt5-63
static_assert(definition_matches_registration(Lusamine::definition, Card::Lusamine,
                                              "sm4-96", "Lusamine")); // Exact Supporter: https://api.pokemontcg.io/v2/cards/sm4-96 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3619
static_assert(definition_matches_registration(Klara::definition, Card::Klara,
                                              "swsh6-145", "Klara")); // Exact Supporter: https://api.pokemontcg.io/v2/cards/swsh6-145 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3625
static_assert(definition_matches_registration(WishfulBaton::definition, Card::WishfulBaton,
                                              "sm3-128", "Wishful Baton")); // Exact Pokémon Tool: https://api.pokemontcg.io/v2/cards/sm3-128 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3631
static_assert(definition_matches_registration(Channeler::definition, Card::Channeler,
                                              "sm11-190", "Channeler")); // Exact Supporter: https://api.pokemontcg.io/v2/cards/sm11-190 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3624
static_assert(definition_matches_registration(ProfessorTuroScenario::definition,
                                              Card::ProfessorTuro, "sv4-171",
                                              "Professor Turo's Scenario")); // Exact Supporter: https://api.pokemontcg.io/v2/cards/sv4-171 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3632
static_assert(definition_matches_registration(StevensResolve::definition,
                                              Card::StevensResolve, "sm7-145",
                                              "Steven's Resolve")); // Exact Supporter: https://api.pokemontcg.io/v2/cards/sm7-145 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3595

}  // namespace sim::cards
