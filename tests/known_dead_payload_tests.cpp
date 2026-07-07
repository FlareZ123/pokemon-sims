#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
  static bool play_evolution_incense(Engine& engine, bool permit_payload) {
    return engine.play_evolution_incense(permit_payload);
  }
  static bool play_mysterious_treasure(Engine& engine, bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static bool run_search_items_one_step(Engine& engine, bool permit_payload) {
    return engine.run_search_items_one_step(permit_payload);
  }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario{"known-dead-payload", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const std::uint64_t seed) : rng(seed), engine(scenario, recipe, rng) {}
};

sim::State known_no_payload_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.deck = {sim::Card::TapuLeleGX};
  state.prizes = {sim::Card::MegaDragonite, sim::Card::Dragapult, sim::Card::GoodraVstar,
                  sim::Card::DialgaGX, sim::Card::Dipplin, sim::Card::ForestSealStone};
  return state;
}

void test_blender_yields_to_live_hand_discard_after_k1() {
  Fixture fixture{199};
  sim::State state = known_no_payload_state();
  state.hand = {sim::Card::HisuianHeavyBall, sim::Card::BrilliantBlender,
                sim::Card::MysteriousTreasure, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Hisuian Heavy Ball reveals the Prize cards, establishing K1 knowledge:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  if (!sim::EngineTestAccess::play_heavy_ball(fixture.engine)) {
    throw std::runtime_error("Heavy Ball should establish K1 before the payload-outlet choice.");
  }
  if (!sim::EngineTestAccess::run_search_items_one_step(fixture.engine, true)) {
    throw std::runtime_error("Mysterious Treasure should remain a live hand-discard payload route.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  // Brilliant Blender discards selected cards from deck, so a K1-proven empty
  // payload deck makes Blender a dead route: https://api.pokemontcg.io/v2/cards/sv8-164
  if (!contains(after.hand, sim::Card::BrilliantBlender) ||
      !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error("K1 should hold Blender and use the live Mysterious Treasure hand-discard route.");
  }
}

void test_dead_outlet_does_not_make_payload_search_live_after_k1() {
  Fixture fixture{201};
  sim::State state = known_no_payload_state();
  state.hand = {sim::Card::HisuianHeavyBall, sim::Card::MysteriousTreasure,
                sim::Card::QuickBall, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (!sim::EngineTestAccess::play_heavy_ball(fixture.engine)) {
    throw std::runtime_error("Heavy Ball should establish K1 before evaluating a payload connector.");
  }
  // Mysterious Treasure can only turn into a payload setup route when a payload
  // can still be found in deck: https://api.pokemontcg.io/v2/cards/sm6-113
  if (sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, false)) {
    throw std::runtime_error("A dead Quick Ball outlet must not justify spending Mysterious Treasure after K1.");
  }
  if (!contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("The known-dead payload connector should remain in hand.");
  }
}

void test_burnet_yields_supporter_slot_after_k1() {
  Fixture fixture{200};
  sim::State state = known_no_payload_state();
  state.hand = {sim::Card::HisuianHeavyBall, sim::Card::ProfessorBurnet,
                sim::Card::Serena, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  if (!sim::EngineTestAccess::play_heavy_ball(fixture.engine)) {
    throw std::runtime_error("Heavy Ball should establish K1 before Supporter selection.");
  }
  sim::EngineTestAccess::choose_supporter(fixture.engine);

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  // Professor Burnet can only discard cards it searches from deck:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Serena can discard the in-hand payload for its draw mode:
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  if (!contains(after.hand, sim::Card::ProfessorBurnet) ||
      !contains(after.discard, sim::Card::Serena) ||
      !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error("K1 should hold dead Burnet and select Serena's live hand-discard route.");
  }
}

void test_evolution_incense_fetches_jit_payload_for_mysterious_treasure() {
  Fixture fixture{202};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
  state.discard = {sim::Card::ProfessorBurnet};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Evolution Incense can search the Stage 2 Mega Dragonite ex payload:
  // https://api.pokemontcg.io/v2/cards/swsh1-163 https://api.pokemontcg.io/v2/cards/me2pt5-152
  if (!sim::EngineTestAccess::play_evolution_incense(fixture.engine, true) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Evolution Incense should fetch Mega Dragonite ex for a live strict-JIT discard route.");
  }

  // Mysterious Treasure may discard that hand payload during the same turn:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  if (!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, true)) {
    throw std::runtime_error("Mysterious Treasure should discard the Evolution Incense payload this turn.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::MegaDragonite) ||
      !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Evolution Incense's fetched payload must remain in this turn's discard.");
  }
}

}  // namespace

int main() {
  test_blender_yields_to_live_hand_discard_after_k1();
  test_dead_outlet_does_not_make_payload_search_live_after_k1();
  test_burnet_yields_supporter_slot_after_k1();
  test_evolution_incense_fetches_jit_payload_for_mysterious_treasure();
  return 0;
}