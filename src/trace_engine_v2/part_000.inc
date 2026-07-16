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
  TurboBlank,
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