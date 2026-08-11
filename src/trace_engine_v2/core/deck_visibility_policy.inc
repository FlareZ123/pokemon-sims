#pragma once

// Keep visibility activation as the compatibility boundary while the implementation
// lives with the deck-state knowledge and public-inventory helpers it depends on.
// K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
// Empty-deck visibility follows public game state: https://www.pokemon.com/us/pokemon-tcg/rules
#define REGIDRAGO_DECK_VISIBILITY_POLICY
#include "deck_knowledge.inc"
#undef REGIDRAGO_DECK_VISIBILITY_POLICY
