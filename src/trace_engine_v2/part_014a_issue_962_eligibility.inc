// Compose all issue-962 route sections at this class-member boundary.
// Keep eligibility, projection, and decision in dependency order because later
// sections consume lambdas introduced by the preceding section.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
#define REGIDRAGO_ISSUE_962_SECTION_ELIGIBILITY
#include "core/routes/issue_962_route.inc"
#undef REGIDRAGO_ISSUE_962_SECTION_ELIGIBILITY

#define REGIDRAGO_ISSUE_962_SECTION_PROJECTION
#include "core/routes/issue_962_route.inc"
#undef REGIDRAGO_ISSUE_962_SECTION_PROJECTION

#define REGIDRAGO_ISSUE_962_SECTION_DECISION
#include "core/routes/issue_962_route.inc"
#undef REGIDRAGO_ISSUE_962_SECTION_DECISION
