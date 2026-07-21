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
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool use_celestial_roar(Engine& engine) {
    return engine.use_celestial_roar();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(), [&needle](const std::string& line) {
    return line.find(needle) != std::string::npos;
  });
}

sim::Scenario strict_first(const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{"issue-1174", sim::DciProfile::StrictJit, locks, true, 5};
}

sim::State guaranteed_next_window_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 2, 2, 0};
  state.hand = {sim::Card::Fire, sim::Card::Gladion,
                sim::Card::GoodraVstar, sim::Card::Lusamine};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Fire,
                sim::Card::Grass, sim::Card::Channeler};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::QuickBall,
                  sim::Card::Grass, sim::Card::Arven,
                  sim::Card::FieldBlower, sim::Card::TapuLeleGX};
  state.manual_energy_used = true;
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, sim::TraceLog* trace = nullptr,
                        const bool deck_seen = true,
                        const bool prizes_revealed = true) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen, prizes_revealed);
  return engine;
}

void test_guaranteed_gladion_treasure_route_holds_attack() {
  std::mt19937_64 rng{117401};
  const sim::Scenario scenario = strict_first();
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine = make_engine(scenario, rng, guaranteed_next_window_state(), &trace);

  // Held Fire completes GGF next turn. Gladion exchanges for the known prized
  // Mysterious Treasure, which discards the held Dragon payload and searches the
  // known deck Regidrago VSTAR. Celestial Roar cannot move the earliest T4 window:
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
  // Core rules: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1174
  expect(!sim::EngineTestAccess::use_celestial_roar(engine),
         "A guaranteed Gladion-Treasure T4 route must hold Celestial Roar.");
  expect(trace_contains(trace, "HOLD ATTACK"),
         "The trace must record the route-preserving attack hold.");
  expect(sim::EngineTestAccess::state(engine).deck.size() == 4,
         "Holding must leave the known deck unchanged.");

  std::mt19937_64 locked_rng{117408};
  const sim::Scenario ability_lock{
      "issue-1174-ability-lock", sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility, true, 5};
  sim::Engine ability_locked = make_engine(
      ability_lock, locked_rng, guaranteed_next_window_state());
  expect(!sim::EngineTestAccess::use_celestial_roar(ability_locked),
         "Rule Box Ability lock must not suppress the Item-Supporter route.");
}

void test_missing_route_components_leave_attack_live() {
  const auto attacks = [](sim::State state, const sim::Scenario scenario,
                          const bool deck_seen, const bool prizes_revealed,
                          const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state), nullptr,
                                     deck_seen, prizes_revealed);
    expect(sim::EngineTestAccess::use_celestial_roar(engine), message);
  };

  sim::State no_gladion = guaranteed_next_window_state();
  std::erase(no_gladion.hand, sim::Card::Gladion);
  attacks(no_gladion, strict_first(), true, true, 117402,
          "Missing Gladion must leave the Energy-advancing attack live.");

  sim::State unknown_prizes = guaranteed_next_window_state();
  attacks(unknown_prizes, strict_first(), false, false, 117403,
          "Unknown Prize identities cannot prove the Treasure route.");

  sim::State no_treasure = guaranteed_next_window_state();
  std::erase(no_treasure.prizes, sim::Card::MysteriousTreasure);
  no_treasure.prizes.push_back(sim::Card::Serena);
  attacks(no_treasure, strict_first(), true, true, 117404,
          "A known Prize set without Treasure must leave the attack live.");

  sim::State no_payload = guaranteed_next_window_state();
  std::erase(no_payload.hand, sim::Card::GoodraVstar);
  attacks(no_payload, strict_first(), true, true, 117405,
          "Missing Treasure discard payload must leave the attack live.");

  sim::State no_vstar = guaranteed_next_window_state();
  std::erase(no_vstar.deck, sim::Card::RegidragoVstar);
  attacks(no_vstar, strict_first(), true, true, 117406,
          "Known missing VSTAR must leave the attack live.");

  sim::State item_locked = guaranteed_next_window_state();
  attacks(item_locked,
          sim::Scenario{"issue-1174-lock", sim::DciProfile::StrictJit,
                        sim::LockMode::FullItem, true, 5},
          true, true, 117407,
          "A continuing Item lock must leave the Treasure route unavailable.");
}

void test_seed_7_holds_and_keeps_turn_four_ready() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{7};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 4,
         "Seed 7 must preserve the deterministic turn-four ready window.");
  expect(trace_contains(trace, "T3 | HOLD ATTACK"),
         "Seed 7 must hold Celestial Roar on turn three.");
  expect(!trace_contains(trace, "T3 | ATTACK"),
         "Seed 7 must not expose the unresolved VSTAR axis.");
  expect(trace_contains(trace, "T4 | PLAY SUPPORTER") &&
             trace_contains(trace, "Exchanged Gladion for Mysterious Treasure"),
         "Seed 7 must execute the known Prize connector on turn four.");
  expect(trace_contains(trace, "T4 | READY"),
         "Seed 7 must still reach strict-JIT readiness on turn four.");
}
}  // namespace

int main() {
  try {
    test_guaranteed_gladion_treasure_route_holds_attack();
    test_missing_route_components_leave_attack_live();
    test_seed_7_holds_and_keeps_turn_four_ready();
    std::cout << "Issue 1174 Celestial Roar Gladion-Treasure tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
