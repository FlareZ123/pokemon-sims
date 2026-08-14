// Single composition boundary for the issue-962 route policy.
// Eligibility introduces admission and Quick Ball-cost helpers; projection adds
// the shadow-state helpers consumed by decision; decision closes the route.
// Keep this exact dependency order and keep the shared policy implementation in
// core/routes/issue_962_route.inc.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
// Route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/routes/issue_962_route.inc

#define REGIDRAGO_ISSUE_962_SECTION_ELIGIBILITY
#include "core/routes/issue_962_route.inc"
#undef REGIDRAGO_ISSUE_962_SECTION_ELIGIBILITY

#define REGIDRAGO_ISSUE_962_SECTION_PROJECTION
#include "core/routes/issue_962_route.inc"
#undef REGIDRAGO_ISSUE_962_SECTION_PROJECTION

#define REGIDRAGO_ISSUE_962_SECTION_DECISION
#include "core/routes/issue_962_route.inc"
#undef REGIDRAGO_ISSUE_962_SECTION_DECISION
