#pragma once

#include <algorithm>
#include <array>
#include <string_view>
#include <utility>

#include "../../../cards/card_id.hpp"

namespace sim {

// Transitional fallback for legacy cards that do not yet own CardDefinition
// metadata. Registered definitions remain the canonical naming source.
// Registry owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
struct LegacyCardCatalog {
  inline static constexpr std::array<std::pair<Card, std::string_view>, 20> kNames{{
      {Card::RegidragoVstar, "Regidrago VSTAR"},
      {Card::Dragapult, "Dragapult ex"},
      {Card::MegaDragonite, "Mega Dragonite ex"},
      {Card::DialgaGX, "Dialga-GX"},
      {Card::GoodraVstar, "Hisuian Goodra VSTAR"},
      {Card::TapuLeleGX, "Tapu Lele-GX"},
      {Card::CrobatV, "Crobat V"},
      {Card::LatiasEx, "Latias ex"},
      {Card::Dipplin, "Dipplin TWM 127"},
      {Card::Pineco, "Pineco"},
      {Card::ForretressEx, "Forretress ex"},
      {Card::UltraBall, "Ultra Ball"},
      {Card::EarthenVessel, "Earthen Vessel"},
      {Card::Grant, "Grant"},
      {Card::Serena, "Serena"},
      {Card::TateLiza, "Tate & Liza"},
      {Card::StevensResolve, "Steven's Resolve"},
      {Card::PathToPeak, "Path to the Peak"},
      {Card::Grass, "Grass Energy"},
      {Card::Fire, "Fire Energy"},
  }};

  [[nodiscard]] static constexpr auto find(const Card card) {
    // C++20 constexpr find_if: https://eel.is/c++draft/alg.find
    return std::find_if(kNames.begin(), kNames.end(),
                        [card](const auto& entry) { return entry.first == card; });
  }

  [[nodiscard]] static constexpr std::string_view name(const Card card) {
    const auto found = find(card);
    return found == kNames.end() ? "Unknown" : found->second;
  }
};

}  // namespace sim
