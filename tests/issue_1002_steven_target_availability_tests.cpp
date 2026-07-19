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
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }

  static bool play_steven(Engine& engine) {
    return engine.play_steven();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine() {
  static const sim::Scenario scenario{"issue-1002", sim::DciProfile::NoDiscardControl,
                                       sim::LockMode::None, false, 5};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng{1002};
  return sim::Engine(scenario, recipe, rng);
}

sim::State known_prize_latias_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0};
  state.hand = {sim::Card::StevensResolve,
                sim::Card::RegidragoVstar,
                sim::Card::RegidragoVstar,
                sim::Card::RegidragoVstar,
                sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::RegidragoV,
                sim::Card::Crispin,
                sim::Card::Crispin,
                sim::Card::Gladion,
                sim::Card::Gladion,
                sim::Card::Grass,
                sim::Card::Grass,
                sim::Card::Fire,
                sim::Card::MegaDragonite};
  state.prizes = {sim::Card::LatiasEx,
                  sim::Card::HisuianHeavyBall,
                  sim::Card::QuickBall,
                  sim::Card::Arven,
                  sim::Card::Grass,
                  sim::Card::Fire};
  return state;
}

void test_unavailable_vstar_does_not_displace_gladion_bridge() {
  sim::Engine engine = make_engine();
  sim::EngineTestAccess::set_state(engine, known_prize_latias_state());

  // All Regidrago VSTAR copies are already public in hand. Steven must select three
  // cards that remain in deck, and K1 proves Gladion is the bridge to prized Latias:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/1002
  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven must resolve in the confirmed K1 bridge state.");
  const sim::State& state = engine.state();
  expect(std::count(state.hand.begin(), state.hand.end(), sim::Card::RegidragoV) == 1,
         "Steven must find the missing Basic Regidrago V.");
  expect(std::count(state.hand.begin(), state.hand.end(), sim::Card::Crispin) == 1,
         "Steven must select only one same-turn Crispin.");
  expect(std::count(state.hand.begin(), state.hand.end(), sim::Card::Gladion) == 1,
         "Steven must retain the known-Prize Latias bridge.");
  expect(state.turn_ended && state.supporter_used,
         "Steven must end the turn after the corrected three-card search.");
}

void test_no_prized_latias_does_not_invent_gladion_target() {
  sim::Engine engine = make_engine();
  sim::State state = known_prize_latias_state();
  std::replace(state.prizes.begin(), state.prizes.end(), sim::Card::LatiasEx,
               sim::Card::ErikasInvitation);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven must still resolve without the Prize bridge condition.");
  expect(std::count(engine.state().hand.begin(), engine.state().hand.end(),
                    sim::Card::Gladion) == 0,
         "The correction must not invent a Gladion target without a known prized Latias.");
}

void test_seed_455_searches_gladion_and_reaches_t4() {
  const sim::Scenario scenario{"no-discard-control/go-second",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{455};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Exact confirmed reproduction and expected corrected bridge:
  // https://github.com/FlareZ123/pokemon-sims/issues/1002
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv8-76
  expect(std::any_of(trace.lines.begin(), trace.lines.end(),
                     [](const std::string& line) {
                       return line.find("Regidrago V, Crispin, Gladion") !=
                              std::string::npos;
                     }),
         "Seed 455 must search the unique Gladion bridge instead of a second Crispin.");
  expect(outcome.ready_by_4,
         "The corrected seed-455 line must promote the setup and reach readiness by T4.");
}
}  // namespace

int main() {
  try {
    test_unavailable_vstar_does_not_displace_gladion_bridge();
    test_no_prized_latias_does_not_invent_gladion_target();
    test_seed_455_searches_gladion_and_reaches_t4();
    std::cout << "Issue 1002 Steven target availability tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
