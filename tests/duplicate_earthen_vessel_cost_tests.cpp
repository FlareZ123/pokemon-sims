#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_earthen_vessel(Engine& engine, const bool permit_payload) {
    return engine.play_earthen_vessel(permit_payload);
  }
  static bool play_tate_draw(Engine& engine) { return engine.play_tate_draw(); }
  static bool has_live_one_discard_hand_payload_line(const Engine& engine) {
    return engine.has_live_one_discard_hand_payload_line();
  }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_duplicate_earthen_vessel_is_legal_cost() {
  const sim::Scenario scenario{"duplicate-earthen-vessel-cost", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{86};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::EarthenVessel, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  // Earthen Vessel requires discarding another card from hand before searching
  // Basic Energy, so the other copy is a legal cost: https://api.pokemontcg.io/v2/cards/sv4-163
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_earthen_vessel(engine, false),
         "Earthen Vessel should use the other copy as its discard cost");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::Grass) && contains(after.hand, sim::Card::Fire),
         "Earthen Vessel should search both needed Basic Energy cards");
  expect(count(after.discard, sim::Card::EarthenVessel) == 2,
         "the played Earthen Vessel and its duplicate cost should both be discarded");
  expect(after.deck.empty(), "the searched Basic Energy cards should leave the deck");
}

void test_earthen_vessel_can_discard_unpayable_ultra_ball() {
  const sim::Scenario scenario{"earthen-vessel-ultra-ball-cost", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{87};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1};
  state.hand = {sim::Card::EarthenVessel, sim::Card::UltraBall};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  // Earthen Vessel needs one other hand card. Ultra Ball needs two other cards only
  // when Ultra Ball itself is played, so it remains a legal Vessel cost here:
  // https://api.pokemontcg.io/v2/cards/sv4-163 https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::play_earthen_vessel(engine, false),
         "Earthen Vessel should use held Ultra Ball as its discard cost");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::Grass) && contains(after.hand, sim::Card::Fire),
         "Earthen Vessel should search both needed Basic Energy cards");
  expect(count(after.discard, sim::Card::EarthenVessel) == 1,
         "the played Earthen Vessel should be discarded");
  expect(count(after.discard, sim::Card::UltraBall) == 1,
         "the held Ultra Ball should be discarded as the cost");
  expect(after.deck.empty(), "the searched Basic Energy cards should leave the deck");
}

void test_earthen_vessel_holds_when_k1_proves_no_needed_energy() {
  const sim::Scenario scenario{"earthen-vessel-known-empty-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{199};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::EarthenVessel, sim::Card::Dipplin};
  state.deck = {sim::Card::Arven};
  // Earthen Vessel pays its discard before searching up to two Basic Energy. K1
  // may use the inspected deck contents to avoid a search known to have no target:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  expect(!sim::EngineTestAccess::play_earthen_vessel(engine, true),
         "Earthen Vessel should be held when K1 proves no needed Basic Energy remains");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::EarthenVessel) && contains(after.hand, sim::Card::Dipplin),
         "The dead search must preserve Earthen Vessel and its would-be discard cost");
  expect(after.discard.empty(), "The dead search must not spend either hand card");
}

void test_earthen_vessel_holds_with_empty_deck_even_for_payload_cost() {
  const sim::Scenario scenario{"earthen-vessel-payload-empty-deck", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{200};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EarthenVessel, sim::Card::MegaDragonite};
  // Earthen Vessel's discard is a play cost. With a publicly empty deck, its search
  // effect is known to do nothing, so the Trainer cannot be played and no cost is paid:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
  // Preserve the strict-JIT Dragon instead of inventing a payload-only exception:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  expect(!sim::EngineTestAccess::play_earthen_vessel(engine, true),
         "Earthen Vessel must remain held when its required deck search cannot begin");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::EarthenVessel) &&
             contains(after.hand, sim::Card::MegaDragonite),
         "The illegal empty-deck play must preserve the Item and Dragon payload");
  expect(after.discard.empty() && after.discarded_this_turn.empty(),
         "The empty-deck rejection must not pay Earthen Vessel's discard cost");
  expect(!sim::EngineTestAccess::payload_ready(engine),
         "A preserved in-hand Dragon must not count as a current-turn discard payload");
}

void test_earthen_vessel_holds_when_k1_proves_no_basic_energy_for_payload_cost() {
  const sim::Scenario scenario{"earthen-vessel-payload-known-no-target",
                               sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{552};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EarthenVessel, sim::Card::MegaDragonite};
  state.deck = {sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // K1 proves the nonempty deck has no Basic Energy. Earthen Vessel therefore cannot
  // pay its discard requirement merely to put the Dragon payload into discard:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  expect(!sim::EngineTestAccess::play_earthen_vessel(engine, true),
         "Earthen Vessel must hold when K1 proves that no Basic Energy target exists");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(contains(after.hand, sim::Card::EarthenVessel) &&
             contains(after.hand, sim::Card::MegaDragonite),
         "The known-zero-target play must preserve the Item and Dragon payload");
  expect(after.discard.empty() && after.discarded_this_turn.empty(),
         "The rejected play must not pay Earthen Vessel's discard requirement");
  expect(!sim::EngineTestAccess::payload_ready(engine),
         "The preserved Dragon must not satisfy strict-JIT readiness");
}

void test_tate_draw_does_not_preserve_dead_earthen_vessel_payload_route() {
  const sim::Scenario scenario{"tate-dead-earthen-vessel-route",
                               sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{553};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::TateLiza, sim::Card::EarthenVessel,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // The planning predicate must apply the same target-aware legality as the direct
  // Item play. A known non-Energy deck cannot make Vessel a post-switch payload outlet:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  expect(!sim::EngineTestAccess::has_live_one_discard_hand_payload_line(engine),
         "Tate planning must reject a known-zero-target Earthen Vessel route");
  expect(sim::EngineTestAccess::play_tate_draw(engine),
         "Tate draw should remain available when the promised Vessel route is dead");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used, "Tate & Liza draw mode should consume the Supporter slot");
  expect(contains(after.discard, sim::Card::TateLiza),
         "Tate & Liza should enter discard after its draw mode resolves");
  expect(after.discarded_this_turn.empty(),
         "The dead Vessel plan must not invent a current-turn payload discard");
}

}  // namespace

int main() {
  try {
    test_duplicate_earthen_vessel_is_legal_cost();
    test_earthen_vessel_can_discard_unpayable_ultra_ball();
    test_earthen_vessel_holds_when_k1_proves_no_needed_energy();
    test_earthen_vessel_holds_with_empty_deck_even_for_payload_cost();
    test_earthen_vessel_holds_when_k1_proves_no_basic_energy_for_payload_cost();
    test_tate_draw_does_not_preserve_dead_earthen_vessel_payload_route();
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
