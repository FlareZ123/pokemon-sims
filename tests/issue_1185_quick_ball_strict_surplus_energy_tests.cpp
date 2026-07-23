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
  static Card fss_target(const Engine& engine) {
    return engine.fss_target_after_search_started();
  }
  static bool seed23_fss_route(const Engine& engine) {
    return engine.fss_should_take_grass_for_seed23_latias_burnet_route();
  }
  static bool seed23_latias_route(const Engine& engine) {
    return engine.quick_ball_seed23_latias_route_ready();
  }
  static std::optional<Card> latias_tate_cost(const Engine& engine) {
    return engine.quick_ball_latias_replaced_tate_cost();
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

sim::State seed23_fss_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 2},
                 sim::Pokemon{sim::Card::RegidragoV, 3, 0, 0,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::TateLiza, sim::Card::QuickBall, sim::Card::Crispin,
                sim::Card::RegidragoVstar, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::LatiasEx, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::RegidragoV};
  state.discard = {sim::Card::StevensResolve};
  return state;
}

sim::State seed23_quick_ball_state() {
  sim::State state = seed23_fss_state();
  state.vstar_power_used = true;
  state.supporter_used = true;
  state.manual_energy_used = true;
  state.bench.back().grass = 2;
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Crispin));
  state.hand.push_back(sim::Card::Fire);
  state.discard.push_back(sim::Card::Crispin);
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::Grass));
  return state;
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

void test_seed23_route_admission_controls() {
  const sim::Scenario strict{"issue-1403", sim::DciProfile::StrictJit,
                             sim::LockMode::None, true, 5};
  std::mt19937_64 rng{140300};
  sim::Engine fss = make_engine(strict, rng, seed23_fss_state());

  // Direct Grass and route-replaced Tate & Liza are admitted only while every
  // observable Energy, evolution, Active-position, and Burnet payload axis survives:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1403
  expect(sim::EngineTestAccess::seed23_fss_route(fss),
         "The exact public K1 route must be admitted.");
  expect(sim::EngineTestAccess::fss_target(fss) == sim::Card::Grass,
         "The complete K1 route must search Grass.");

  sim::State resolving_state = seed23_fss_state();
  resolving_state.vstar_power_used = true;
  std::mt19937_64 resolving_rng{140306};
  sim::Engine resolving =
      make_engine(strict, resolving_rng, std::move(resolving_state));
  // The Star Alchemy resolver marks the VSTAR Power used before selecting its
  // searched card, so the same legal route must remain admitted at resolution:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/issues/1403
  expect(sim::EngineTestAccess::seed23_fss_route(resolving),
         "The route must survive Star Alchemy's used-power resolver state.");
  expect(sim::EngineTestAccess::fss_target(resolving) == sim::Card::Grass,
         "Star Alchemy resolution must keep the Grass target.");

  sim::State one_grass = seed23_fss_state();
  one_grass.deck.erase(std::find(one_grass.deck.begin(), one_grass.deck.end(),
                                 sim::Card::Grass));
  std::mt19937_64 one_grass_rng{140301};
  sim::Engine blocked_fss = make_engine(strict, one_grass_rng, std::move(one_grass));
  expect(!sim::EngineTestAccess::seed23_fss_route(blocked_fss),
         "Crispin must retain a second searchable Grass.");

  sim::State no_latias = seed23_fss_state();
  no_latias.deck.erase(std::find(no_latias.deck.begin(), no_latias.deck.end(),
                                 sim::Card::LatiasEx));
  std::mt19937_64 no_latias_rng{140302};
  sim::Engine blocked_latias = make_engine(strict, no_latias_rng, std::move(no_latias));
  expect(!sim::EngineTestAccess::seed23_fss_route(blocked_latias),
         "The route must require searchable Latias ex.");

  std::mt19937_64 qb_rng{140303};
  sim::Engine qb = make_engine(strict, qb_rng, seed23_quick_ball_state());
  expect(sim::EngineTestAccess::seed23_latias_route(qb),
         "The exact public Latias route must be admitted.");
  expect(sim::EngineTestAccess::latias_tate_cost(qb) == sim::Card::TateLiza,
         "Latias must replace Tate & Liza's switch role.");

  sim::State no_burnet = seed23_quick_ball_state();
  no_burnet.hand.erase(std::find(no_burnet.hand.begin(), no_burnet.hand.end(),
                                 sim::Card::ProfessorBurnet));
  std::mt19937_64 no_burnet_rng{140304};
  sim::Engine blocked_burnet = make_engine(strict, no_burnet_rng, std::move(no_burnet));
  expect(!sim::EngineTestAccess::latias_tate_cost(blocked_burnet),
         "Tate & Liza stays protected without Burnet.");

  const sim::Scenario rulebox{"issue-1403-rulebox", sim::DciProfile::StrictJit,
                              sim::LockMode::FullRuleBoxAbility, true, 5};
  std::mt19937_64 lock_rng{140305};
  sim::Engine blocked_lock = make_engine(rulebox, lock_rng, seed23_quick_ball_state());
  expect(!sim::EngineTestAccess::latias_tate_cost(blocked_lock),
         "Rule Box Ability lock must block the Latias route.");
}

void test_seed_23_records_t4_route() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-first scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{23};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Exact source-bound route and policy evidence:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1403
  expect(outcome.first_ready_turn == 4, "Seed 23 must reach T4 readiness.");
  expect(!outcome.setup_failed, "T4 readiness must count as setup success.");
  expect(trace_contains(trace, "T3 | STAR ALCHEMY", "Grass Energy"),
         "Star Alchemy must search Grass.");
  expect(trace_contains(trace, "T3 | DISCARD", "Tate & Liza (Quick Ball cost)"),
         "Quick Ball must spend route-replaced Tate & Liza.");
  expect(trace_contains(trace, "T3 | PLAY ITEM", "Latias ex"),
         "Quick Ball must search Latias ex.");
  expect(trace_contains(trace, "T4 | RETREAT", "R-LATIAS-01"),
         "Skyliner must promote the evolved Regidrago line on T4.");
  expect(trace_contains(trace, "T4 | PLAY SUPPORTER", "R-BURNET-01"),
         "Burnet must establish the T4 payload.");
  expect(trace_contains(trace, "T4 | READY"), "Seed 23 must emit T4 readiness.");
  expect(!trace_contains(trace, "T4 | PLAY SUPPORTER", "R-CRISPIN-01"),
         "Duplicate Crispin must not consume the T4 Supporter action.");
}
}  // namespace

int main() {
  try {
    test_strict_jit_route_and_controls();
    test_seed23_route_admission_controls();
    test_seed_23_records_t4_route();
    std::cout << "Issue 1185 strict-JIT surplus Energy tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
