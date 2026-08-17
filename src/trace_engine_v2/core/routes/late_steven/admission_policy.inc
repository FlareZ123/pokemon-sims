  struct LateStevenAdmissionPolicy final {
    static bool required_actions_available(
        const bool item_available,
        const bool supporter_available,
        const bool latias_ability_available) {
      // This policy composes already-resolved permissions only. Engine remains the
      // owner of dynamic Item, Supporter, and Ability lock state.
      // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
      // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
      // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
      // Advanced Item, Supporter, Stadium, and Ability procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
      return item_available && supporter_available && latias_ability_available;
    }
  };
