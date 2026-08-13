#pragma once

#include <cstdint>

namespace sim {

// Stable simulator card identifiers. Keep existing enumerator names and values
// during the incremental card-class migration so legacy engine code and migrated
// card modules can coexist in the same translation unit.
enum class Card : std::uint8_t {
  RegidragoV,
  RegidragoVstar,
  Dragapult,
  MegaDragonite,
  DialgaGX,
  GoodraVstar,
  TapuLeleGX,
  CrobatV,
  LatiasEx,
  MawileGX,
  Oricorio,
  Dipplin,
  Appletun,
  Pineco,
  ForretressEx,
  BrilliantBlender,
  SecretBox,
  BattleVipPass,
  MysteriousTreasure,
  QuickBall,
  UltraBall,
  EvolutionIncense,
  PokemonCommunication,
  EarthenVessel,
  ProfessorsLetter,
  Dawn,
  Grant,
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
  Klara,
  ProfessorTuro,
  ErikasInvitation,
  ForestSealStone,
  WishfulBaton,
  Powerglass,
  FieldBlower,
  ChaoticSwell,
  ForestOfVitality,
  PathToPeak,
  HisuianHeavyBall,
  DoubleDragonEnergy,
  Grass,
  Fire,
  GuzmaHala,
};

}  // namespace sim
