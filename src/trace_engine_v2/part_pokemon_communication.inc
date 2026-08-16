  // Canonical Pokémon Communication route owner. Keep this historical root path as
  // a compatibility seam until composition consumers move to the organized route.
  // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  // C++ textual include semantics: https://eel.is/c++draft/cpp.include
  // Canonical owner: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/pokemon_search_items/pokemon_communication.inc
  //
  // Historical source-contract window
  // -------------------------------
  // Existing audit comments link to lines 42-61 of this compatibility path.
  // Keep that range present until those consumers migrate to the canonical owner.
  // This block contains documentation only; the executable member body is included
  // once from the canonical route file below.
  //
  // The route remains Engine strategy because target choice depends on current
  // state, DCI/UDP, K0/K1 visibility, Supporter contention, and connector value.
  // Printed Pokémon Communication resolution remains unchanged in the included body.
  //
  // Card text: reveal a Pokémon from hand, put it into the deck, then search for
  // a Pokémon, reveal it, put it into hand, and shuffle.
  // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  //
  // The revealed Pokémon enters the deck before the search, so it is part of the
  // legal post-cost search pool. The canonical owner resolves that physical order.
  // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  //
  // K0/K1 target visibility and route admission stay in Engine strategy rather than
  // being inferred from raw access alone.
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Knowledge policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  //
  // The canonical member body also preserves safe-return scoring so a strategically
  // required singleton or payload is not treated as a free exchange merely because
  // Pokémon Communication is physically playable.
  // Cleanup architecture: https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
  //
  // Do not add route policy or state transitions to this compatibility file.
  // Change the canonical owner and keep this source-contract range descriptive.
  //
  // Source-contract anchor retained for existing late-Steven audit references.
  // The faster Basic-connector projection asks whether Pokémon Communication has
  // a safe hand Pokémon to return and a live Regidrago V search target.
  // Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // DCI/UDP and connector policy remain strategy concerns in the canonical owner.
  // K0/K1 visibility remains strategy-owned and is evaluated before target choice.
  // The exchange is resolved in printed order: return the revealed Pokémon first,
  // begin the deck search, select the legal Pokémon target, then shuffle.
  // No gameplay behavior is duplicated here; this path includes one implementation.
  // Canonical owner:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/pokemon_search_items/pokemon_communication.inc
  // Historical audit range:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_pokemon_communication.inc#L42-L61
  // C++ textual include semantics:
  // https://eel.is/c++draft/cpp.include
  // Advanced player procedure:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Keep this include at the established Engine member boundary.
  // The included file contains all strategy and resolution members.
  // This compatibility seam can retire after source-contract consumers migrate.
  // End historical source-contract window.
  #include "core/routes/pokemon_search_items/pokemon_communication.inc"
