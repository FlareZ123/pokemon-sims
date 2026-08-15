constexpr const cards::CardDefinition* registered_pokemon_definition(const Card card) {
  const cards::CardDefinition* definition = cards::find_definition(card);
  return definition != nullptr && definition->kind == cards::CardKind::Pokemon
             ? definition
             : nullptr;
}

struct LegacyPokemonClassification {
  // Registered Pokémon are classified from exact-print CardDefinition metadata.
  // These tables contain only the shrinking compatibility set that has not migrated:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
  inline static constexpr std::array<Card, 5> kBasic{{
      Card::DialgaGX,
      Card::TapuLeleGX,
      Card::CrobatV,
      Card::LatiasEx,
      Card::Pineco,
  }};

  inline static constexpr std::array<Card, 11> kPokemon{{
      Card::RegidragoVstar,
      Card::Dragapult,
      Card::MegaDragonite,
      Card::DialgaGX,
      Card::GoodraVstar,
      Card::TapuLeleGX,
      Card::CrobatV,
      Card::LatiasEx,
      Card::Dipplin,
      Card::Pineco,
      Card::ForretressEx,
  }};

  inline static constexpr std::array<Card, 3> kPokemonV{{
      Card::RegidragoVstar,
      Card::GoodraVstar,
      Card::CrobatV,
  }};

  inline static constexpr std::array<Card, 9> kRuleBox{{
      Card::RegidragoVstar,
      Card::Dragapult,
      Card::MegaDragonite,
      Card::DialgaGX,
      Card::GoodraVstar,
      Card::TapuLeleGX,
      Card::CrobatV,
      Card::LatiasEx,
      Card::ForretressEx,
  }};

  inline static constexpr std::array<Card, 8> kDragonOrPsychic{{
      Card::RegidragoVstar,
      Card::Dragapult,
      Card::MegaDragonite,
      Card::DialgaGX,
      Card::GoodraVstar,
      Card::TapuLeleGX,
      Card::LatiasEx,
      Card::Dipplin,
  }};

  inline static constexpr std::array<Card, 6> kDragon{{
      Card::RegidragoVstar,
      Card::Dragapult,
      Card::MegaDragonite,
      Card::DialgaGX,
      Card::GoodraVstar,
      Card::Dipplin,
  }};

  template <std::size_t N>
  [[nodiscard]] static constexpr bool contains(
      const std::array<Card, N>& cards, const Card card) {
    for (const Card candidate : cards) {
      if (candidate == card) return true;
    }
    return false;
  }
};

constexpr bool is_basic(const Card card) {
  if (const cards::CardDefinition* definition = registered_pokemon_definition(card)) {
    return definition->pokemon_stage == cards::PokemonStage::Basic;
  }
  return LegacyPokemonClassification::contains(
      LegacyPokemonClassification::kBasic, card);
}

// Ultra Ball may search any Pokémon, so K1 target legality must recognize every
// Pokémon represented by the simulator rather than only Basic or payload subsets:
// https://api.pokemontcg.io/v2/cards/swsh12pt5-146
constexpr bool is_pokemon(const Card card) {
  if (registered_pokemon_definition(card) != nullptr) return true;
  return LegacyPokemonClassification::contains(
      LegacyPokemonClassification::kPokemon, card);
}

// TPCi defines Pokémon V to include Pokémon VSTAR. Forest Seal Stone can remain
// usable after its Pokémon V evolves into a Pokémon VSTAR:
// https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
// https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
constexpr bool is_pokemon_v(const Card card) {
  if (const cards::CardDefinition* definition = registered_pokemon_definition(card)) {
    return definition->pokemon_v;
  }
  return LegacyPokemonClassification::contains(
      LegacyPokemonClassification::kPokemonV, card);
}

// Keep Regidrago evolution-line identity beside the other intrinsic Pokémon classifiers.
// Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
// Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
[[nodiscard]] constexpr bool is_regidrago_v_line(const Card card) {
  return card == Card::RegidragoV || card == Card::RegidragoVstar;
}

constexpr bool is_rule_box_pokemon(const Card card) {
  if (const cards::CardDefinition* definition = registered_pokemon_definition(card)) {
    return definition->rule_box;
  }
  return LegacyPokemonClassification::contains(
      LegacyPokemonClassification::kRuleBox, card);
}

constexpr bool is_dragon_or_psychic(const Card card) {
  if (const cards::CardDefinition* definition = registered_pokemon_definition(card)) {
    return cards::has_pokemon_type(*definition, cards::PokemonType::Dragon) ||
           cards::has_pokemon_type(*definition, cards::PokemonType::Psychic);
  }
  return LegacyPokemonClassification::contains(
      LegacyPokemonClassification::kDragonOrPsychic, card);
}

// This is the model's A/S-tier Apex payload and readiness set. Dipplin TWM 127
// is a Dragon Pokémon for legal search effects: https://api.pokemontcg.io/v2/cards/sv6-127
// Maintainer policy excludes Syrup Catcher from the A/S win-condition set, so
// Dipplin never satisfies readiness or opens a payload-discard route.
// DDE may remain attached only to Dragon Pokémon. Keep this classifier
// separate from Mysterious Treasure's combined Psychic-or-Dragon target set:
// https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
// https://github.com/FlareZ123/pokemon-sims/issues/2238
constexpr bool is_dragon(const Card card) {
  if (const cards::CardDefinition* definition = registered_pokemon_definition(card)) {
    return cards::has_pokemon_type(*definition, cards::PokemonType::Dragon);
  }
  return LegacyPokemonClassification::contains(
      LegacyPokemonClassification::kDragon, card);
}

