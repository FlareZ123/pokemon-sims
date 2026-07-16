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
    case Card::TurboBlank: return "Turbo sensitivity blank";
    case Card::BattleVipPass: return "Battle VIP Pass";
    case Card::PokeStop: return "PokéStop";
    case Card::Carmine: return "Carmine";
    case Card::ProfessorsResearch: return "Professor's Research";
    case Card::PokemonCommunication: return "Pokémon Communication";
    case Card::Pokegear: return "Pokégear 3.0";
    case Card::Lillie: return "Lillie";
    case Card::NestBall: return "Nest Ball";
    case Card::AdventurersDiscovery: return "Adventurer's Discovery";
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

// This is the model's A/S-tier Apex payload and readiness set. Dipplin TWM 127
// is a Dragon Pokémon for legal search effects: https://api.pokemontcg.io/v2/cards/sv6-127
// Maintainer policy excludes Syrup Catcher from the A/S win-condition set, so
// Dipplin never satisfies readiness or opens a payload-discard route.
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