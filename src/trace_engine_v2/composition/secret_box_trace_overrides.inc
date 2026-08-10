#pragma once

// Keep the Crispin provenance trace shim adjacent to the Secret Box implementation
// it wraps, followed by the Celestial Roar bridge that consumes that implementation.
// C++ preprocessing include semantics: https://eel.is/c++draft/cpp.include
#if defined(trace)
#error "Secret Box trace composition requires trace to be undefined on entry"
#endif

#include "../part_issue_1437_crispin_trace_override.inc"
#define trace trace_issue_1437_crispin_provenance
#include "../part_issue_1118_secret_box.inc"
#undef trace
#include "../part_issue_1369_celestial_roar_secret_box_override.inc"

#if defined(trace)
#error "Secret Box trace composition leaked trace"
#endif
