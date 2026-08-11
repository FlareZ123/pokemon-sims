#pragma once

// Own the part_012 supporter selector alias boundary in one reusable composition
// unit. The compatibility body includes this file at the historical textual point.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Arven: https://api.pokemontcg.io/v2/cards/sv1-166
// Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
// Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
#if defined(play_arven) || defined(play_gladion) || defined(play_serena)
#error "Supporter selector composition requires aliases to be undefined on entry"
#endif

#define play_arven play_arven_original
#define play_gladion play_gladion_original
#define play_serena play_serena_issue1683_original
#include "../part_012.inc"
#undef play_serena
#undef play_gladion
#undef play_arven

#if defined(play_arven) || defined(play_gladion) || defined(play_serena)
#error "Supporter selector composition leaked a local alias"
#endif
