constexpr bool is_basic(const Card card) {
  switch (card) {
    case Card::RegidragoV:
    case Card::DialgaGX:
    case Card::TapuLeleGX:
    case Card::CrobatV:  // Basic Pokémon V: https://api.pokemontcg.io/v2/cards/swsh3-104
    case Card::LatiasEx:
    case Card::MawileGX:
    case Card::Oricorio:
    case Card::Pineco:
      return true;
    default:
      return false;
  }
}

// Ultra Ball may search any Pokémon, so K1 target legality must recognize every
// Pokémon represented by the simulator rather than only Basic or payload subsets:
// https://api.pokemontcg.io/v2/cards/swsh12pt5-146
constexpr bool is_pokemon(const Card card) {
  switch (card) {
    case Card::RegidragoV:
    case Card::RegidragoVstar:
    case Card::Dragapult:
    case Card::MegaDragonite:
    case Card::DialgaGX:
    case Card::GoodraVstar:
    case Card::TapuLeleGX:
    case Card::CrobatV:  // Pokémon identity: https://api.pokemontcg.io/v2/cards/swsh3-104
    case Card::LatiasEx:
    case Card::MawileGX:
    case Card::Oricorio:
    case Card::Dipplin:
    case Card::Appletun:
    case Card::Pineco:
    case Card::ForretressEx:
      return true;
    default:
      return false;
  }
}

// TPCi defines Pokémon V to include Pokémon VSTAR. Forest Seal Stone can remain
// usable after its Pokémon V evolves into a Pokémon VSTAR:
// https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
// https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
constexpr bool is_pokemon_v(const Card card) {
  return card == Card::RegidragoV || card == Card::RegidragoVstar ||
         card == Card::CrobatV;  // Pokémon V: https://api.pokemontcg.io/v2/cards/swsh3-104
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
    case Card::CrobatV:  // Rule Box Pokémon V: https://api.pokemontcg.io/v2/cards/swsh3-104
    case Card::LatiasEx:
    case Card::MawileGX:
    case Card::ForretressEx:
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
    case Card::Appletun:
      return true;
    default:
      return false;
  }
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
  switch (card) {
    case Card::RegidragoV:
    case Card::RegidragoVstar:
    case Card::Dragapult:
    case Card::MegaDragonite:
    case Card::DialgaGX:
    case Card::GoodraVstar:
    case Card::Dipplin:
    case Card::Appletun:
      return true;
    default:
      return false;
  }
}

constexpr bool is_payload(const Card card) {
  return card == Card::Dragapult || card == Card::MegaDragonite || card == Card::DialgaGX ||
         card == Card::GoodraVstar || card == Card::Appletun;
}

constexpr int retreat_cost(const Card card) {
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
    case Card::RegidragoV: return 3;  // Exact swsh12-135 print: https://api.pokemontcg.io/v2/cards/swsh12-135
    case Card::RegidragoVstar: return 3;
    case Card::LatiasEx: return 2;  // Exact sv8-76 print: https://api.pokemontcg.io/v2/cards/sv8-76
    case Card::MawileGX: return 1;  // Exact sm11-141 print: https://api.pokemontcg.io/v2/cards/sm11-141 ; issue: https://github.com/FlareZ123/pokemon-sims/issues/1301
    case Card::Oricorio: return 1;
    default: return 0;
  }
}

// Grass and Fire are the modeled Basic Energy cards. DDE is a Special
// Energy, so Basic-only searches must continue to use is_basic_energy or
// their explicit Grass/Fire target lists:
// https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
// https://github.com/FlareZ123/pokemon-sims/issues/2238
constexpr bool is_basic_energy(const Card card) {
  return card == Card::Grass || card == Card::Fire;
}
constexpr bool is_energy(const Card card) {
  return is_basic_energy(card) || card == Card::DoubleDragonEnergy;
}
constexpr bool is_supporter(const Card card) {
  switch (card) {
    case Card::Dawn:
    case Card::Grant:
    case Card::Arven:
    case Card::Crispin:
    case Card::ProfessorBurnet:
    case Card::Serena:
    case Card::TateLiza:
    case Card::StevensResolve:
    case Card::Guzma:
    case Card::Channeler:
    case Card::Gladion:
    case Card::ErikasInvitation:
    case Card::Lusamine:
    case Card::TeamYellsCheer:
    case Card::RoseannesBackup:
    case Card::Klara:
    case Card::ProfessorTuro:
      return true;
    default:
      return false;
  }
}

constexpr bool is_item(const Card card) {
  switch (card) {
    case Card::BrilliantBlender:
    case Card::SecretBox:
    case Card::BattleVipPass:
    case Card::MysteriousTreasure:
    case Card::QuickBall:
    case Card::UltraBall:
    case Card::EvolutionIncense:
    case Card::PokemonCommunication:
    case Card::EarthenVessel:
    case Card::ProfessorsLetter:  // https://api.pokemontcg.io/v2/cards/xy1-123 ; https://github.com/FlareZ123/pokemon-sims/issues/2509
    case Card::FieldBlower:
    case Card::HisuianHeavyBall:
      return true;
    default:
      return false;
  }
}

constexpr bool is_tool(const Card card) {
  return card == Card::ForestSealStone || card == Card::WishfulBaton ||
         card == Card::Powerglass;
}

constexpr bool is_stadium(const Card card) {
  return card == Card::ChaoticSwell || card == Card::ForestOfVitality ||
         card == Card::PathToPeak;
}

constexpr bool is_ace_spec(const Card card) {
  return card == Card::BrilliantBlender || card == Card::SecretBox;
}
