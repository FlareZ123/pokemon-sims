#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1877_treasure_quick_ball_payload_bridge_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::EarthenVessel,
                sim::Card::QuickBall, sim::Card::FieldBlower};
  state.deck = {sim::Card::Dragapult, sim::Card::MegaDragonite,
                sim::Card::RegidragoV, sim::Card::LatiasEx};
  return state;
}

sim::Engine make_engine(const sim::DciProfile dci, const sim::LockMode lock,
                        const int max_turn, const int turn,
                        const bool k1 = true) {
  const sim::Scenario scenario{"issue-3405", dci, lock, false, max_turn};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng{3405};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, route_state(turn), k1);
  return engine;
}

void test_relative_turns_and_horizon() {
  // Items have no printed T3-only timing; these two search Items are legal on any
  // modeled turn where their conditions and the current Item-lock state permit it.
  // Advanced Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md#L382-L404
  // Mysterious Treasure / Quick Ball: https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh1-179
  // Same-ready-turn policy / confirmed regression: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/issues/3405
  for (const int turn : {2, 3, 4}) {
    sim::Engine engine = make_engine(sim::DciProfile::StrictJit,
                                     sim::LockMode::None, 4, turn);
    expect(sim::EngineTestAccess::route_available(engine),
           "StrictJit rejected a legal bridge inside the scenario horizon.");
  }

  sim::Engine flex = make_engine(sim::DciProfile::MatchupFlexJit,
                                 sim::LockMode::None, 4, 2);
  expect(sim::EngineTestAccess::route_available(flex),
         "MatchupFlexJit did not share same-ready-turn bridge timing.");

  sim::Engine expired = make_engine(sim::DciProfile::StrictJit,
                                    sim::LockMode::None, 3, 4);
  expect(!sim::EngineTestAccess::route_available(expired),
         "The bridge was admitted beyond the scenario horizon.");
}

void test_semantic_negative_gates_remain() {
  // K1 and Item legality remain real route requirements.
  // K1: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Item-lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed regression: https://github.com/FlareZ123/pokemon-sims/issues/3405
  sim::Engine no_control = make_engine(sim::DciProfile::NoDiscardControl,
                                       sim::LockMode::None, 4, 2);
  expect(!sim::EngineTestAccess::route_available(no_control),
         "NoDiscardControl entered the same-ready-turn bridge.");

  sim::Engine item_locked = make_engine(sim::DciProfile::StrictJit,
                                        sim::LockMode::FullItem, 4, 2);
  expect(!sim::EngineTestAccess::route_available(item_locked),
         "Item lock admitted the two-Item bridge.");

  sim::Engine k0 = make_engine(sim::DciProfile::StrictJit,
                               sim::LockMode::None, 4, 2, false);
  expect(!sim::EngineTestAccess::route_available(k0),
         "K0 admitted the K1-only bridge.");
}
}  // namespace

int main() {
  test_relative_turns_and_horizon();
  test_semantic_negative_gates_remain();
}