constexpr bool is_payload(const Card card) {
  return card == Card::Dragapult || card == Card::MegaDragonite || card == Card::DialgaGX ||
         card == Card::GoodraVstar || card == Card::Appletun;
}

constexpr int retreat_cost(const Card card) {
  // Registered exact-print metadata is the canonical Retreat Cost owner. The
  // remaining switch is only the shrinking compatibility set:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
  if (const cards::CardDefinition* definition = registered_pokemon_definition(card)) {
    return definition->retreat_cost;
  }

  // Retreat requires discarding attached Energy whose total meets or exceeds the printed Retreat Cost:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Printed Retreat Costs for the opening-position recovery modeled by the
  // Pineco/Forretress ex route:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  switch (card) {
    case Card::TapuLeleGX: return 1;
    case Card::CrobatV: return 1;  // Printed Retreat Cost: https://api.pokemontcg.io/v2/cards/swsh3-104
    case Card::Pineco: return 2;
    case Card::DialgaGX: return 3;
    case Card::RegidragoVstar: return 3;
    case Card::Dragapult: return 1;  // Exact sv6-130 print: https://api.pokemontcg.io/v2/cards/sv6-130
    case Card::MegaDragonite: return 2;  // Exact me2pt5-152 print: https://api.pokemontcg.io/v2/cards/me2pt5-152
    case Card::GoodraVstar: return 3;  // Exact swsh11-136 print: https://api.pokemontcg.io/v2/cards/swsh11-136
    case Card::Dipplin: return 2;  // Exact sv6-127 print: https://api.pokemontcg.io/v2/cards/sv6-127
    case Card::ForretressEx: return 3;  // Exact sv4pt5-2 print: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    case Card::LatiasEx: return 2;  // Exact sv8-76 print: https://api.pokemontcg.io/v2/cards/sv8-76
    default: return 0;
  }
}

// Grass and Fire are the modeled Basic Energy cards. Registered Energy cards
// own intrinsic Energy and Basic/Special classification in CardDefinition.
// Double Dragon Energy exact Special Energy print: https://api.pokemontcg.io/v2/cards/xy6-97
constexpr bool is_basic_energy(const Card card) {
  if (cards::has_definition(card)) return cards::registered_is_basic_energy(card);
  return card == Card::Grass || card == Card::Fire;
}
constexpr bool is_energy(const Card card) {
  if (cards::has_definition(card)) return cards::registered_is_energy(card);
  return is_basic_energy(card);
}
constexpr bool is_special_energy(const Card card) {
  if (cards::has_definition(card)) return cards::registered_is_special_energy(card);
  return false;
}
constexpr bool is_supporter(const Card card) {
  // Registered Supporters own intrinsic subtype classification in CardDefinition.
  // Erika's Invitation exact Supporter print: https://api.pokemontcg.io/v2/cards/sv3pt5-160 ; migration: https://github.com/FlareZ123/pokemon-sims/issues/3598
  // Guzma exact Supporter print: https://api.pokemontcg.io/v2/cards/sm3-115 ; migration: https://github.com/FlareZ123/pokemon-sims/issues/3618
  // Gladion exact Supporter print: https://api.pokemontcg.io/v2/cards/sm4-95 ; migration: https://github.com/FlareZ123/pokemon-sims/issues/3604
  // Team Yell's Cheer exact Supporter print: https://api.pokemontcg.io/v2/cards/swsh9-149 ; migration: https://github.com/FlareZ123/pokemon-sims/issues/3620
  // Advanced Supporter procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  if (cards::has_definition(card)) return cards::registered_is_supporter(card);

  switch (card) {
    case Card::Grant:
    case Card::Serena:
    case Card::TateLiza:
    case Card::StevensResolve:
      return true;
    default:
      return false;
  }
}

constexpr bool is_item(const Card card) {
  // Migrated cards own intrinsic Item classification in the explicit card registry.
  // Registry: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
  // Pokémon Communication exact Item print: https://api.pokemontcg.io/v2/cards/sm9-152
  if (cards::has_definition(card)) return cards::registered_is_item(card);

  switch (card) {
    case Card::UltraBall:
    case Card::EarthenVessel:
      return true;
    default:
      return false;
  }
}

constexpr bool is_tool(const Card card) {
  // Registered Tools own their intrinsic Trainer subtype in CardDefinition.
  // Registry: https://github.com/FlareZ123/pokemon-sims/blob/main/src/cards/card_registry.hpp
  // Wishful Baton exact print: https://api.pokemontcg.io/v2/cards/sm3-128 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3631
  if (cards::has_definition(card)) {
    return cards::registered_is_trainer_kind(card, cards::TrainerKind::Tool);
  }
  return false;
}

constexpr bool is_stadium(const Card card) {
  // Registered Stadiums own their intrinsic Trainer subtype in CardDefinition.
  // Forest of Vitality exact print: https://api.pokemontcg.io/v2/cards/me1-117 ; cleanup: https://github.com/FlareZ123/pokemon-sims/issues/3639
  if (cards::has_definition(card)) {
    return cards::registered_is_trainer_kind(card, cards::TrainerKind::Stadium);
  }
  return card == Card::PathToPeak;
}

constexpr bool is_ace_spec(const Card card) {
  // Registered ACE SPEC cards own this intrinsic rarity rule in CardDefinition.
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  if (cards::has_definition(card)) return cards::registered_is_ace_spec(card);
  return false;
}
