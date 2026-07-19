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

  static bool steven_candidate(const Engine& engine) {
    return engine.steven_zero_energy_latias_vessel_candidate();
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

void test_exact_k1_state_admits_complete_three_card_route() {
  const sim::Scenario scenario{"issue-1009-positive", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{1009};
  sim::TraceLog trace;
  sim::Engine engine = make_engine(scenario, trace, rng);
  sim::EngineTestAccess::set_k1_state(engine, seed_17_pre_steven_state());

  // K1 proves that these three unrestricted targets deterministically complete T3
  // from the zero-Energy Regidrago, held VSTAR, held Dragon, and Basic Active:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1009
  expect(sim::EngineTestAccess::steven_candidate(engine),
         "The exact K1 state must admit the complete Latias-Vessel route.");
}

void test_item_lock_rejects_following_turn_vessel_route() {
  const sim::Scenario scenario{"issue-1009-item-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, false, 5};
  std::mt19937_64 rng{1010};
  sim::TraceLog trace;
  sim::Engine engine = make_engine(scenario, trace, rng);
  sim::EngineTestAccess::set_k1_state(engine, seed_17_pre_steven_state());

  // Earthen Vessel cannot serve as the T3 connector after modeled Item lock begins:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1009
  expect(!sim::EngineTestAccess::steven_candidate(engine),
         "The candidate must reject a scheduled Item lock.");
}

void test_rule_box_lock_rejects_latias_route() {
  const sim::Scenario scenario{"issue-1009-rulebox-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 5};
  std::mt19937_64 rng{1011};
  sim::TraceLog trace;
  sim::Engine engine = make_engine(scenario, trace, rng);
  sim::EngineTestAccess::set_k1_state(engine, seed_17_pre_steven_state());

  // Latias ex is a Rule Box Pokémon, so Skyliner is unavailable under this lock:
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://github.com/FlareZ123/pokemon-sims/issues/1009
  expect(!sim::EngineTestAccess::steven_candidate(engine),
         "The candidate must reject a locked Skyliner route.");
}

void test_missing_payload_rejects_vessel_compression() {
  const sim::Scenario scenario{"issue-1009-no-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{1012};
  sim::TraceLog trace;
  sim::Engine engine = make_engine(scenario, trace, rng);
  sim::State state = seed_17_pre_steven_state();
  std::replace(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite,
               sim::Card::Arven);
  sim::EngineTestAccess::set_k1_state(engine, std::move(state));

  // Vessel compresses strict JIT only when its cost can discard a held Dragon during
  // the ready turn: https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1009
  expect(!sim::EngineTestAccess::steven_candidate(engine),
         "The candidate must require a held strict-JIT Dragon payload.");
}

void test_insufficient_energy_inventory_rejects_draw_fragile_route() {
  const sim::Scenario scenario{"issue-1009-energy-inventory", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{1013};
  sim::TraceLog trace;
  sim::Engine engine = make_engine(scenario, trace, rng);
  sim::State state = seed_17_pre_steven_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Grass),
                   state.deck.end());
  state.deck.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_k1_state(engine, std::move(state));

  // Crispin plus the two-turn Vessel route requires at least two Grass copies in the
  // inspected deck before Steven resolves: https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/issues/1009
  expect(!sim::EngineTestAccess::steven_candidate(engine),
         "The candidate must reject an insufficient Grass inventory.");
}

void test_seed_17_uses_vessel_on_t3_and_preserves_supporter() {
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
  expect(!trace_contains(trace, "T2 | DISCARD | rules: R-EV-01"),
         "Seed 17 must preserve Vessel and the held Dragon through T2.");
  expect(trace_contains(trace,
                        "T3 | DISCARD | rules: R-EV-01 | Mega Dragonite ex (Earthen Vessel cost)"),
         "Seed 17 must discard the held Dragon with Vessel on T3.");
  expect(!trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-BURNET-01"),
         "The compressed T3 route must preserve the Supporter slot.");
  expect(trace_contains(trace, "T3 | READY"),
         "Seed 17 must reach strict-JIT readiness on T3.");
  expect(outcome.ready_by_3 && outcome.first_ready_turn == 3,
         "Seed 17 must be ready by exactly T3.");
}
}  // namespace

int main() {
  try {
    test_exact_k1_state_admits_complete_three_card_route();
    test_item_lock_rejects_following_turn_vessel_route();
    test_rule_box_lock_rejects_latias_route();
    test_missing_payload_rejects_vessel_compression();
    test_insufficient_energy_inventory_rejects_draw_fragile_route();
    test_seed_17_uses_vessel_on_t3_and_preserves_supporter();
    std::cout << "Issue 1009 Steven zero-Energy Latias-Vessel tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
