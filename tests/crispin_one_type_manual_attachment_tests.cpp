#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static void set_prizes_revealed(Engine& engine) { engine.prizes_revealed_ = true; }
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

void test_public_k0_one_type_crispin_is_held_after_manual_attachment() {
  // Crispin can attach only when two different Basic Energy types are selected;
  // with one searchable type, it puts that Energy into hand and attaches none:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // After the one manual Energy attachment is already used, that one-type line
  // cannot advance GGF this turn: https://www.pokemon.com/us/pokemon-tcg/rules
  sim::Scenario scenario{"crispin-public-k0-one-type-manual-used", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Crispin};
  state.deck = {sim::Card::Fire};
  // K0 may not inspect hidden zones, but all six Grass copies are public through
  // the Active attachments plus discard, so no Grass can still be unseen.
  state.discard = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::crispin_can_advance_energy_axis(engine),
         "public K0 copy counts must hold a one-type Crispin line after manual attachment");
  expect(!sim::EngineTestAccess::play_crispin(engine),
         "Crispin must not spend the Supporter play when it can only put Fire into hand");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::Crispin), "held Crispin must remain in hand at K0");
  expect(!after.supporter_used, "the Supporter slot must remain unused at K0");
  expect(contains(after.deck, sim::Card::Fire), "the Fire Energy must remain hidden in deck at K0");
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

void test_one_type_crispin_is_held_after_heavy_ball_prize_reveal() {
  // Heavy Ball reveals all Prize cards, which establishes K1 in this fixed-list
  // model. With only Fire remaining, Crispin can put Fire into hand but cannot
  // attach it without a second Basic Energy type; the manual attachment was
  // already used: https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/sv7-133
  sim::Scenario scenario{"crispin-one-type-heavy-ball-k1", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{65};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Crispin};
  state.deck = {sim::Card::Fire};
  state.prizes = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_prizes_revealed(engine);

  expect(!sim::EngineTestAccess::crispin_can_advance_energy_axis(engine),
         "Prize-reveal K1 must apply the one-type Crispin guard after manual attachment");
  expect(!sim::EngineTestAccess::play_crispin(engine),
         "Crispin must remain held when Heavy Ball knowledge proves it cannot attach Energy");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::Crispin), "held Crispin must remain in hand after Prize reveal");
  expect(!after.supporter_used, "the Supporter slot must remain unused after the known dead Crispin line");
  expect(contains(after.deck, sim::Card::Fire), "the known Fire Energy must remain in deck after Prize reveal");
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
    test_public_k0_one_type_crispin_is_held_after_manual_attachment();
    test_one_type_crispin_is_held_after_manual_attachment();
    test_one_type_crispin_is_held_after_heavy_ball_prize_reveal();
    test_one_type_crispin_remains_live_before_manual_attachment();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
