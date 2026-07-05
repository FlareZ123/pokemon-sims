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
