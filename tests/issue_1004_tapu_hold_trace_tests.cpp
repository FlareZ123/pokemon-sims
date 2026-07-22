#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>

namespace sim {
struct EngineTestAccess {
  static void emit_tapu_hold(Engine& engine, std::string_view key,
                              std::string_view rules, std::string_view detail) {
    engine.trace_tapu_hold_once(key, rules, detail);
  }

  static void add_to_hand(Engine& engine, const Card card) {
    engine.state_.hand.push_back(card);
  }

  static void set_turn(Engine& engine, const int turn) {
    engine.state_.turn = turn;
  }
};
}  // namespace sim

namespace {
constexpr std::string_view kLegacyDetail =
    "Retained the second Tapu because held Crispin, evolution, manual attachment, and Legacy Star already complete setup.";
constexpr std::string_view kGladionDetail =
    "Retained Tapu because the held needed Energy and manual attachment already complete setup.";
constexpr std::string_view kArvenDetail =
    "Retained Tapu because held Energy, Professor Burnet, and Regidrago VSTAR already complete setup.";

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

std::size_t detail_count(const sim::TraceLog& trace, const std::string_view detail) {
  return static_cast<std::size_t>(std::count_if(
      trace.lines.begin(), trace.lines.end(), [detail](const std::string& line) {
        return line.find(detail) != std::string::npos;
      }));
}

sim::TraceLog run_trace(const sim::Scenario& scenario, const std::uint64_t seed) {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{seed};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  engine.run();
  return trace;
}

void test_seed_136_direct_route_emits_no_tapu_hold() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::TraceLog trace = run_trace(scenario, 136);

  // The stronger preflight keeps the first Tapu Lele-GX in hand, so the older
  // unchanged-state hold for a searched second copy is no longer reached:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Prior trace contract: https://github.com/FlareZ123/pokemon-sims/issues/1004
  // Superseding route fix: https://github.com/FlareZ123/pokemon-sims/issues/1016
  expect(detail_count(trace, kLegacyDetail) == 0U,
         "Seed 136 must skip the obsolete second-Tapu hold event.");
}

void test_seed_143_arven_route_emits_once() {
  const sim::Scenario scenario{"matchup-flex-jit/go-second",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 5};
  const sim::TraceLog trace = run_trace(scenario, 143);

  // Tapu Lele-GX and Arven remain unused when held Burnet, VSTAR, and Energy
  // already complete the route:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://github.com/FlareZ123/pokemon-sims/issues/1004
  expect(detail_count(trace, kArvenDetail) == 1U,
         "Seed 143 must emit one unchanged-state Arven Tapu hold.");
}

void test_seed_80_gladion_route_emits_once() {
  const sim::Scenario scenario{"no-discard-control/go-first",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, true, 5};
  const sim::TraceLog trace = run_trace(scenario, 80);

  // Gladion is unnecessary when the exact Prize-target Energy is already held and
  // the manual attachment remains open:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1004
  expect(detail_count(trace, kGladionDetail) == 1U,
         "Seed 80 must emit one unchanged-state Gladion Tapu hold.");
}

void expect_state_change_reemits(std::string_view key, std::string_view rules,
                                  std::string_view detail) {
  const sim::Scenario scenario{"issue-1004", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1004};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  sim::EngineTestAccess::set_turn(engine, 3);

  sim::EngineTestAccess::emit_tapu_hold(engine, key, rules, detail);
  sim::EngineTestAccess::emit_tapu_hold(engine, key, rules, detail);
  expect(detail_count(trace, detail) == 1U,
         "An unchanged Tapu retention state must emit one event.");

  sim::EngineTestAccess::add_to_hand(engine, sim::Card::Grass);
  sim::EngineTestAccess::emit_tapu_hold(engine, key, rules, detail);
  expect(detail_count(trace, detail) == 2U,
         "A changed Tapu retention state must permit a new event.");
}

void test_each_route_reemits_after_state_change() {
  // Exact-state trace behavior follows the readable-hand audit contract:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#run-one-readable-hand
  // https://github.com/FlareZ123/pokemon-sims/issues/1004
  // https://github.com/FlareZ123/pokemon-sims/pull/1014
  expect_state_change_reemits(
      "issue-1004-tapu-legacy-star-route",
      "R-TAPU-01; P-DCI-01; P-KNOWLEDGE-01", kLegacyDetail);
  expect_state_change_reemits(
      "issue-1004-tapu-gladion-route",
      "R-TAPU-01; P-KNOWLEDGE-01; P-DCI-01", kGladionDetail);
  expect_state_change_reemits(
      "issue-1004-tapu-arven-burnet-route",
      "R-TAPU-01; P-DCI-01; P-KNOWLEDGE-01", kArvenDetail);
}
}  // namespace

int main() {
  try {
    test_seed_136_direct_route_emits_no_tapu_hold();
    test_seed_143_arven_route_emits_once();
    test_seed_80_gladion_route_emits_once();
    test_each_route_reemits_after_state_change();
    std::cout << "Issue 1004 Tapu hold trace tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
