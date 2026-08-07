// Keep the complete-route Tapu selector intact and add only #2270's newly
// confirmed K1 Arven -> Quick Ball -> Latias branch at the final connector gate.
// Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/2270
#define needs_tapu_connector needs_tapu_connector_issue2270_original
#include "part_issue_989_wonder_tag_complete_route_issue2270_original.inc"
#undef needs_tapu_connector
#include "part_issue_2270_tapu_connector_override.inc"
