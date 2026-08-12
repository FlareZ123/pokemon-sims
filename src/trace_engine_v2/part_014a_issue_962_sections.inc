// Centralized composition seam for issue #962 route sections.
// Core implementation: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/issue_962_route.inc
#define REGIDRAGO_ISSUE_962_SECTION_ELIGIBILITY
#include "core/issue_962_route.inc"
#undef REGIDRAGO_ISSUE_962_SECTION_ELIGIBILITY

#define REGIDRAGO_ISSUE_962_SECTION_PROJECTION
#include "core/issue_962_route.inc"
#undef REGIDRAGO_ISSUE_962_SECTION_PROJECTION

#define REGIDRAGO_ISSUE_962_SECTION_DECISION
#include "core/issue_962_route.inc"
#undef REGIDRAGO_ISSUE_962_SECTION_DECISION
