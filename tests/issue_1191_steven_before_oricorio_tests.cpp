#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static bool route_available(const Engine& engine) {
    return engine.known_t2_steven_crispin_latias_route_available();
  }
  static bool choose_supporter(Engine& engine) {
    const bool used_before = engine.state_.supporter_used;
    engine.choose_supporter();
    return !used_before && engine.state_.supporter_used;
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 0, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::StevensResolve, sim::Card::Gladion};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::Oricorio};
  state.discard = {sim::Card::Dragapult};
  return state;
}

const sim::Scenario& scenario() {
  static const sim::Scenario value{"issue-1191", sim::DciProfile::NoDiscardControl,
                                   sim::LockMode::None, false, 5};
  return value;
}

sim::Engine make_engine(std::mt19937_64& rng, sim::TraceLog* trace,
                        sim::State state = route_state()) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario(), recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_complete_route_preempts_gladion() {
  std::mt19937_64 rng{119100};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine = make_engine(rng, &trace);

  expect(sim::EngineTestAccess::route_available(engine),
         "The complete K1 Steven package must be recognized.");
  expect(sim::EngineTestAccess::choose_supporter(engine),
         "Steven's Resolve must consume the T1 Supporter play.");
  expect(engine.state().supporter_used,
         "The selected Supporter must be marked used.");
  expect(std::count(engine.state().hand.begin(), engine.state().hand.end(),
                    sim::Card::Gladion) == 1,
         "Gladion must remain in hand.");
  expect(std::count(engine.state().hand.begin(), engine.state().hand.end(),
                    sim::Card::RegidragoVstar) == 1 &&
             std::count(engine.state().hand.begin(), engine.state().hand.end(),
                        sim::Card::Crispin) == 1 &&
             std::count(engine.state().hand.begin(), engine.state().hand.end(),
                        sim::Card::LatiasEx) == 1,
         "Steven must search VSTAR, Crispin, and Latias ex.");
  expect(std::none_of(trace.lines.begin(), trace.lines.end(),
                      [](const std::string& line) {
                        return line.find("exchanged Gladion for Oricorio") !=
                            std::string::npos;
                      }),
         "Gladion must not recover prized Oricorio.");
}

void test_route_controls() {
  const auto blocked = [](sim::State state, const std::uint64_t seed,
                          const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(rng, nullptr, std::move(state));
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  sim::State missing_vstar = route_state();
  missing_vstar.deck.erase(missing_vstar.deck.begin());
  blocked(std::move(missing_vstar), 119110,
          "A known searchable VSTAR must be required.");

  sim::State missing_crispin = route_state();
  missing_crispin.deck.erase(missing_crispin.deck.begin() + 1);
  blocked(std::move(missing_crispin), 119111,
          "A known searchable Crispin must be required.");

  sim::State missing_latias = route_state();
  missing_latias.deck.erase(missing_latias.deck.begin() + 2);
  blocked(std::move(missing_latias), 119112,
          "A known searchable Latias ex must be required.");

  sim::State missing_fire = route_state();
  missing_fire.deck.pop_back();
  blocked(std::move(missing_fire), 119113,
          "Crispin must have both Basic Energy types.");

  sim::State no_payload = route_state();
  no_payload.discard.clear();
  blocked(std::move(no_payload), 119114,
          "The no-discard-control payload must already be banked.");

  sim::State no_bench = route_state();
  while (no_bench.bench.size() < 5) {
    no_bench.bench.push_back(
        sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None});
  }
  blocked(std::move(no_bench), 119115,
          "Latias ex must have an open Bench slot.");

  const sim::Scenario ability_lock{
      "issue-1191-ability-lock", sim::DciProfile::NoDiscardControl,
      sim::LockMode::FullRuleBoxAbility, false, 5};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 ability_lock_rng{119117};
  sim::Engine ability_lock_engine(ability_lock, recipe, ability_lock_rng);
  sim::EngineTestAccess::set_state(ability_lock_engine, route_state());
  // Rule Box Ability lock suppresses Skyliner, so the Basic Active cannot use
  // Latias ex for the deterministic promotion route:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://github.com/FlareZ123/pokemon-sims/issues/1191
  expect(!sim::EngineTestAccess::route_available(ability_lock_engine),
         "Rule Box Ability lock must block the Latias route.");

  sim::State no_grass_start = route_state();
  no_grass_start.bench.front().grass = 0;
  no_grass_start.bench.front().fire = 1;
  blocked(std::move(no_grass_start), 119116,
          "The Regidrago V must already have one Grass Energy.");
}

void test_seed_509_reaches_t2() {
  const auto live = sim::scenario_by_label("no-discard-control/go-second");
  if (!live) throw std::runtime_error("Missing no-discard-control scenario");
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{509};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*live, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The fixed route uses only cards held or known in deck after Quick Ball's K1
  // inspection. Steven gets VSTAR, Crispin, and Latias; T2 Crispin plus the
  // manual attachment reaches GGF, evolution resolves, and Skyliner promotes:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1191
  expect(outcome.first_ready_turn == 2,
         "Seed 509 must reach no-discard-control readiness on T2.");
  expect(std::none_of(trace.lines.begin(), trace.lines.end(),
                      [](const std::string& line) {
                        return line.find("exchanged Gladion for Oricorio") !=
                            std::string::npos;
                      }),
         "Seed 509 must preserve Gladion instead of taking Oricorio.");
}
}  // namespace

int main() {
  test_complete_route_preempts_gladion();
  test_route_controls();
  test_seed_509_reaches_t2();
  return 0;
}
