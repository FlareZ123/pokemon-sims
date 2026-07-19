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
  static void set_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
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

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State seed_17_pre_steven_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1},
                 sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::Channeler,
                sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite,
                sim::Card::ProfessorBurnet,
                sim::Card::StevensResolve};
  state.discard = {sim::Card::QuickBall, sim::Card::RegidragoVstar};
  state.prizes = {sim::Card::Powerglass,
                  sim::Card::Lusamine,
                  sim::Card::ChaoticSwell,
                  sim::Card::Grass,
                  sim::Card::Arven,
                  sim::Card::QuickBall};
  state.deck = {sim::Card::RegidragoV,
                sim::Card::RegidragoV,
                sim::Card::RegidragoV,
                sim::Card::RegidragoVstar,
                sim::Card::Crispin,
                sim::Card::Crispin,
                sim::Card::LatiasEx,
                sim::Card::EarthenVessel,
                sim::Card::EarthenVessel,
                sim::Card::Grass,
                sim::Card::Grass,
                sim::Card::Grass,
                sim::Card::Grass,
                sim::Card::Grass,
                sim::Card::Fire,
                sim::Card::Fire,
                sim::Card::Fire,
                sim::Card::Dragapult,
                sim::Card::GoodraVstar,
                sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure,
                sim::Card::MysteriousTreasure,
                sim::Card::BrilliantBlender,
                sim::Card::HisuianHeavyBall,
                sim::Card::TateLiza,
                sim::Card::Gladion,
                sim::Card::Gladion};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, sim::TraceLog& trace,
                        std::mt19937_64& rng) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return sim::Engine(scenario, recipe, rng, &trace);
}

void test_exact_k1_state_selects_complete_three_card_route() {
  const sim::Scenario scenario{"issue-1009-positive", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{1009};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine = make_engine(scenario, trace, rng);
  sim::EngineTestAccess::set_k1_state(engine, seed_17_pre_steven_state());

  const int regi_before = static_cast<int>(std::count(
      engine.state().hand.begin(), engine.state().hand.end(), sim::Card::RegidragoV));
  const int vstar_before = static_cast<int>(std::count(
      engine.state().hand.begin(), engine.state().hand.end(), sim::Card::RegidragoVstar));
  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven must resolve in the exact K1 route state.");

  // These three unrestricted targets deterministically complete T3 from the public
  // zero-Energy Regidrago, held VSTAR, held Dragon payload, and Basic Active:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1009
  expect(trace_contains(trace, "Crispin, Latias ex, Earthen Vessel"),
         "Steven must select the complete Latias-Vessel route.");
  expect(std::count(engine.state().hand.begin(), engine.state().hand.end(),
                    sim::Card::Crispin) == 1,
         "Steven must select one Crispin.");
  expect(std::count(engine.state().hand.begin(), engine.state().hand.end(),
                    sim::Card::LatiasEx) == 1,
         "Steven must select Latias ex for Active-position compression.");
  expect(std::count(engine.state().hand.begin(), engine.state().hand.end(),
                    sim::Card::EarthenVessel) == 1,
         "Steven must select Earthen Vessel for final Energy plus payload discard.");
  expect(std::count(engine.state().hand.begin(), engine.state().hand.end(),
                    sim::Card::RegidragoV) == regi_before,
         "Steven must not add a redundant Regidrago V.");
  expect(std::count(engine.state().hand.begin(), engine.state().hand.end(),
                    sim::Card::RegidragoVstar) == vstar_before,
         "Steven must not add a redundant Regidrago VSTAR.");
}

void test_item_lock_rejects_following_turn_vessel_route() {
  const sim::Scenario scenario{"issue-1009-item-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, false, 5};
  std::mt19937_64 rng{1010};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine = make_engine(scenario, trace, rng);
  sim::EngineTestAccess::set_k1_state(engine, seed_17_pre_steven_state());

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven must still resolve under scheduled Item lock.");
  // Earthen Vessel is an Item and cannot serve as the T3 connector after turn-two
  // Item lock begins: https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1009
  expect(!trace_contains(trace, "Crispin, Latias ex, Earthen Vessel"),
         "The correction must not claim an Item-locked Vessel route.");
}

void test_rule_box_lock_rejects_latias_route() {
  const sim::Scenario scenario{"issue-1009-rulebox-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 5};
  std::mt19937_64 rng{1011};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine = make_engine(scenario, trace, rng);
  sim::EngineTestAccess::set_k1_state(engine, seed_17_pre_steven_state());

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven must still resolve under Rule Box Ability lock.");
  // Latias ex is a Rule Box Pokémon, so Skyliner is unavailable in this modeled lock:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://github.com/FlareZ123/pokemon-sims/issues/1009
  expect(!trace_contains(trace, "Crispin, Latias ex, Earthen Vessel"),
         "The correction must not claim a locked Skyliner route.");
}

void test_missing_payload_rejects_vessel_compression() {
  const sim::Scenario scenario{"issue-1009-no-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{1012};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine = make_engine(scenario, trace, rng);
  sim::State state = seed_17_pre_steven_state();
  std::replace(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite,
               sim::Card::Arven);
  sim::EngineTestAccess::set_k1_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_steven(engine),
         "Steven must still resolve without a held Dragon payload.");
  // Earthen Vessel compresses strict JIT only when its discard cost can put the held
  // Dragon into discard during the ready turn:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1009
  expect(!trace_contains(trace, "Crispin, Latias ex, Earthen Vessel"),
         "The correction must require a held strict-JIT Dragon payload.");
}

void test_seed_17_reaches_deterministic_t3() {
  const sim::Scenario scenario{"strict-jit/go-second", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{17};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Exact merged-main reproduction and complete route:
  // https://github.com/FlareZ123/pokemon-sims/issues/1009
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  expect(trace_contains(trace, "Crispin, Latias ex, Earthen Vessel"),
         "Seed 17 must search the complete route.");
  expect(trace_contains(trace, "Mega Dragonite ex (Earthen Vessel cost)"),
         "Seed 17 must discard the held Dragon with Vessel on T3.");
  expect(trace_contains(trace, "T3 | READY"),
         "Seed 17 must reach strict-JIT readiness on T3.");
  expect(outcome.ready_by_3 && outcome.first_ready_turn == 3,
         "Seed 17 must be ready by exactly T3.");
}
}  // namespace

int main() {
  try {
    test_exact_k1_state_selects_complete_three_card_route();
    test_item_lock_rejects_following_turn_vessel_route();
    test_rule_box_lock_rejects_latias_route();
    test_missing_payload_rejects_vessel_compression();
    test_seed_17_reaches_deterministic_t3();
    std::cout << "Issue 1009 Steven zero-Energy Latias-Vessel tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
