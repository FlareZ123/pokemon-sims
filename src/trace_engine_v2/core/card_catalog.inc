#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "../../cards/card_registry.hpp"
#include "card_context_adapter.inc"

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace sim {

// Stable Card identifiers now live in src/cards/card_id.hpp. The existing names
// and values are preserved so migrated card modules and legacy engine code coexist.
static_assert(cards::QuickBall::definition.id == Card::QuickBall);
static_assert(cards::QuickBall::definition.canonical_id == "swsh1-179");
static_assert(cards::QuickBall::definition.name == "Quick Ball");

#include "../composition/deck_recipe.inc"

constexpr std::string_view name(const Card card) {
  if (const cards::CardDefinition* definition = cards::find_definition(card)) {
    return definition->name;
  }
  switch (card) {
    case Card::RegidragoV: return "Regidrago V";
    case Card::RegidragoVstar: return "Regidrago VSTAR";
    case Card::Dragapult: return "Dragapult ex";
    case Card::MegaDragonite: return "Mega Dragonite ex";
    case Card::DialgaGX: return "Dialga-GX";
    case Card::GoodraVstar: return "Hisuian Goodra VSTAR";
    case Card::TapuLeleGX: return "Tapu Lele-GX";
    case Card::CrobatV: return "Crobat V";
    case Card::LatiasEx: return "Latias ex";
    case Card::MawileGX: return "Mawile-GX";
    case Card::Oricorio: return "Oricorio GRI 55";
    case Card::Dipplin: return "Dipplin TWM 127";
    case Card::Appletun: return "Appletun sv8-140";
    case Card::Pineco: return "Pineco";
    case Card::ForretressEx: return "Forretress ex";
    case Card::BrilliantBlender: return "Brilliant Blender";
    case Card::SecretBox: return "Secret Box";
    case Card::BattleVipPass: return "Battle VIP Pass";
    case Card::UltraBall: return "Ultra Ball";
    case Card::PokemonCommunication: return "Pokémon Communication";
    case Card::EarthenVessel: return "Earthen Vessel";
    case Card::Dawn: return "Dawn";
    case Card::Grant: return "Grant";
    case Card::Arven: return "Arven";
    case Card::Crispin: return "Crispin";
    case Card::ProfessorBurnet: return "Professor Burnet";
    case Card::Serena: return "Serena";
    case Card::TateLiza: return "Tate & Liza";
    case Card::StevensResolve: return "Steven's Resolve";
    case Card::Guzma: return "Guzma";
    case Card::Channeler: return "Channeler";
    case Card::Gladion: return "Gladion";
    case Card::Lusamine: return "Lusamine";
    case Card::TeamYellsCheer: return "Team Yell's Cheer";
    case Card::RoseannesBackup: return "Roseanne's Backup";
    case Card::Klara: return "Klara";
    case Card::ProfessorTuro: return "Professor Turo's Scenario";
    case Card::ErikasInvitation: return "Erika's Invitation";
    case Card::ForestSealStone: return "Forest Seal Stone";
    case Card::WishfulBaton: return "Wishful Baton";
    case Card::Powerglass: return "Powerglass";
    case Card::ChaoticSwell: return "Chaotic Swell";
    case Card::ForestOfVitality: return "Forest of Vitality";
    case Card::PathToPeak: return "Path to the Peak";
    case Card::HisuianHeavyBall: return "Hisuian Heavy Ball";
    case Card::DoubleDragonEnergy: return "Double Dragon Energy";  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ ; https://github.com/FlareZ123/pokemon-sims/issues/2238
    case Card::Grass: return "Grass Energy";
    case Card::Fire: return "Fire Energy";
    case Card::FieldBlower:
    case Card::MysteriousTreasure:
    case Card::QuickBall:
    case Card::ProfessorsLetter:
    case Card::EvolutionIncense:
      break;  // Registered CardDefinition owns these names: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
  }
  return "Unknown";
}
