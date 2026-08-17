  class ProfessorBurnetSelectionPolicy final {
   public:
    inline static constexpr std::size_t kSelectionLimit = 2U;

    inline static constexpr std::array<Card, 5> kPayloadPriority{{
        Card::MegaDragonite,
        Card::Dragapult,
        Card::GoodraVstar,
        Card::DialgaGX,
        Card::Appletun,
    }};

    inline static constexpr std::array<Card, 6> kDeadThinningPriority{{
        Card::PathToPeak,
        Card::ErikasInvitation,
        Card::Guzma,
        Card::Channeler,
        Card::FieldBlower,
        Card::ChaoticSwell,
    }};
  };

  // Professor Burnet searches for and discards up to 2 cards from the deck:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // The payload order is strategy-owned because Apex Dragon copies attacks from
  // Dragon Pokemon in the discard pile: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Keep route selection policy in Engine while centralizing its fixed constants:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
