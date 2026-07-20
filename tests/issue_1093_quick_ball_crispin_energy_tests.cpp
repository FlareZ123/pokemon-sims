#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(false); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count_card(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(), [&needle](const std::string& line) {
    return line.find(needle) != std::string::npos;
  });
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {sim::Card::QuickBall, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Crispin, sim::Card::Arven,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::ForestSealStone, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_replaced_grass_pays_quick_ball() {
  const sim::Scenario scenario{"issue-1093-positive", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{109301};
  sim::Engine engine = make_engine(scenario, rng, route_state());

  // Quick Ball may spend one Grass because Crispin immediately restores that exact
  // Energy role while the held Arven, Forest Seal Stone, VSTAR, Blender, and Latias
  // graph deterministically completes the next-turn route:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1093
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Crispin-replaced Grass must pay Quick Ball for the missing Regidrago V.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count_card(after.discard, sim::Card::Grass) == 1,
         "Exactly one Grass must pay Quick Ball.");
  expect(count_card(after.hand, sim::Card::Grass) == 1,
         "One held Grass must remain for the turn-one manual attachment.");
  expect(count_card(after.hand, sim::Card::RegidragoV) == 1,
         "Quick Ball must search Regidrago V.");
}

void test_route_prerequisites_protect_energy() {
  const sim::Scenario scenario{"issue-1093-negative", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  const auto blocked = [&](const sim::State& state, const std::uint64_t seed,
                           const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, state);
    expect(!sim::EngineTestAccess::play_quick_ball(engine), message);
  };

  sim::State no_crispin = route_state();
  no_crispin.hand.erase(std::find(no_crispin.hand.begin(), no_crispin.hand.end(), sim::Card::Crispin));
  blocked(no_crispin, 109302, "Energy must remain protected without held Crispin.");

  sim::State one_grass = route_state();
  one_grass.hand.erase(std::find(one_grass.hand.begin(), one_grass.hand.end(), sim::Card::Grass));
  blocked(one_grass, 109303, "The only held Grass must remain protected.");

  sim::State no_fire = route_state();
  no_fire.deck.erase(std::find(no_fire.deck.begin(), no_fire.deck.end(), sim::Card::Fire));
  blocked(no_fire, 109304, "The route must require both searchable Basic Energy types.");

  sim::State supporter_spent = route_state();
  supporter_spent.supporter_used = true;
  blocked(supporter_spent, 109305, "A spent Supporter slot must block the Crispin projection.");

  sim::State attachment_spent = route_state();
  attachment_spent.manual_energy_used = true;
  blocked(attachment_spent, 109306, "A spent manual attachment must block the route.");

  sim::State no_arven = route_state();
  no_arven.hand.erase(std::find(no_arven.hand.begin(), no_arven.hand.end(), sim::Card::Arven));
  blocked(no_arven, 109307, "The deterministic next-turn VSTAR route must remain required.");

  sim::State no_blender = route_state();
  no_blender.hand.erase(std::find(no_blender.hand.begin(), no_blender.hand.end(), sim::Card::BrilliantBlender));
  blocked(no_blender, 109308, "The strict-JIT payload outlet must remain required.");
}

void test_lower_dci_cost_keeps_priority() {
  const sim::Scenario scenario{"issue-1093-priority", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{109309};
  sim::State state = route_state();
  state.hand.push_back(sim::Card::QuickBall);
  sim::Engine engine = make_engine(scenario, rng, std::move(state));

  // A duplicate Quick Ball is already lower-DCI legal fuel and stays ahead of the
  // route-conditioned Energy fallback: https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/1093
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "The duplicate-Item route must remain payable.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count_card(after.discard, sim::Card::QuickBall) == 2,
         "The played and discarded Quick Ball copies must enter discard.");
  expect(count_card(after.discard, sim::Card::Grass) == 0,
         "Grass must remain protected when lower-DCI fuel exists.");
}

void test_seed_229_reaches_turn_two() {
  const sim::Scenario scenario{"strict-jit/go-second", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{229};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The exact source-bound regression must establish Regidrago V and GG on turn one,
  // then use Arven, Forest Seal Stone, VSTAR, Fire, Blender, and Skyliner to reach the
  // earliest demonstrated ready state on turn two:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/issues/1093
  expect(outcome.first_ready_turn == 2, "Seed 229 must become ready on turn two.");
  expect(trace_contains(trace, "T1 | DISCARD | rules: R-QB-01 | Grass Energy"),
         "Seed 229 must spend the Crispin-replaced Grass with Quick Ball.");
  expect(trace_contains(trace, "T1 | PLAY SUPPORTER | rules: R-CRISPIN-01"),
         "Seed 229 must play Crispin on turn one.");
  expect(trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-ARVEN-01"),
         "Seed 229 must play Arven on turn two.");
  expect(trace_contains(trace, "T2 | READY |"),
         "Seed 229 must record turn-two readiness.");
}
}  // namespace

int main() {
  try {
    test_replaced_grass_pays_quick_ball();
    test_route_prerequisites_protect_energy();
    test_lower_dci_cost_keeps_priority();
    test_seed_229_reaches_turn_two();
    std::cout << "Issue 1093 Quick Ball Crispin-Energy tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
