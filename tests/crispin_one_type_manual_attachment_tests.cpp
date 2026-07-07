#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool crispin_can_advance_energy_axis(const Engine& engine) {
    return engine.crispin_can_advance_energy_axis();
  }
  static bool play_crispin(Engine& engine) { return engine.play_crispin(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_one_type_crispin_is_held_after_manual_attachment() {
  // Crispin puts one searched Basic Energy into hand and attaches the other:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  sim::Scenario scenario{"crispin-one-type-manual-used", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{63};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Crispin};
  state.deck = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  expect(!sim::EngineTestAccess::crispin_can_advance_energy_axis(engine),
         "a known one-type Crispin line cannot advance Energy after the manual attachment is used");
  expect(!sim::EngineTestAccess::play_crispin(engine),
         "Crispin must be held instead of spending the Supporter play for an unattached Energy");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::Crispin), "held Crispin must remain in hand");
  expect(!after.supporter_used, "the Supporter slot must remain unused");
  expect(contains(after.deck, sim::Card::Fire), "the known Fire Energy must remain in deck");
}

void test_one_type_crispin_remains_live_before_manual_attachment() {
  // The single found Energy can be manually attached after Crispin puts it into hand:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  sim::Scenario scenario{"crispin-one-type-manual-open", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{64};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Crispin};
  state.deck = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  expect(sim::EngineTestAccess::crispin_can_advance_energy_axis(engine),
         "a known one-type Crispin line remains useful while the manual attachment is available");
  expect(sim::EngineTestAccess::play_crispin(engine), "Crispin should resolve to put the needed Fire Energy into hand");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::Fire), "the sole searchable Energy must enter hand");
  expect(after.supporter_used, "Crispin must use the Supporter play after a useful one-type resolution");
}

}  // namespace

int main() {
  try {
    test_one_type_crispin_is_held_after_manual_attachment();
    test_one_type_crispin_remains_live_before_manual_attachment();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
