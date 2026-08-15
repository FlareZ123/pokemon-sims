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
#include "card_context_adapter.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <fcntl.h>
#include <sys/file.h>
#include <unistd.h>
#endif

namespace sim {

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

struct LegacyCardCatalog {
  // Deduce the fallback catalog size from its entries so future CardDefinition
  // migrations remove one compatibility row without a second size edit.
  // C++ class template argument deduction: https://eel.is/c++draft/temp.deduct.guide
  inline static constexpr std::array kNames{
      std::pair{Card::Dragapult, std::string_view{"Dragapult ex"}},
      std::pair{Card::MegaDragonite, std::string_view{"Mega Dragonite ex"}},
      std::pair{Card::DialgaGX, std::string_view{"Dialga-GX"}},
      std::pair{Card::GoodraVstar, std::string_view{"Hisuian Goodra VSTAR"}},
      std::pair{Card::TapuLeleGX, std::string_view{"Tapu Lele-GX"}},
      std::pair{Card::CrobatV, std::string_view{"Crobat V"}},
      std::pair{Card::LatiasEx, std::string_view{"Latias ex"}},
      std::pair{Card::Dipplin, std::string_view{"Dipplin TWM 127"}},
      std::pair{Card::Pineco, std::string_view{"Pineco"}},
      std::pair{Card::ForretressEx, std::string_view{"Forretress ex"}},
      std::pair{Card::UltraBall, std::string_view{"Ultra Ball"}},
      std::pair{Card::EarthenVessel, std::string_view{"Earthen Vessel"}},
      std::pair{Card::Grant, std::string_view{"Grant"}},
      std::pair{Card::Serena, std::string_view{"Serena"}},
      std::pair{Card::TateLiza, std::string_view{"Tate & Liza"}},
      std::pair{Card::StevensResolve, std::string_view{"Steven's Resolve"}},
      std::pair{Card::PathToPeak, std::string_view{"Path to the Peak"}},
      std::pair{Card::Grass, std::string_view{"Grass Energy"}},
      std::pair{Card::Fire, std::string_view{"Fire Energy"}},
  };

  [[nodiscard]] static constexpr auto find(const Card card) {
    // Keep the shrinking fallback catalog behind one lookup seam while registered
    // CardDefinition metadata remains the canonical name owner.
    // C++20 constexpr find_if: https://eel.is/c++draft/alg.find
    // Registry owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
    return std::find_if(kNames.begin(), kNames.end(),
                        [card](const auto& entry) { return entry.first == card; });
  }

  [[nodiscard]] static constexpr std::string_view name(const Card card) {
    const auto found = find(card);
    return found == kNames.end() ? "Unknown" : found->second;
  }
};

constexpr std::string_view name(const Card card) {
  // Registered metadata remains the canonical name owner; this function keeps
  // the shrinking legacy fallback isolated for cards not yet migrated.
  // Registry owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
  // Cleanup owner: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md#catalog-and-knowledge-cleanup
  if (const cards::CardDefinition* definition = cards::find_definition(card)) {
    return definition->name;
  }
  return LegacyCardCatalog::name(card);
}