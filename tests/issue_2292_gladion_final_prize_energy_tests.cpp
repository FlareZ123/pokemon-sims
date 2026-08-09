#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

// Reclaimed regression for the exact confirmed route:
// https://github.com/FlareZ123/pokemon-sims/issues/2292
namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = false;
  }

  static bool route_available(const Engine& engine, const Card energy) {
    return engine.issue_2292_gladion_final_prize_energy_finish(energy);
  }

  static bool play_route(Engine& engine) {
    return engine.play_issue_2292_gladion_final_prize_energy_finish();
  }

  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
struct Fixture {
  sim::Scenario scenario{"issue-2292/exact", sim::DciProfile::StrictJit,
                         sim::LockMode::None, true, 5};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2292};
  sim::Engine engine{scenario, recipe, rng};
};

sim::State winning_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None};
  state.hand = {sim::Card::Gladion};
  state.deck = {sim::Card::Grass, sim::Card::EarthenVessel, sim::Card::QuickBall};
  state.prizes = {sim::Card::Grass, sim::Card::TateLiza, sim::Card::Crispin,
                  sim::Card::RegidragoVstar, sim::Card::DialgaGX, sim::Card::RegidragoV};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  return state;
}

void test_known_prized_grass_is_live_even_with_grass_in_deck() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, winning_state());
  // Gladion can exchange for the K1-known Prize Grass; Mega Dragonite ex already
  // entered discard this turn, and the unused manual attachment completes GGF.
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Prize, Supporter, and attachment rules: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 / strict-JIT / earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2292
  if (!sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 known Prize Grass finish was rejected.");
  }
  if (!sim::EngineTestAccess::play_route(fixture.engine)) {
    throw std::runtime_error("#2292 known Prize Grass route did not resolve.");
  }
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  if (std::find(state.hand.begin(), state.hand.end(), sim::Card::Grass) ==
      state.hand.end()) {
    throw std::runtime_error("#2292 did not put the known Prize Grass into hand.");
  }
  if (std::find(state.prizes.begin(), state.prizes.end(), sim::Card::Gladion) ==
      state.prizes.end()) {
    throw std::runtime_error("#2292 did not shuffle Gladion into the remaining Prizes.");
  }
}

void test_route_requires_k1_current_turn_payload_and_unused_actions() {
  Fixture fixture;
  sim::State state = winning_state();
  sim::EngineTestAccess::set_state(fixture.engine, state, false);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route used Prize identity while K0.");
  }
  state = winning_state();
  state.discarded_this_turn.clear();
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route accepted without a current-turn JIT payload.");
  }
  state = winning_state();
  state.supporter_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route accepted after Supporter use.");
  }
  state = winning_state();
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route accepted after manual attachment use.");
  }
}

void test_route_requires_exact_missing_prized_energy() {
  Fixture fixture;
  sim::State state = winning_state();
  state.active->grass = 2;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route accepted Grass when only Fire was missing.");
  }
  state = winning_state();
  state.prizes[0] = sim::Card::Fire;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route accepted a non-prized Energy.");
  }
  state = winning_state();
  state.hand.push_back(sim::Card::Grass);
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  if (sim::EngineTestAccess::route_available(fixture.engine, sim::Card::Grass)) {
    throw std::runtime_error("#2292 route spent Gladion when the final Energy was held.");
  }
}
}  // namespace

int main() {
  test_known_prized_grass_is_live_even_with_grass_in_deck();
  test_route_requires_k1_current_turn_payload_and_unused_actions();
  test_route_requires_exact_missing_prized_energy();
}
