#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static State& state(Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) {
    return engine.outcome_;
  }
  static bool recover_grant(Engine& engine) {
    return engine.recover_grant_for_strict_jit_payload();
  }
  static void record_ready(Engine& engine) { engine.record_ready(); }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

struct Fixture {
  sim::Scenario scenario{"issue-1790/exact", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe{sim::pineco_recipe()};
  std::mt19937_64 rng{1790};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{scenario, recipe, rng, &trace};
};

sim::State grant_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Dragapult, sim::Card::WishfulBaton};
  state.discard = {sim::Card::Grant};
  return state;
}

void test_grant_completes_the_only_missing_strict_jit_axis() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, grant_route_state());

  // Grant may discard two non-Grant cards to return itself from discard without
  // using the Supporter action. Dragapult ex is the current-turn Dragon payload,
  // while Wishful Baton is low-DCI in this opponent-free setup model:
  // Grant: https://api.pokemontcg.io/v2/cards/swsh10-144
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Wishful Baton: https://api.pokemontcg.io/v2/cards/sm3-128
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Trainer and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Strict-JIT and UDP/DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1790
  // Fix PR: https://github.com/FlareZ123/pokemon-sims/pull/1793
  if (!sim::EngineTestAccess::recover_grant(fixture.engine)) {
    throw std::runtime_error("The legal Grant strict-JIT route was rejected.");
  }

  sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::Grant) ||
      contains(after.discard, sim::Card::Grant) ||
      !contains(after.discard, sim::Card::Dragapult) ||
      !contains(after.discard, sim::Card::WishfulBaton) ||
      !contains(after.discarded_this_turn, sim::Card::Dragapult) ||
      after.supporter_used) {
    throw std::runtime_error("Grant recovery changed the wrong zones or consumed the Supporter action.");
  }

  sim::EngineTestAccess::record_ready(fixture.engine);
  if (sim::EngineTestAccess::outcome(fixture.engine).first_ready_turn != 2) {
    throw std::runtime_error("Grant's current-turn Dragon discard did not produce T2 readiness.");
  }
}

void test_grant_rejects_insufficient_or_protected_costs() {
  Fixture insufficient;
  sim::State state = grant_route_state();
  state.hand = {sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(insufficient.engine, state);
  if (sim::EngineTestAccess::recover_grant(insufficient.engine)) {
    throw std::runtime_error("Grant recovered with fewer than two legal costs.");
  }

  Fixture protected_cost;
  state = grant_route_state();
  state.hand = {sim::Card::Dragapult, sim::Card::ForestSealStone};
  sim::EngineTestAccess::set_state(protected_cost.engine, state);

  // The unused singleton Forest Seal Stone retains discrete setup value and must
  // remain UDP rather than becoming Grant's ordinary second cost:
  // Grant: https://api.pokemontcg.io/v2/cards/swsh10-144
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // UDP/DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1790
  // Fix PR: https://github.com/FlareZ123/pokemon-sims/pull/1793
  if (sim::EngineTestAccess::recover_grant(protected_cost.engine)) {
    throw std::runtime_error("Grant spent a protected Forest Seal Stone.");
  }
}

void test_grant_requires_the_card_and_an_incomplete_payload_axis() {
  Fixture absent;
  sim::State state = grant_route_state();
  state.discard.clear();
  sim::EngineTestAccess::set_state(absent.engine, state);
  if (sim::EngineTestAccess::recover_grant(absent.engine)) {
    throw std::runtime_error("Grant recovered while absent from discard.");
  }

  Fixture payload_complete;
  state = grant_route_state();
  state.discard.push_back(sim::Card::MegaDragonite);
  state.discarded_this_turn.push_back(sim::Card::MegaDragonite);
  sim::EngineTestAccess::set_state(payload_complete.engine, state);
  if (sim::EngineTestAccess::recover_grant(payload_complete.engine)) {
    throw std::runtime_error("Grant spent two cards after the strict-JIT payload was complete.");
  }

  Fixture energy_incomplete;
  state = grant_route_state();
  state.active->fire = 0;
  sim::EngineTestAccess::set_state(energy_incomplete.engine, state);
  if (sim::EngineTestAccess::recover_grant(energy_incomplete.engine)) {
    throw std::runtime_error("Grant banked a strict-JIT payload before GGF was complete.");
  }
}

}  // namespace

int main() {
  test_grant_completes_the_only_missing_strict_jit_axis();
  test_grant_rejects_insufficient_or_protected_costs();
  test_grant_requires_the_card_and_an_incomplete_payload_axis();
}
