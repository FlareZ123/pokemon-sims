#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static std::optional<Card> final_surplus_cost(const Engine& engine) {
    return engine.quick_ball_final_surplus_energy_cost();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string_view first,
                    const std::string_view second = {}) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [first, second](const std::string& line) {
                       return line.find(first) != std::string::npos &&
                           (second.empty() || line.find(second) != std::string::npos);
                     });
}

sim::State strict_burnet_route_state() {
  sim::State state;
  state.turn = 5;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::QuickBall, sim::Card::Fire,
                sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::LatiasEx, sim::Card::MegaDragonite,
                sim::Card::Dragapult};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_strict_jit_route_and_controls() {
  const sim::Scenario strict{"issue-1185", sim::DciProfile::StrictJit,
                             sim::LockMode::None, true, 5};
  std::mt19937_64 rng{118500};
  sim::Engine engine = make_engine(strict, rng, strict_burnet_route_state());

  // A selected Regidrago VSTAR with GGF has no remaining Energy axis. Quick Ball
  // may therefore use the final Fire only when Latias ex plus held Burnet complete
  // the observable strict-JIT Active and payload axes on this turn:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, Bench, Ability, Supporter, and retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Dynamic DCI and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1185
  expect(sim::EngineTestAccess::final_surplus_cost(engine) == sim::Card::Fire,
         "Strict JIT must admit the route-conditioned final surplus Fire.");

  const auto blocked = [&](sim::State state, const sim::Scenario& scenario,
                           const std::uint64_t seed, const char* message) {
    std::mt19937_64 local_rng{seed};
    sim::Engine blocked_engine = make_engine(scenario, local_rng, std::move(state));
    expect(!sim::EngineTestAccess::final_surplus_cost(blocked_engine), message);
  };

  sim::State missing_energy = strict_burnet_route_state();
  missing_energy.bench.front().fire = 0;
  blocked(missing_energy, strict, 118501,
          "Energy must remain protected until the selected VSTAR has GGF.");

  sim::State no_burnet = strict_burnet_route_state();
  no_burnet.hand.erase(std::find(no_burnet.hand.begin(), no_burnet.hand.end(),
                                 sim::Card::ProfessorBurnet));
  blocked(no_burnet, strict, 118502,
          "Strict JIT must require a current-turn payload route.");

  sim::State no_payload = strict_burnet_route_state();
  no_payload.deck.erase(std::remove_if(no_payload.deck.begin(), no_payload.deck.end(),
                                       sim::is_payload),
                        no_payload.deck.end());
  blocked(no_payload, strict, 118503,
          "The Burnet route must require a searchable permitted payload.");

  sim::State no_latias = strict_burnet_route_state();
  no_latias.deck.erase(std::find(no_latias.deck.begin(), no_latias.deck.end(),
                                 sim::Card::LatiasEx));
  blocked(no_latias, strict, 118504,
          "The route must require a searchable Latias ex.");

  sim::State full_bench = strict_burnet_route_state();
  while (full_bench.bench.size() < 5) {
    full_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  }
  blocked(full_bench, strict, 118505,
          "The route must require an open Bench slot.");

  sim::State retreat_used = strict_burnet_route_state();
  retreat_used.retreat_used = true;
  blocked(retreat_used, strict, 118506,
          "The route must preserve the one-retreat action.");

  const sim::Scenario item_lock{"issue-1185-item-lock",
                                sim::DciProfile::StrictJit,
                                sim::LockMode::FullItem, true, 5};
  blocked(strict_burnet_route_state(), item_lock, 118507,
          "Item lock must block Quick Ball.");

  const sim::Scenario rulebox_lock{"issue-1185-rulebox-lock",
                                   sim::DciProfile::StrictJit,
                                   sim::LockMode::FullRuleBoxAbility, true, 5};
  blocked(strict_burnet_route_state(), rulebox_lock, 118508,
          "Rule Box Ability lock must block Latias ex Skyliner.");

  const sim::Scenario no_control{"issue-1185-no-control",
                                 sim::DciProfile::NoDiscardControl,
                                 sim::LockMode::None, true, 5};
  blocked(strict_burnet_route_state(), no_control, 118509,
          "The specialized JIT surplus fallback must remain JIT-only.");
}

void test_seed_23_records_t5_recovery() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-first scenario");

  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{23};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The exact deterministic seed must use Quick Ball, Latias ex, Professor Burnet,
  // and retreat to record T5 readiness while retaining the T5 setup-loss result:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/T5_FAILURE_POLICY.md
  // https://github.com/FlareZ123/pokemon-sims/issues/1185
  expect(outcome.first_ready_turn == 5,
         "Seed 23 must record the deterministic T5 recovery.");
  expect(outcome.setup_failed,
         "T5 readiness must retain the repository setup-loss classification.");
  expect(trace_contains(trace, "T5 | DISCARD", "Fire Energy"),
         "Seed 23 must spend the final surplus Fire on Quick Ball.");
  expect(trace_contains(trace, "T5 | PLAY ITEM", "Latias ex"),
         "Seed 23 must search Latias ex with Quick Ball.");
  expect(trace_contains(trace, "T5 | PLAY SUPPORTER", "R-BURNET-01"),
         "Seed 23 must use Professor Burnet during the ready turn.");
  expect(trace_contains(trace, "T5 | RETREAT", "R-LATIAS-01"),
         "Seed 23 must retreat the Basic Active through Skyliner.");
  expect(trace_contains(trace, "T5 | READY"),
         "Seed 23 must emit T5 readiness.");
  expect(!trace_contains(trace, "T5 | PLAY SUPPORTER", "Tate & Liza draw mode"),
         "Blind Tate & Liza draw mode must not preempt the deterministic route.");
}
}  // namespace

int main() {
  try {
    test_strict_jit_route_and_controls();
    test_seed_23_records_t5_recovery();
    std::cout << "Issue 1185 strict-JIT surplus Energy tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
