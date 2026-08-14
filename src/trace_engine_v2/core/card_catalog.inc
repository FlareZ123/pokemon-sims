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
static_assert(cards::GuzmaHala::definition.id == Card::GuzmaHala);
static_assert(cards::GuzmaHala::definition.canonical_id == "sm12-229");
static_assert(cards::GuzmaHala::definition.name == "Guzma & Hala");
static_assert(cards::Dawn::definition.id == Card::Dawn); // Exact Supporter: https://api.pokemontcg.io/v2/cards/me2-87
static_assert(cards::Dawn::definition.canonical_id == "me2-87");
static_assert(cards::Dawn::definition.name == "Dawn");
static_assert(cards::Powerglass::definition.id == Card::Powerglass); // Exact Tool: https://api.pokemontcg.io/v2/cards/sv6pt5-63
static_assert(cards::Powerglass::definition.canonical_id == "sv6pt5-63");
static_assert(cards::Powerglass::definition.name == "Powerglass");

constexpr std::array<std::pair<Card, int>, 36> kDeckRecipe{{
    {Card::RegidragoV, 4},         {Card::RegidragoVstar, 3}, {Card::Dragapult, 2},
    {Card::MegaDragonite, 2},      {Card::DialgaGX, 1},       {Card::GoodraVstar, 1},
    {Card::TapuLeleGX, 2},         {Card::LatiasEx, 1},       {Card::ErikasInvitation, 1},
    {Card::Oricorio, 1},           {Card::Dipplin, 1},        {Card::BrilliantBlender, 1},
    {Card::MysteriousTreasure, 4}, {Card::QuickBall, 3},      {Card::EarthenVessel, 2},
    {Card::Arven, 2},              {Card::Crispin, 2},        {Card::ProfessorBurnet, 1},
    {Card::Serena, 1},             {Card::TateLiza, 1},       {Card::StevensResolve, 1},
    {Card::Guzma, 1},              {Card::Channeler, 1},      {Card::Gladion, 2},
    {Card::Lusamine, 1},           {Card::TeamYellsCheer, 1}, {Card::Klara, 1},
    {Card::ProfessorTuro, 1},      {Card::ForestSealStone, 1}, {Card::Powerglass, 1},
    {Card::FieldBlower, 1},        {Card::ChaoticSwell, 1},   {Card::PathToPeak, 1},
    {Card::HisuianHeavyBall, 1},   {Card::Grass, 6},          {Card::Fire, 3},
}};

using DeckRecipe = std::vector<std::pair<Card, int>>;

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
    case Card::SecretBox: return "Secret Box";
    case Card::UltraBall: return "Ultra Ball";
    case Card::PokemonCommunication: return "Pokémon Communication";
    case Card::EarthenVessel: return "Earthen Vessel";
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
    case Card::ForestOfVitality: return "Forest of Vitality";
    case Card::PathToPeak: return "Path to the Peak";
    case Card::DoubleDragonEnergy: return "Double Dragon Energy";  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/ ; https://github.com/FlareZ123/pokemon-sims/issues/2238
    case Card::Grass: return "Grass Energy";
    case Card::Fire: return "Fire Energy";
    case Card::BattleVipPass:
    case Card::BrilliantBlender:
    case Card::ChaoticSwell:
    case Card::Dawn:
    case Card::EvolutionIncense:
    case Card::FieldBlower:
    case Card::GuzmaHala:
    case Card::HisuianHeavyBall:
    case Card::MysteriousTreasure:
    case Card::Powerglass:
    case Card::ProfessorsLetter:
    case Card::QuickBall:
      break;  // Registered CardDefinition owns these names: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
  }
  return "Unknown";
}
