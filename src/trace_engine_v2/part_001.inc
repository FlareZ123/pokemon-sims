    case Card::Oricorio: return "Oricorio GRI 55";
    case Card::Dipplin: return "Dipplin TWM 127";
    case Card::Appletun: return "Appletun sv8-140";
    case Card::Pineco: return "Pineco";
    case Card::ForretressEx: return "Forretress ex";
    case Card::BrilliantBlender: return "Brilliant Blender";
    case Card::SecretBox: return "Secret Box";
    case Card::MysteriousTreasure: return "Mysterious Treasure";
    case Card::QuickBall: return "Quick Ball";
    case Card::UltraBall: return "Ultra Ball";
    case Card::EvolutionIncense: return "Evolution Incense";
    case Card::PokemonCommunication: return "Pokémon Communication";
    case Card::EarthenVessel: return "Earthen Vessel";
    case Card::Dawn: return "Dawn";
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
    case Card::ProfessorTuro: return "Professor Turo's Scenario";
    case Card::ErikasInvitation: return "Erika's Invitation";
    case Card::ForestSealStone: return "Forest Seal Stone";
    case Card::WishfulBaton: return "Wishful Baton";
    case Card::Powerglass: return "Powerglass";
    case Card::FieldBlower: return "Field Blower";
    case Card::ChaoticSwell: return "Chaotic Swell";
    case Card::ForestOfVitality: return "Forest of Vitality";
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
  return card == Card::RegidragoV || card == Card::RegidragoVstar;
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
constexpr bool is_payload(const Card card) {
  return card == Card::Dragapult || card == Card::MegaDragonite || card == Card::DialgaGX ||
         card == Card::GoodraVstar || card == Card::Appletun;
}

constexpr int retreat_cost(const Card card) {
  // Printed Retreat Costs for the opening-position recovery modeled by the
  // Pineco/Forretress ex route:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  switch (card) {
    case Card::TapuLeleGX: return 1;
    case Card::Pineco: return 2;
    case Card::DialgaGX: return 3;
    case Card::RegidragoV: return 3;  // Exact swsh12-135 print: https://api.pokemontcg.io/v2/cards/swsh12-135
    case Card::RegidragoVstar: return 3;
    case Card::LatiasEx: return 1;
    case Card::Oricorio: return 1;
    default: return 0;
  }
}

constexpr bool is_energy(const Card card) { return card == Card::Grass || card == Card::Fire; }
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
