// Centralized selector for the three issue-962 route sections. This file is
// intentionally re-includable because each call-site wrapper selects one section.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#if (defined(REGIDRAGO_ISSUE_962_SELECT_ELIGIBILITY) + \
     defined(REGIDRAGO_ISSUE_962_SELECT_PROJECTION) + \
     defined(REGIDRAGO_ISSUE_962_SELECT_DECISION)) != 1
#error "Select exactly one issue-962 route section"
#endif

#if defined(REGIDRAGO_ISSUE_962_SELECT_ELIGIBILITY)
#define REGIDRAGO_ISSUE_962_SECTION_ELIGIBILITY
#elif defined(REGIDRAGO_ISSUE_962_SELECT_PROJECTION)
#define REGIDRAGO_ISSUE_962_SECTION_PROJECTION
#else
#define REGIDRAGO_ISSUE_962_SECTION_DECISION
#endif

#include "../core/issue_962_route.inc"

#if defined(REGIDRAGO_ISSUE_962_SELECT_ELIGIBILITY)
#undef REGIDRAGO_ISSUE_962_SECTION_ELIGIBILITY
#elif defined(REGIDRAGO_ISSUE_962_SELECT_PROJECTION)
#undef REGIDRAGO_ISSUE_962_SECTION_PROJECTION
#else
#undef REGIDRAGO_ISSUE_962_SECTION_DECISION
#endif
