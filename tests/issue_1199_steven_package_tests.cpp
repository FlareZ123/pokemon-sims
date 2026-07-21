#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = false;
  }
  static bool route_available(Engine& engine) {
    return engine.issue_1199_route_available();
  }
  static bool deck_seen(const Engine& engine) { return engine.deck_seen_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure,
                sim::Card::Fire, sim::Card::TeamYellsCheer};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Grass, sim::Card::BrilliantBlender,
                sim::Card::MegaDragonite, sim::Card::QuickBall};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

bool trace_has(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
    return line.find(text) != std::string::npos;
  });
}

void test_exact_k0_projection_admits_complete_package() {
  const sim::Scenario scenario{"issue-1199-exact",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 3};
  std::mt19937_64 rng{119900};
  sim::Engine engine = make_engine(scenario, rng, route_state());

  // The K0 decision may project only the legal held Treasure inspection. It admits
  // Steven after that inspection proves VSTAR, two Grass Energy, Blender, and a
  // payload, while the real state remains unmodified until the route is selected:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Hidden-information boundary: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1199
  expect(sim::EngineTestAccess::route_available(engine),
         "The complete source-bounded Steven package was not admitted.");
  expect(!sim::EngineTestAccess::deck_seen(engine),
         "The K0 projection leaked deck knowledge into the real state.");
  expect(engine.state().hand.size() == 4,
         "The K0 projection mutated the real hand.");
}

void test_route_boundaries() {
  const sim::Scenario live{"issue-1199-controls",
                           sim::DciProfile::NoDiscardControl,
                           sim::LockMode::None, false, 3};
  const auto reject = [&](sim::State state, const sim::Scenario& scenario,
                          const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state));
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  sim::State no_fire = route_state();
  no_fire.hand.erase(std::remove(no_fire.hand.begin(), no_fire.hand.end(),
                                 sim::Card::Fire), no_fire.hand.end());
  reject(std::move(no_fire), live, 119901,
         "Missing held Fire must reject the attachment schedule.");

  sim::State one_grass = route_state();
  one_grass.deck.erase(std::find(one_grass.deck.begin(), one_grass.deck.end(),
                                 sim::Card::Grass));
  reject(std::move(one_grass), live, 119902,
         "Fewer than two searchable Grass Energy must reject Steven.");

  sim::State no_blender = route_state();
  no_blender.deck.erase(std::remove(no_blender.deck.begin(), no_blender.deck.end(),
                                    sim::Card::BrilliantBlender), no_blender.deck.end());
  reject(std::move(no_blender), live, 119903,
         "Missing Brilliant Blender must reject the package.");

  sim::State no_payload = route_state();
  no_payload.deck.erase(std::remove(no_payload.deck.begin(), no_payload.deck.end(),
                                    sim::Card::MegaDragonite), no_payload.deck.end());
  reject(std::move(no_payload), live, 119904,
         "Missing deck payload must reject the package.");

  sim::State no_vstar = route_state();
  no_vstar.deck.erase(std::remove(no_vstar.deck.begin(), no_vstar.deck.end(),
                                  sim::Card::RegidragoVstar), no_vstar.deck.end());
  reject(std::move(no_vstar), live, 119905,
         "Missing searchable VSTAR must reject Treasure projection.");

  sim::State same_turn = route_state();
  same_turn.active->entered_turn = same_turn.turn;
  reject(std::move(same_turn), live, 119906,
         "A same-turn Regidrago V must reject the T2 evolution schedule.");

  sim::State held_grass = route_state();
  held_grass.hand.push_back(sim::Card::Grass);
  reject(std::move(held_grass), live, 119907,
         "The narrow two-Grass package must yield to already-held Energy routes.");

  sim::State supporter_used = route_state();
  supporter_used.supporter_used = true;
  reject(std::move(supporter_used), live, 119908,
         "A spent Supporter slot must reject Steven.");

  const sim::Scenario going_first{"issue-1199-first",
                                  sim::DciProfile::NoDiscardControl,
                                  sim::LockMode::None, true, 3};
  reject(route_state(), going_first, 119909,
         "The confirmed going-second route must not affect going first.");

  const sim::Scenario strict{"issue-1199-strict", sim::DciProfile::StrictJit,
                             sim::LockMode::None, false, 3};
  reject(route_state(), strict, 119910,
         "Strict JIT must reject the T2 payload banking schedule.");

  const sim::Scenario item_lock{"issue-1199-item-lock",
                                sim::DciProfile::NoDiscardControl,
                                sim::LockMode::FullItem, false, 3};
  reject(route_state(), item_lock, 119911,
         "Item lock must reject the Treasure and Blender route.");
}

void test_seed_341_uses_steven_before_one_shot_connectors() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-second");
  if (!scenario) throw std::runtime_error("Missing no-discard-control/go-second scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{341};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Seed 341 must establish the deterministic Steven package on T1. A later legal
  // T2 Crispin compression may improve the approved T3 floor, but the former T1
  // Quick Ball, Wonder Tag, Star Alchemy, and Celestial Roar line must be absent:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago V / Celestial Roar: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Earlier complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1199
  expect(outcome.first_ready_turn > 0 && outcome.first_ready_turn <= 3,
         "Seed 341 did not meet the approved deterministic T3 floor.");
  expect(trace_has(trace, "T1 | PLAY SUPPORTER") &&
             trace_has(trace, "Searched the issue-1199 T3 package: Grass Energy, Grass Energy, Brilliant Blender"),
         "Seed 341 did not select the exact Steven package on T1.");
  expect(!trace_has(trace, "T1 | QUICK BALL") &&
             !trace_has(trace, "T1 | WONDER TAG") &&
             !trace_has(trace, "T1 | STAR ALCHEMY") &&
             !trace_has(trace, "T1 | ATTACK |") &&
             !trace_has(trace, "T1 | CELESTIAL ROAR"),
         "Seed 341 spent a prohibited one-shot connector before Steven.");
}

}  // namespace

int main() {
  test_exact_k0_projection_admits_complete_package();
  test_route_boundaries();
  test_seed_341_uses_steven_before_one_shot_connectors();
  return 0;
}
