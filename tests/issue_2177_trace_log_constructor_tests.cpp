#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <stdexcept>
#include <string>
#include <type_traits>

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_default_construction() {
  const sim::TraceLog trace;
  expect(!trace.enabled, "A default TraceLog unexpectedly enabled tracing.");
  expect(trace.lines.empty(), "A default TraceLog started with trace lines.");
  expect(trace.last_policy_state_by_key.empty(),
         "A default TraceLog started with policy-state history.");
}

void test_existing_two_argument_construction() {
  // A declared constructor keeps the repository's existing TraceLog{true, {}}
  // call sites source-compatible and initializes the policy-state map explicitly:
  // https://eel.is/c++draft/dcl.init.aggr
  // https://github.com/FlareZ123/pokemon-sims/issues/2177
  const sim::TraceLog trace{true, {"seed line"}};
  expect(trace.enabled, "Two-argument TraceLog construction lost enabled state.");
  expect(trace.lines.size() == 1 && trace.lines.front() == "seed line",
         "Two-argument TraceLog construction lost initial trace lines.");
  expect(trace.last_policy_state_by_key.empty(),
         "Two-argument TraceLog construction seeded policy-state history.");
}

void test_three_argument_construction() {
  // Preserve the complete prior aggregate shape for any explicit state-carrying
  // construction while removing partial-aggregate warnings from two-argument sites:
  // https://eel.is/c++draft/dcl.init.aggr
  // https://github.com/FlareZ123/pokemon-sims/issues/2177
  const sim::TraceLog trace{true, {}, {{"route", "fingerprint"}}};
  const auto found = trace.last_policy_state_by_key.find("route");
  expect(found != trace.last_policy_state_by_key.end() &&
             found->second == "fingerprint",
         "Three-argument TraceLog construction lost policy-state history.");
}

static_assert(!std::is_aggregate_v<sim::TraceLog>,
              "TraceLog must use its warning-free constructor path.");

}  // namespace

int main() {
  test_default_construction();
  test_existing_two_argument_construction();
  test_three_argument_construction();
  return 0;
}
