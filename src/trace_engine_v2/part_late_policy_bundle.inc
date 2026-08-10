// Keep the late Secret Box, trace-provenance, and route-wrapper chain together.
// These includes intentionally preserve their existing order and macro lifetime from
// the translation-unit assembly in src/regidrago_sim.cpp.
#include "part_issue_2272_route_replaced_arven_quick_ball_override.inc"
#include "part_issue_1437_crispin_trace_override.inc"
#define trace trace_issue_1437_crispin_provenance
#include "part_issue_1118_secret_box.inc"
#undef trace
#include "part_issue_1369_celestial_roar_secret_box_override.inc"
