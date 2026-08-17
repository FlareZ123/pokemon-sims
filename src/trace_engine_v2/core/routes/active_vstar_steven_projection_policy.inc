  class ActiveVstarStevenProjectionPolicy final {
   public:
    [[nodiscard]] static constexpr int failing_topdecks(
        const int grass_after_steven, const int fire_in_deck) {
      // Preserve the established known-deck topdeck-risk model for Steven's
      // Resolve -> Crispin. After Steven banks one Grass, a lone remaining Grass
      // or lone Fire is counted as a failing next draw for the deterministic
      // continuation. This class owns only the pure arithmetic projection; Engine
      // keeps K1 visibility, route admission, lock checks, DCI/JIT, and actions.
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
      // Route-risk provenance: https://github.com/FlareZ123/pokemon-sims/issues/1098
      // Cleanup architecture: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
      return static_cast<int>(grass_after_steven == 1) +
             static_cast<int>(fire_in_deck == 1);
    }

    [[nodiscard]] static constexpr bool continuation_survives_next_draw(
        const int post_steven_deck_size, const int grass_after_steven,
        const int fire_in_deck) {
      return post_steven_deck_size >
             failing_topdecks(grass_after_steven, fire_in_deck);
    }
  };
