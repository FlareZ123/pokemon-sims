#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool t1_state(const Engine& engine) {
    return engine.issue_1235_t1_quick_ball_connector_state();
  }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(false); }
  static void play_basics(Engine& engine) { engine.play_basics_from_hand(); }
  static void attach_manual(Engine& engine) { engine.attach_manual(); }
  static bool t2_completion(const Engine& engine) {
    return engine.issue_1235_t2_treasure_tapu_crispin_completion_available();
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static bool ready(const Engine& engine) {
    return engine.active_is_vstar() && engine.state_.active->grass >= 2 &&
        engine.state_.active->fire >= 1 && engine.payload_ready();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count_card(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::Scenario strict_second(const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{"issue-1235", sim::DciProfile::StrictJit, locks, false, 5};
}

sim::State t1_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0}};
  state.hand = {sim::Card::QuickBall, sim::Card::RegidragoVstar,
                sim::Card::RegidragoV, sim::Card::DialgaGX,
                sim::Card::MysteriousTreasure, sim::Card::ChaoticSwell};
  state.deck = {sim::Card::Oricorio, sim::Card::TapuLeleGX,
                sim::Card::Crispin, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Fire, sim::Card::Arven};
  return state;
}

sim::State t2_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0},
                 sim::Pokemon{sim::Card::Oricorio, 1}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::DialgaGX,
                sim::Card::RegidragoVstar, sim::Card::Fire,
                sim::Card::Gladion};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoV, sim::Card::Arven};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = false,
                        sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void test_t1_quick_ball_selects_oricorio_after_inspection() {
  std::mt19937_64 rng{123501};
  const sim::Scenario scenario = strict_second();
  sim::Engine engine = make_engine(scenario, rng, t1_state());

  // Quick Ball's legal deck inspection proves that Oricorio supplies the T1 Grass
  // attachment while leaving the T2 Treasure -> Tapu Lele-GX -> Crispin chain live.
  // This complete graph reaches T2, one turn before Tapu -> Steven's Resolve:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Official Item, Ability, Supporter, Bench, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1235
  expect(sim::EngineTestAccess::t1_state(engine),
         "The exact opening connector state must be recognized.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball must execute the cost-aware connector comparison.");
  const sim::State& searched = sim::EngineTestAccess::state(engine);
  expect(count_card(searched.discard, sim::Card::RegidragoV) == 1,
         "The redundant Regidrago V must pay Quick Ball.");
  expect(count_card(searched.hand, sim::Card::Oricorio) == 1,
         "Quick Ball must search Oricorio when the full T2 route is K1-proven.");
  expect(count_card(searched.hand, sim::Card::TapuLeleGX) == 0,
         "Tapu Lele-GX must remain for the T2 Treasure search.");
  sim::EngineTestAccess::play_basics(engine);
  sim::EngineTestAccess::attach_manual(engine);
  expect(sim::EngineTestAccess::state(engine).active->grass == 1,
         "Vital Dance plus the manual attachment must establish the T1 Grass.");
}

void test_t2_holds_gladion_and_finishes() {
  std::mt19937_64 rng{123502};
  sim::TraceLog trace;
  trace.enabled = true;
  const sim::Scenario scenario = strict_second();
  sim::Engine engine = make_engine(scenario, rng, t2_state(), true, &trace);
  expect(sim::EngineTestAccess::t2_completion(engine),
         "The Active-Regidrago Treasure-Tapu-Crispin route must project T2 readiness.");
  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(sim::EngineTestAccess::ready(engine),
         "The complete route must produce an Active GGF VSTAR and current-turn payload.");
  expect(count_card(after.discard, sim::Card::DialgaGX) == 1,
         "Mysterious Treasure must discard Dialga-GX on T2.");
  expect(count_card(after.discard, sim::Card::Crispin) == 1,
         "Wonder Tag must obtain and play Crispin.");
  expect(count_card(after.hand, sim::Card::Gladion) == 1,
         "The inferior held Supporter must remain unused.");
}

void test_controls_preserve_tapu_priority() {
  const auto blocked = [](sim::State state, const sim::Scenario scenario,
                          const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state));
    expect(!sim::EngineTestAccess::t1_state(engine), message);
  };

  sim::State no_treasure = t1_state();
  no_treasure.hand.erase(std::find(no_treasure.hand.begin(), no_treasure.hand.end(),
                                  sim::Card::MysteriousTreasure));
  blocked(no_treasure, strict_second(), 123503, "Missing Treasure must block Oricorio priority.");

  sim::State no_payload = t1_state();
  no_payload.hand.erase(std::find(no_payload.hand.begin(), no_payload.hand.end(), sim::Card::DialgaGX));
  blocked(no_payload, strict_second(), 123504, "Missing payload must block the T2 continuation.");

  blocked(t1_state(), strict_second(sim::LockMode::FullRuleBoxAbility), 123505,
          "Rule Box Ability lock must block the Tapu continuation.");

  sim::State one_open_slot = t1_state();
  one_open_slot.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 0});
  one_open_slot.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 0});
  one_open_slot.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 0});
  blocked(one_open_slot, strict_second(), 123506,
          "Fewer than two open Bench slots must block the two-connector route.");
}
}  // namespace

int main() {
  try {
    test_t1_quick_ball_selects_oricorio_after_inspection();
    test_t2_holds_gladion_and_finishes();
    test_controls_preserve_tapu_priority();
    std::cout << "issue 1235 tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
