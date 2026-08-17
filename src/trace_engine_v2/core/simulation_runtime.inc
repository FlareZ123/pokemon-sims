// Runtime support is composed in declaration order from focused ownership units.
// Keep state before metrics before trace so downstream Engine formatting helpers see
// the same complete types at the historical boundary.
// C++ textual include semantics: https://eel.is/c++draft/cpp.include
#include "runtime/state.inc"
#include "runtime/metrics.inc"
#include "runtime/trace_log.inc"
