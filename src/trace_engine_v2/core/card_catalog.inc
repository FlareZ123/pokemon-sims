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
static_assert(cards::find_definition(Card::QuickBall) != nullptr);
static_assert(cards::find_definition(Card::QuickBall)->canonical_id == "swsh1-179");
static_assert(cards::find_definition(Card::QuickBall)->name == "Quick Ball");

constexpr std::array<std::pair<Card, int>, 36> kDeckRecipe{{
    {Card::RegidragoV, 4},        {Card::RegidragoVstar, 3}, {Card::Dragapult, 2},
    {Card::MegaDragonite, 2},     {Card::DialgaGX, 1},       {Card::GoodraVstar, 1},
    {Card::TapuLeleGX, 2},        {Card::LatiasEx, 1},       {Card::ErikasInvitation, 1},
    {Card::Oricorio, 1},          {Card::Dipplin, 1},       {Card::BrilliantBlender, 1},
    {Card::MysteriousTreasure, 4},{Card::QuickBall, 3},      {Card::EarthenVessel, 2},
    {Card::Arven, 2},             {Card::Crispin, 2},        {Card::ProfessorBurnet, 1},
    {Card::Serena, 1},            {Card::TateLiza, 1},       {Card::StevensResolve, 1},
    {Card::Guzma, 1},             {Card::Channeler, 1},      {Card::Gladion, 2},
    {Card::Lusamine, 1},          {Card::TeamYellsCheer, 1}, {Card::Klara, 1},
    {Card::ProfessorTuro, 1},     {Card::ForestSealStone, 1},{Card::Powerglass, 1},
    {Card::FieldBlower, 1},       {Card::ChaoticSwell, 1},   {Card::PathToPeak, 1},
    {Card::HisuianHeavyBall, 1},  {Card::Grass, 6},          {Card::Fire, 3},
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
