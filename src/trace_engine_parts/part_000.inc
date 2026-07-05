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

// The traceability IDs referenced below are defined in docs/RULES_TRACEABILITY.md.
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
    {Card::RegidragoV, 4},        {Card::RegidragoVstar, 3}, {Card::Dragapult, 2},
    {Card::MegaDragonite, 2},     {Card::DialgaGX, 1},       {Card::GoodraVstar, 1},
    {Card::TapuLeleGX, 2},        {Card::LatiasEx, 1},       {Card::MawileGX, 1},
    {Card::Oricorio, 1},          {Card::Dipplin, 1},        {Card::BrilliantBlender, 1},
    {Card::MysteriousTreasure, 4},{Card::QuickBall, 3},      {Card::EarthenVessel, 2},
    {Card::Arven, 2},             {Card::Crispin, 2},        {Card::ProfessorBurnet, 1},
    {Card::Serena, 1},            {Card::TateLiza, 1},       {Card::StevensResolve, 1},
    {Card::Guzma, 1},             {Card::Channeler, 1},      {Card::Gladion, 2},
    {Card::Lusamine, 1},          {Card::TeamYellsCheer, 1}, {Card::RoseannesBackup, 1},
    {Card::ProfessorTuro, 1},     {Card::ForestSealStone, 1},{Card::Powerglass, 1},
    {Card::FieldBlower, 1},       {Card::ChaoticSwell, 1},   {Card::PathToPeak, 1},
    {Card::HisuianHeavyBall, 1},  {Card::Grass, 6},          {Card::Fire, 3},
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

constexpr bool is_pokemon_v(const Card card) { return card == Card::RegidragoV; }
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

constexpr bool is_dragon_or_psychic(const Card card) {
  switch (card) {
    case Card::RegidragoV:
    case Card::RegidragoVstar:
    case Card::Dragapult:
    case Card::MegaDragonite:
    case Card::DialgaGX:
    case Card::GoodraVstar:
    case Card::TapuLeleGX:
    case Card::LatiasEx:
    case Card::Oricorio:
    case Card::Dipplin:
      return true;
    default:
      return false;
  }
}

constexpr bool is_payload(const Card card) {
  return card == Card::Dragapult || card == Card::MegaDragonite || card == Card::DialgaGX ||
         card == Card::GoodraVstar;
}

constexpr bool is_energy(const Card card) { return card == Card::Grass || card == Card::Fire; }
constexpr bool is_supporter(const Card card) {
  switch (card) {
    case Card::Arven:
    case Card::Crispin:
    case Card::ProfessorBurnet:
    case Card::Serena:
    case Card::TateLiza:
    case Card::StevensResolve:
    case Card::Guzma:
    case Card::Channeler:
    case Card::Gladion:
    case Card::Lusamine:
    case Card::TeamYellsCheer:
    case Card::RoseannesBackup:
    case Card::ProfessorTuro:
      return true;
    default:
      return false;
  }
}

enum class DciProfile : std::uint8_t { StrictJit, MatchupFlexJit, NoDiscardControl };
enum class LockMode : std::uint8_t { None, TurnTwoItem, FullItem, FullRuleBoxAbility, FullCombined };
enum class Tool : std::uint8_t { None, ForestSealStone, Powerglass };

struct Scenario {
  std::string label;
  DciProfile dci{DciProfile::StrictJit};
  LockMode locks{LockMode::None};
  bool going_first{true};
  int max_turn{4};
};

struct Pokemon {
  Card card{};
  int entered_turn{0};
  int grass{0};
  int fire{0};
  Tool tool{Tool::None};
};

struct State {
  std::vector<Card> deck;
  std::vector<Card> hand;
  std::vector<Card> prizes;
  std::vector<Card> discard;
  std::vector<Card> discarded_this_turn;
  std::optional<Pokemon> active;
  std::vector<Pokemon> bench;
  int turn{0};
  bool supporter_used{false};
  bool manual_energy_used{false};
  bool retreat_used{false};
  bool vstar_power_used{false};
  bool turn_ended{false};
};

struct TrialOutcome {
  bool ready_by_2{false};
  bool ready_by_3{false};
  bool ready_by_4{false};
  int first_ready_turn{0};
  int mulligans{0};
  bool started_regi{false};
  bool used_fss{false};
  bool used_legacy_star{false};
  bool used_steven{false};
  bool used_blender{false};
};

struct Aggregate {
