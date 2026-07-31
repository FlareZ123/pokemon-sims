#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_ultra_ball(Engine& engine) {
    return engine.play_ultra_ball(false);
  }
};
}  // namespace sim

namespace {
void test_k0_ultra_ball_takes_only_remaining_crobat_v() {
  const sim::Scenario scenario{"issue-1952-k0-crobat-fallback",
                               sim::DciProfile::NoDiscardControl,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{1952};
  sim::TraceLog trace{true, {}, {}};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng, &trace);

  sim::State state;
  state.turn = 1;
  state.hand = {sim::Card::UltraBall, sim::Card::Grant,
                sim::Card::WishfulBaton};
  state.deck = {sim::Card::CrobatV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball discards two other cards and searches for any Pokemon. Crobat V is
  // a Basic Pokemon V, so the K0-to-K1 exhaustive fallback must take it when it is
  // the only Pokemon remaining after the printed costs are paid:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/swsh3-104
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/1952
  if (!sim::EngineTestAccess::play_ultra_ball(engine)) {
    throw std::runtime_error("The K0 Ultra Ball route should remain playable.");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (std::count(after.hand.begin(), after.hand.end(), sim::Card::CrobatV) != 1 ||
      std::count(after.deck.begin(), after.deck.end(), sim::Card::CrobatV) != 0 ||
      after.discard.size() != 3U) {
    throw std::runtime_error(
        "Ultra Ball did not take the only remaining Crobat V after K1 inspection.");
  }
}
}  // namespace

int main() {
  try {
    test_k0_ultra_ball_takes_only_remaining_crobat_v();
    std::cout << "Issue 1952 Ultra Ball Crobat V fallback tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
