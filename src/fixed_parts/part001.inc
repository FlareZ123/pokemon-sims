#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <optional>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace sim {

enum class Card : std::uint8_t {
  RegidragoV,
  RegidragoVstar,
  Dragapult,
  MegaDragonite,
  DialgaGX,
  GoodraVstar,
  TapuLeleGX,
  LatiasEx,
  MawileGX,
  Oricorio,
  Dipplin,
  BrilliantBlender,
  MysteriousTreasure,
  QuickBall,
  UltraBall,
  EvolutionIncense,
  EarthenVessel,
  Arven,
  Crispin,
  ProfessorBurnet,
  Serena,
  TateLiza,
  StevensResolve,
  Guzma,
  Channeler,
  Gladion,
  Lusamine,
  TeamYellsCheer,
  RoseannesBackup,
  ProfessorTuro,
  ForestSealStone,
  Powerglass,
  FieldBlower,
  ChaoticSwell,
  PathToPeak,
  HisuianHeavyBall,
  Grass,
  Fire,
};

constexpr std::array<std::pair<Card, int>, 36> kDeckRecipe{{
    {Card::RegidragoV, 4},       {Card::RegidragoVstar, 3}, {Card::Dragapult, 2},
    {Card::MegaDragonite, 2},    {Card::DialgaGX, 1},       {Card::GoodraVstar, 1},
    {Card::TapuLeleGX, 2},       {Card::LatiasEx, 1},       {Card::MawileGX, 1},
    {Card::Oricorio, 1},         {Card::Dipplin, 1},        {Card::BrilliantBlender, 1},
    {Card::MysteriousTreasure, 4},{Card::QuickBall, 3},     {Card::EarthenVessel, 2},
    {Card::Arven, 2},            {Card::Crispin, 2},        {Card::ProfessorBurnet, 1},
    {Card::Serena, 1},           {Card::TateLiza, 1},       {Card::StevensResolve, 1},
    {Card::Guzma, 1},            {Card::Channeler, 1},      {Card::Gladion, 2},
    {Card::Lusamine, 1},         {Card::TeamYellsCheer, 1}, {Card::RoseannesBackup, 1},
    {Card::ProfessorTuro, 1},    {Card::ForestSealStone, 1},{Card::Powerglass, 1},
    {Card::FieldBlower, 1},      {Card::ChaoticSwell, 1},   {Card::PathToPeak, 1},
    {Card::HisuianHeavyBall, 1}, {Card::Grass, 6},          {Card::Fire, 3},
}};

using DeckRecipe = std::vector<std::pair<Card, int>>;

constexpr std::string_view name(const Card card) {
  switch (card) {
    case Card::RegidragoV: return "Regidrago V";
    case Card::RegidragoVstar: return "Regidrago VSTAR";
    case Card::Dragapult: return "Dragapult ex";
    case Card::MegaDragonite: return "Mega Dragonite ex";
    case Card::DialgaGX: return "Dialga-GX";
    case Card::GoodraVstar: return "Hisuian Goodra VSTAR";
    case Card::TapuLeleGX: return "Tapu Lele-GX";
    case Card::LatiasEx: return "Latias ex";
    case Card::MawileGX: return "Mawile-GX";
    case Card::Oricorio: return "Oricorio GRI 55";
    case Card::Dipplin: return "Dipplin TWM 127";
    case Card::BrilliantBlender: return "Brilliant Blender";
    case Card::MysteriousTreasure: return "Mysterious Treasure";
    case Card::QuickBall: return "Quick Ball";
    case Card::UltraBall: return "Ultra Ball";
    case Card::EvolutionIncense: return "Evolution Incense";
    case Card::EarthenVessel: return "Earthen Vessel";
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
    case Card::ProfessorTuro: return "Professor Turo's Scenario";
    case Card::ForestSealStone: return "Forest Seal Stone";
    case Card::Powerglass: return "Powerglass";
    case Card::FieldBlower: return "Field Blower";
    case Card::ChaoticSwell: return "Chaotic Swell";
    case Card::PathToPeak: return "Path to the Peak";
    case Card::HisuianHeavyBall: return "Hisuian Heavy Ball";
    case Card::Grass: return "Grass Energy";
    case Card::Fire: return "Fire Energy";
  }
  return "Unknown";
}

constexpr bool is_basic(const Card card) {
  switch (card) {
    case Card::RegidragoV:
    case Card::DialgaGX:
    case Card::TapuLeleGX:
    case Card::LatiasEx:
    case Card::MawileGX:
    case Card::Oricorio:
      return true;
    default:
      return false;
  }
}

constexpr bool is_rule_box_pokemon(const Card card) {
  switch (card) {
    case Card::RegidragoV:
    case Card::RegidragoVstar:
    case Card::Dragapult:
    case Card::MegaDragonite:
    case Card::DialgaGX:
    case Card::GoodraVstar:
    case Card::TapuLeleGX:
    case Card::LatiasEx:
    case Card::MawileGX:
      return true;
    default:
      return false;
  }
}
