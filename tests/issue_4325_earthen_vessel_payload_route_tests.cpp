#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void set_deck_seen(Engine& engine, const bool value) {
    engine.deck_seen_ = value;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_earthen_vessel(Engine& engine, const bool permit_payload) {
    return engine.play_earthen_vessel(permit_payload);
  }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode locks = sim::LockMode::None)
      : scenario{"issue-4325/exact", sim::DciProfile::StrictJit, locks,
                 false, 5},
        recipe(sim::pineco_recipe()),
        rng(4325),
        trace{true, {}},
        engine(scenario, recipe, rng, &trace) {}
};

sim::State equivalent_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::EarthenVessel, sim::Card::RegidragoVstar,
                sim::Card::Dragapult, sim::Card::WishfulBaton};
  state.deck = {sim::Card::Fire, sim::Card::Grass, sim::Card::Grant};
  state.discard = {sim::Card::QuickBall};
  return state;
}

void test_state_relative_completion_without_historical_fingerprint() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, equivalent_route_state());
  sim::EngineTestAccess::set_deck_seen(fixture.engine, true);

  // K1 proves the finishing Fire is in deck, and this legal state has no Secret
  // Box, Dawn, Forretress ex, or Pineco history. Vessel can use the held Dragon as
  // its cost while the searched Fire plus the unused manual attachment completes
  // GGF and the prior-turn Regidrago V can evolve into the held VSTAR this turn:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Item, discard, search, attachment, and evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K0/K1 and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed systemic overfit: https://github.com/FlareZ123/pokemon-sims/issues/4325
  if (!sim::EngineTestAccess::play_earthen_vessel(fixture.engine, false)) {
    throw std::runtime_error(
        "Equivalent state-relative Earthen Vessel route was rejected.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::EarthenVessel) ||
      !contains(after.discard, sim::Card::Dragapult) ||
      !contains(after.hand, sim::Card::WishfulBaton) ||
      !contains(after.hand, sim::Card::Fire)) {
    throw std::runtime_error(
        "Earthen Vessel did not prioritize the K1-proved Dragon payload route.");
  }
}

void test_k0_does_not_force_unproved_payload_completion() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, equivalent_route_state());

  // Before a legal deck or Prize inspection, a fixed-list Fire may still be prized.
  // That K0 possibility is insufficient to prove that Vessel plus the unused manual
  // attachment completes Apex this turn, so the Dragon remains protected and the
  // ordinary low-DCI Wishful Baton pays the otherwise legal Vessel:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed systemic overfit: https://github.com/FlareZ123/pokemon-sims/issues/4325
  if (!sim::EngineTestAccess::play_earthen_vessel(fixture.engine, false)) {
    throw std::runtime_error("Legal K0 Earthen Vessel play was rejected.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::Dragapult) ||
      !contains(after.discard, sim::Card::WishfulBaton)) {
    throw std::runtime_error(
        "K0 incorrectly forced an unproved strict-JIT payload discard.");
  }
}

void test_payload_priority_requires_unused_manual_attachment() {
  Fixture fixture;
  sim::State state = equivalent_route_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine, true);

  // Once the normal manual attachment has already been spent, the searched Fire
  // cannot complete GGF this turn. The strict-JIT Dragon stays protected and the
  // ordinary low-DCI Wishful Baton pays the otherwise legal Vessel instead:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed boundary: https://github.com/FlareZ123/pokemon-sims/issues/4325
  if (!sim::EngineTestAccess::play_earthen_vessel(fixture.engine, false)) {
    throw std::runtime_error("Legal non-JIT Earthen Vessel play was rejected.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::Dragapult) ||
      !contains(after.discard, sim::Card::WishfulBaton)) {
    throw std::runtime_error(
        "Earthen Vessel spent a strict-JIT payload after manual attachment was used.");
  }
}

void test_payload_priority_requires_evolution_eligible_regidrago() {
  Fixture fixture;
  sim::State state = equivalent_route_state();
  state.active->entered_turn = state.turn;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine, true);

  // A Regidrago V played this turn cannot evolve normally this turn, so Vessel
  // must preserve the Dragon payload rather than claiming same-turn readiness:
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed boundary: https://github.com/FlareZ123/pokemon-sims/issues/4325
  if (!sim::EngineTestAccess::play_earthen_vessel(fixture.engine, false)) {
    throw std::runtime_error("Legal fresh-Regidrago Vessel play was rejected.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::Dragapult) ||
      !contains(after.discard, sim::Card::WishfulBaton)) {
    throw std::runtime_error(
        "Earthen Vessel spent a payload before Regidrago V could evolve.");
  }
}

void test_known_missing_basic_preserves_payload() {
  Fixture fixture;
  sim::State state = equivalent_route_state();
  state.deck = {sim::Card::Grant};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine, true);

  // K1 proves that no Basic Energy remains to complete the Energy axis. Vessel has
  // no strategic payload-completion route, so the sole held Dragon stays protected:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed boundary: https://github.com/FlareZ123/pokemon-sims/issues/4325
  const bool played = sim::EngineTestAccess::play_earthen_vessel(fixture.engine, false);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (played || !contains(after.hand, sim::Card::Dragapult)) {
    throw std::runtime_error(
        "Known-empty Basic Energy route consumed or exposed the strict-JIT payload.");
  }
}

void test_item_lock_blocks_vessel() {
  Fixture fixture(sim::LockMode::FullItem);
  sim::EngineTestAccess::set_state(fixture.engine, equivalent_route_state());
  sim::EngineTestAccess::set_deck_seen(fixture.engine, true);

  // Item cards cannot be played while the modeled Item lock is active. The same
  // state-relative route therefore remains unavailable under that scenario:
  // Item procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-interpretation
  // Confirmed boundary: https://github.com/FlareZ123/pokemon-sims/issues/4325
  if (sim::EngineTestAccess::play_earthen_vessel(fixture.engine, false)) {
    throw std::runtime_error("Earthen Vessel resolved through Item lock.");
  }
}

}  // namespace

int main() {
  test_state_relative_completion_without_historical_fingerprint();
  test_k0_does_not_force_unproved_payload_completion();
  test_payload_priority_requires_unused_manual_attachment();
  test_payload_priority_requires_evolution_eligible_regidrago();
  test_known_missing_basic_preserves_payload();
  test_item_lock_blocks_vessel();
  return 0;
}
