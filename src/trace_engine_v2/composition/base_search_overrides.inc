// Ordered composition for the first search-item wrapper layer. The aliases are
// intentionally left in the same state that the following legacy parts expect.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include

#define play_mysterious_treasure play_mysterious_treasure_empty_deck_original
#define play_heavy_ball play_heavy_ball_prize_payload_original
#include "../part_008b.inc"
#undef play_heavy_ball
// Wonder Tag is copy-specific. Preserve all existing selector controls while an
// in-play Tapu no longer suppresses a second physical copy:
// https://api.pokemontcg.io/v2/cards/sm2-60
// https://github.com/FlareZ123/pokemon-sims/issues/746
#define in_play tapu_connector_copy_aware_in_play
#include "../part_009a.inc"
#undef in_play
#undef play_mysterious_treasure
#define play_quick_ball play_quick_ball_empty_deck_original
#define in_play tapu_connector_copy_aware_in_play
#include "../part_009b1.inc"
#undef in_play
#undef play_quick_ball
#define play_ultra_ball play_ultra_ball_original
#include "../part_009b2.inc"
