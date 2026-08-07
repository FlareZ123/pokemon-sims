  // Preserve #648's existing helper as the base implementation, then expose the
  // #2270 K1 Quick Ball -> Latias completion as the active helper.
  // C++ class-scope lookup permits the wrapper to call the later member definition:
  // https://eel.is/c++draft/class.member.lookup
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2270
#include "part_issue_2270_wonder_tag_arven_latias_override.inc"
#define wonder_tag_arven_held_payload_route_available \
  wonder_tag_arven_held_payload_route_available_issue2270_original
#include "part_006_issue2270_original.inc"
#undef wonder_tag_arven_held_payload_route_available
