#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_opening_bench(Engine& engine) { engine.choose_opening_bench(); }
};
}  // namespace sim

namespace {
bool benched(const sim::State& state, const sim::Card card) {
  return std::any_of(state.bench.begin(), state.bench.end(), [card](const sim::Pokemon& pokemon) {
    return pokemon.card == card;
  });
}

void test_dense_regidrago_opening_benches_latias() {
  const sim::Scenario scenario{"opening-latias-dense-regi", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{65701};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::State state;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::LatiasEx, sim::Card::Grass, sim::Card::StevensResolve,
                sim::Card::Dragapult, sim::Card::Fire, sim::Card::BrilliantBlender};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Setup allows Latias ex onto the Bench. The dense public graph already contains
  // Steven, Blender, both Energy types, and a Dragon payload, so removing Latias from
  // hand improves the current earliest-ready policy without using hidden information:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/657
  sim::EngineTestAccess::choose_opening_bench(engine);
  if (!benched(sim::EngineTestAccess::state(engine), sim::Card::LatiasEx)) {
    throw std::runtime_error("The dense Regidrago opening should setup-Bench Latias ex.");
  }
}

void test_generic_regidrago_opening_preserves_latias() {
  const sim::Scenario scenario{"opening-latias-generic-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{65702};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::State state;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_bench(engine);
  if (benched(sim::EngineTestAccess::state(engine), sim::Card::LatiasEx)) {
    throw std::runtime_error("A generic Regidrago opening should preserve Latias ex in hand.");
  }
}

void test_rule_box_lock_rejects_latias_bench_route() {
  const sim::Scenario scenario{"opening-latias-rule-box-control", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, false, 4};
  std::mt19937_64 rng{65703};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::State state;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::LatiasEx, sim::Card::TateLiza};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_bench(engine);
  if (benched(sim::EngineTestAccess::state(engine), sim::Card::LatiasEx)) {
    throw std::runtime_error("Rule Box Ability lock should reject the Latias bench route.");
  }
}
}  // namespace

int main() {
  test_dense_regidrago_opening_benches_latias();
  test_generic_regidrago_opening_preserves_latias();
  test_rule_box_lock_rejects_latias_bench_route();
  return 0;
}
