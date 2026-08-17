#pragma once

#include <array>
#include <cstdint>
#include <string_view>

#include "card_id.hpp"

namespace sim::cards {

enum class CardKind : std::uint8_t {
  Pokemon,
  Trainer,
  Energy,
};

enum class TrainerKind : std::uint8_t {
  None,
  Item,
  Supporter,
  Stadium,
  Tool,
};

enum class PokemonStage : std::uint8_t {
  None,
  Basic,
  Stage1,
  Stage2,
  VStar,
};

enum class PokemonType : std::uint8_t {
  None,
  Grass,
  Fire,
  Water,
  Lightning,
  Psychic,
  Fighting,
  Darkness,
  Metal,
  Dragon,
  Colorless,
};

// Intrinsic facts for one exact modeled print. Strategy roles such as payload,
// DCI value, route priority, and JIT timing intentionally remain outside this
// reusable schema.
struct CardDefinition {
  Card id;
  std::string_view canonical_id;
  std::string_view name;
  CardKind kind;
  TrainerKind trainer_kind{TrainerKind::None};
  PokemonStage pokemon_stage{PokemonStage::None};
  std::array<PokemonType, 2> pokemon_types{};
  std::uint8_t pokemon_type_count{};
  std::uint8_t retreat_cost{};
  bool rule_box{};
  bool pokemon_v{};
  bool ace_spec{};
  bool basic_energy{};
  std::string_view source_url;
};

// Keep reusable intrinsic classification beside CardDefinition rather than
// reproducing raw enum comparisons in each registry or card module.
// Cleanup architecture: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
class CardDefinitionPredicates final {
 public:
  [[nodiscard]] static constexpr bool is_kind(
      const CardDefinition& definition, const CardKind kind) {
    return definition.kind == kind;
  }

  [[nodiscard]] static constexpr bool is_trainer_kind(
      const CardDefinition& definition, const TrainerKind trainer_kind) {
    return is_kind(definition, CardKind::Trainer) &&
           definition.trainer_kind == trainer_kind;
  }

  [[nodiscard]] static constexpr bool has_pokemon_type(
      const CardDefinition& definition, const PokemonType type) {
    for (std::uint8_t index = 0; index < definition.pokemon_type_count; ++index) {
      if (definition.pokemon_types[index] == type) return true;
    }
    return false;
  }
};

constexpr bool is_trainer_kind(const CardDefinition& definition,
                               const TrainerKind trainer_kind) {
  return CardDefinitionPredicates::is_trainer_kind(definition, trainer_kind);
}

constexpr bool has_pokemon_type(const CardDefinition& definition,
                                const PokemonType type) {
  return CardDefinitionPredicates::has_pokemon_type(definition, type);
}

}  // namespace sim::cards
