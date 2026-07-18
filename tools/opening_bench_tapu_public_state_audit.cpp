#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
  static void set_started_regi(Engine& engine) { engine.outcome_.started_regi = true; }
  static void choose_opening_bench(Engine& engine) { engine.choose_opening_bench(); }
  static void begin_turn(Engine& engine, const int turn) { engine.begin_turn(turn); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void record_ready(Engine& engine) { engine.record_ready(); }
  static void resolve_powerglass(Engine& engine) { engine.resolve_powerglass_end_turn(); }
};

}  // namespace sim

namespace {

sim::State conditioned_opening_state() {
  sim::State state;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::Guzma, sim::Card::Gladion, sim::Card::TapuLeleGX,
                sim::Card::Crispin, sim::Card::Grass, sim::Card::MegaDragonite};
  return state;
}

void remove_required(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto found = std::find(cards.begin(), cards.end(), card);
  if (found == cards.end()) {
    throw std::runtime_error("Conditioned opening card is absent from the deck recipe.");
  }
  cards.erase(found);
}

sim::TrialOutcome run_conditioned_policy(const std::uint64_t seed,
                                         const bool use_opening_bench_selector) {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::vector<sim::Card> remaining;
  for (const auto& [card, copies] : recipe) {
    for (int copy = 0; copy < copies; ++copy) remaining.push_back(card);
  }

  for (const sim::Card card : {sim::Card::Guzma, sim::Card::Gladion,
                               sim::Card::RegidragoV, sim::Card::TapuLeleGX,
                               sim::Card::Crispin, sim::Card::Grass,
                               sim::Card::MegaDragonite}) {
    remove_required(remaining, card);
  }

  std::mt19937_64 shuffle_rng{seed};
  std::shuffle(remaining.begin(), remaining.end(), shuffle_rng);
  std::mt19937_64 policy_rng = shuffle_rng;
  const sim::Scenario scenario{"opening-bench-tapu-public-state-audit",
                               sim::DciProfile::StrictJit, sim::LockMode::None,
                               true, 4};
  sim::Engine engine(scenario, recipe, policy_rng);

  sim::State state = conditioned_opening_state();
  state.deck = std::move(remaining);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_started_regi(engine);

  // Setup may place Tapu Lele-GX on the Bench, while Wonder Tag requires playing
  // it from hand during the turn. Compare the two legal public-state actions over
  // matched hidden deck and Prize states rather than selecting from one hidden seed:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/671
  // https://github.com/FlareZ123/pokemon-sims/issues/885
  // https://github.com/FlareZ123/pokemon-sims/issues/932
  if (use_opening_bench_selector) sim::EngineTestAccess::choose_opening_bench(engine);

  sim::State& setup = const_cast<sim::State&>(sim::EngineTestAccess::state(engine));
  for (int prize = 0; prize < 6; ++prize) {
    setup.prizes.push_back(setup.deck.back());
    setup.deck.pop_back();
  }

  for (int turn = 1; turn <= 4; ++turn) {
    sim::EngineTestAccess::begin_turn(engine, turn);
    if (sim::EngineTestAccess::state(engine).turn_ended) break;
    sim::EngineTestAccess::run_turn(engine);
    sim::EngineTestAccess::record_ready(engine);
    if (sim::EngineTestAccess::outcome(engine).first_ready_turn != 0) break;
    sim::EngineTestAccess::resolve_powerglass(engine);
  }
  return sim::EngineTestAccess::outcome(engine);
}

bool ready_by(const sim::TrialOutcome& outcome, const int turn) {
  return outcome.first_ready_turn != 0 && outcome.first_ready_turn <= turn;
}

double percent(const std::uint64_t count, const std::uint64_t trials) {
  return 100.0 * static_cast<double>(count) / static_cast<double>(trials);
}

}  // namespace

int main() {
  constexpr std::uint64_t trials = 100000;
  std::array<std::uint64_t, 3> held_ready{};
  std::array<std::uint64_t, 3> setup_ready{};
  std::uint64_t held_only_t4 = 0;
  std::uint64_t setup_only_t4 = 0;

  for (std::uint64_t seed = 1; seed <= trials; ++seed) {
    const sim::TrialOutcome held = run_conditioned_policy(seed, false);
    const sim::TrialOutcome setup_benched = run_conditioned_policy(seed, true);

    for (int index = 0; index < 3; ++index) {
      const int horizon = index + 2;
      if (ready_by(held, horizon)) ++held_ready[static_cast<std::size_t>(index)];
      if (ready_by(setup_benched, horizon)) ++setup_ready[static_cast<std::size_t>(index)];
    }

    const bool held_t4 = ready_by(held, 4);
    const bool setup_t4 = ready_by(setup_benched, 4);
    if (held_t4 && !setup_t4) ++held_only_t4;
    if (setup_t4 && !held_t4) ++setup_only_t4;
  }

  std::cout << std::fixed << std::setprecision(6);
  std::cout << "route,ready_by_t2_pct,ready_by_t3_pct,ready_by_t4_pct\n";
  std::cout << "held_tapu," << percent(held_ready[0], trials) << ','
            << percent(held_ready[1], trials) << ','
            << percent(held_ready[2], trials) << '\n';
  std::cout << "setup_benched_tapu," << percent(setup_ready[0], trials) << ','
            << percent(setup_ready[1], trials) << ','
            << percent(setup_ready[2], trials) << '\n';
  std::cout << "setup_minus_held_pp,"
            << percent(setup_ready[0], trials) - percent(held_ready[0], trials) << ','
            << percent(setup_ready[1], trials) - percent(held_ready[1], trials) << ','
            << percent(setup_ready[2], trials) - percent(held_ready[2], trials) << '\n';
  std::cout << "paired_held_only_t4," << held_only_t4 << '\n';
  std::cout << "paired_setup_only_t4," << setup_only_t4 << '\n';
  return 0;
}
