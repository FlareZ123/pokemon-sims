#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool delayed_vessel_route(Engine& engine) {
    return engine.legacy_star_delayed_vessel_route();
  }
  static bool use_legacy_star(Engine& engine) {
    return engine.use_legacy_star_issue1016_original();
  }
};

}  // namespace sim

namespace {

int count_card(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

struct Fixture {
  sim::Scenario scenario{"issue-2494", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2494};
  sim::Engine engine{scenario, recipe, rng};
};

void seed_state(sim::Engine& engine, const bool hold_vessel,
                const bool discard_vessel_with_legacy) {
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.manual_energy_used = true;
  state.hand = {sim::Card::MegaDragonite};
  if (hold_vessel) state.hand.push_back(sim::Card::EarthenVessel);

  // Legacy Star removes from the vector back. Grass remains in the inspected deck.
  // Goodra plus six inert cards supply the seven-card discard and keep the pre-use
  // random-payload admission live. Vessel is inserted into that seven only when the
  // control explicitly needs a physical discard copy.
  // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2494
  state.deck = {sim::Card::Grass, sim::Card::ChaoticSwell,
                sim::Card::PathToPeak, sim::Card::ErikasInvitation,
                sim::Card::Guzma, sim::Card::Channeler,
                discard_vessel_with_legacy ? sim::Card::EarthenVessel
                                           : sim::Card::TeamYellsCheer,
                sim::Card::GoodraVstar};
  sim::EngineTestAccess::set_deck_seen(engine);
}

void test_held_only_vessel_does_not_consume_recovery() {
  Fixture fixture;
  seed_state(fixture.engine, true, false);
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);

  // A held Vessel satisfies the deterministic next-turn route, but Legacy Star can
  // select only cards that are actually in discard. The held physical copy must stay
  // in hand without spending either of Legacy Star's recovery choices:
  // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2494
  if (!sim::EngineTestAccess::delayed_vessel_route(fixture.engine)) {
    throw std::runtime_error("Held Vessel should keep the delayed route available.");
  }
  if (!sim::EngineTestAccess::use_legacy_star(fixture.engine)) {
    throw std::runtime_error("Legacy Star should resolve in the held-only fixture.");
  }
  if (count_card(state.hand, sim::Card::EarthenVessel) != 1 ||
      count_card(state.discard, sim::Card::EarthenVessel) != 0) {
    throw std::runtime_error(
        "Held-only Vessel must remain one physical hand copy after Legacy Star.");
  }
}

void test_discard_vessel_is_recovered_when_not_held() {
  Fixture fixture;
  seed_state(fixture.engine, false, true);
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);

  // The established #1844 line still recovers Vessel when Legacy Star physically
  // discards it and no copy is already held:
  // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Existing delayed-route specification: https://github.com/FlareZ123/pokemon-sims/issues/1844
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2494
  if (!sim::EngineTestAccess::use_legacy_star(fixture.engine)) {
    throw std::runtime_error("Legacy Star should resolve in the discard fixture.");
  }
  if (count_card(state.hand, sim::Card::EarthenVessel) != 1 ||
      count_card(state.discard, sim::Card::EarthenVessel) != 0) {
    throw std::runtime_error(
        "A newly discarded Vessel should still be recovered for the delayed route.");
  }
}

void test_held_vessel_preserves_additional_discard_copy() {
  Fixture fixture;
  seed_state(fixture.engine, true, true);
  sim::State& state = sim::EngineTestAccess::state(fixture.engine);

  // One held Vessel already completes the delayed route. Recovering another physical
  // copy would spend a scarce Legacy Star choice without advancing the setup axes, so
  // the discarded copy stays in discard for stronger recovery selection:
  // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earliest-route and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2494
  if (!sim::EngineTestAccess::use_legacy_star(fixture.engine)) {
    throw std::runtime_error("Legacy Star should resolve in the held-plus-discard fixture.");
  }
  if (count_card(state.hand, sim::Card::EarthenVessel) != 1 ||
      count_card(state.discard, sim::Card::EarthenVessel) != 1) {
    throw std::runtime_error(
        "Held Vessel should prevent redundant recovery of the discarded second copy.");
  }
}

}  // namespace

int main() {
  try {
    test_held_only_vessel_does_not_consume_recovery();
    test_discard_vessel_is_recovered_when_not_held();
    test_held_vessel_preserves_additional_discard_copy();
    std::cout << "issue 2494 Legacy Star held Vessel tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
