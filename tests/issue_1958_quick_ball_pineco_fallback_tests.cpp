#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
};

}  // namespace sim

namespace {

void test_k0_quick_ball_takes_only_remaining_pineco() {
  const sim::Scenario scenario{"issue-1958-k0-pineco-fallback",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{1958};
  sim::TraceLog trace{true, {}, {}};
  sim::Engine engine(scenario, sim::pineco_recipe(), rng, &trace);

  sim::State state;
  state.turn = 2;
  state.hand = {sim::Card::QuickBall, sim::Card::Grant};
  state.deck = {sim::Card::Pineco};  // Pineco is Basic: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Quick Ball pays one other card, establishes K1, and must take the only Basic:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // Official Item, cost, deck-search, reveal, and shuffle procedure:
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed reproduction: https://github.com/FlareZ123/pokemon-sims/issues/1958
  if (!sim::EngineTestAccess::play_quick_ball(engine)) {
    throw std::runtime_error("The K0 Quick Ball route should remain playable.");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (std::count(after.hand.begin(), after.hand.end(), sim::Card::Pineco) != 1 ||
      std::count(after.deck.begin(), after.deck.end(), sim::Card::Pineco) != 0 ||
      after.discard.size() != 2U) {
    throw std::runtime_error(
        "Quick Ball did not take the only remaining Basic Pineco after K1 inspection.");
  }
}

}  // namespace

int main() {
  try {
    test_k0_quick_ball_takes_only_remaining_pineco();
    std::cout << "Issue 1958 Quick Ball Pineco fallback tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
