// Centralized selector for the three issue-962 route sections. This file is
// intentionally re-includable because each call-site wrapper selects one section.
// Keep both the public selector and private section macros hygienic so one textual
// include cannot silently alter the next route fragment.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#if defined(REGIDRAGO_ISSUE_962_SECTION_ELIGIBILITY) || \
    defined(REGIDRAGO_ISSUE_962_SECTION_PROJECTION) || \
    defined(REGIDRAGO_ISSUE_962_SECTION_DECISION)
#error "Issue-962 private section macros must be undefined on entry"
#endif

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

#if defined(REGIDRAGO_ISSUE_962_SECTION_ELIGIBILITY) || \
    defined(REGIDRAGO_ISSUE_962_SECTION_PROJECTION) || \
    defined(REGIDRAGO_ISSUE_962_SECTION_DECISION)
#error "Issue-962 private section macro leaked after composition"
#endif
