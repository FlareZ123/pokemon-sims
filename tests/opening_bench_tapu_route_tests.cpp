#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static State& state(Engine& engine) { return engine.state_; }
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

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool benched(const sim::State& state, const sim::Card card) {
  return std::any_of(state.bench.begin(), state.bench.end(), [card](const sim::Pokemon& pokemon) {
    return pokemon.card == card;
  });
}

sim::State approved_opening_state() {
  sim::State state;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::Guzma, sim::Card::Gladion, sim::Card::TapuLeleGX,
                sim::Card::Crispin, sim::Card::Grass, sim::Card::MegaDragonite};
  return state;
}

void expect_tapu_placement(const sim::Scenario& scenario, sim::State state,
                           const bool expected, const char* message) {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{671};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_bench(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  const bool placed = benched(after, sim::Card::TapuLeleGX) &&
                      !contains(after.hand, sim::Card::TapuLeleGX);
  if (placed != expected) throw std::runtime_error(message);
}

void test_approved_opening_setup_benches_redundant_tapu() {
  const sim::Scenario scenario{"opening-bench-redundant-tapu", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};

  // Setup-Benching Tapu deliberately gives up Wonder Tag. The approved public hand
  // already holds three Supporters, Crispin for Energy, Gladion for Prize recovery,
  // Basic Energy, and Mega Dragonite ex for the strict-JIT payload axis:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/issues/671
  expect_tapu_placement(scenario, approved_opening_state(), true,
                        "The approved saturated route should place Tapu Lele-GX on the opening Bench.");
}

void remove_required(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto found = std::find(cards.begin(), cards.end(), card);
  if (found == cards.end()) throw std::runtime_error("Conditioned opening card is absent from the deck recipe.");
  cards.erase(found);
}

sim::TrialOutcome run_seed_one_conditioned_policy(const bool use_opening_bench_selector) {
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

  std::mt19937_64 shuffle_rng{1};
  std::shuffle(remaining.begin(), remaining.end(), shuffle_rng);
  std::mt19937_64 policy_rng = shuffle_rng;
  const sim::Scenario scenario{"opening-bench-tapu-seed-one", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  sim::Engine engine(scenario, recipe, policy_rng);

  sim::State state = approved_opening_state();
  state.deck = std::move(remaining);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_started_regi(engine);
  if (use_opening_bench_selector) sim::EngineTestAccess::choose_opening_bench(engine);

  sim::State& setup = sim::EngineTestAccess::state(engine);
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

void test_seed_one_downstream_policy_characterization() {
  // Matched seed 1 holds the shuffled 53-card remainder, six Prize cards, and all
  // downstream choices constant. Holding Tapu reaches T3 through Wonder Tag. After
  // #718 removes the invalid early Blender spend, the setup-Benched route has no live
  // VSTAR-card connector and remains unready through the T4 fixture deadline. Issue
  // #932 owns the distinct opening-selector correction:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/671
  // https://github.com/FlareZ123/pokemon-sims/issues/885
  // https://github.com/FlareZ123/pokemon-sims/issues/718
  // https://github.com/FlareZ123/pokemon-sims/issues/932
  const sim::TrialOutcome held_tapu = run_seed_one_conditioned_policy(false);
  const sim::TrialOutcome setup_benched_tapu = run_seed_one_conditioned_policy(true);
  if (held_tapu.first_ready_turn != 3 || setup_benched_tapu.first_ready_turn != 0) {
    throw std::runtime_error(
        "Matched seed 1 should reach T3 with held Tapu and remain unready through T4 after setup placement.");
  }
}

void test_opening_tapu_bench_controls() {
  const sim::Scenario strict{"opening-bench-tapu-control", sim::DciProfile::StrictJit,
                             sim::LockMode::None, true, 4};

  {
    sim::State state = approved_opening_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Guzma));
    expect_tapu_placement(strict, std::move(state), false,
                          "Two held Supporters should preserve the scarce Wonder Tag connector.");
  }
  {
    sim::State state = approved_opening_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Gladion));
    state.hand.push_back(sim::Card::ChaoticSwell);
    expect_tapu_placement(strict, std::move(state), false,
                          "Without held Prize recovery, Tapu Lele-GX should remain available for Wonder Tag.");
  }
  {
    sim::State state = approved_opening_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Crispin));
    state.hand.push_back(sim::Card::Arven);
    expect_tapu_placement(strict, std::move(state), false,
                          "Without held Crispin, the immediate Energy route should preserve Wonder Tag.");
  }
  {
    sim::State state = approved_opening_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Grass));
    state.hand.push_back(sim::Card::ChaoticSwell);
    expect_tapu_placement(strict, std::move(state), false,
                          "Without held Basic Energy, the public Crispin route is incomplete.");
  }
  {
    sim::State state = approved_opening_state();
    state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::MegaDragonite));
    state.hand.push_back(sim::Card::ChaoticSwell);
    expect_tapu_placement(strict, std::move(state), false,
                          "Without a held Dragon payload, Tapu Lele-GX should remain a live connector.");
  }
  {
    const sim::Scenario locked{"opening-bench-tapu-rulebox-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, true, 4};
    expect_tapu_placement(locked, approved_opening_state(), false,
                          "Rule Box Ability lock should preserve Tapu Lele-GX for a later unlock route.");
  }
  {
    const sim::Scenario item_locked{"opening-bench-tapu-item-lock", sim::DciProfile::StrictJit,
                                    sim::LockMode::FullItem, true, 4};
    expect_tapu_placement(item_locked, approved_opening_state(), false,
                          "Full Item lock should preserve Wonder Tag instead of assuming the unlocked route.");
  }
  {
    const sim::Scenario control{"opening-bench-tapu-no-discard", sim::DciProfile::NoDiscardControl,
                                sim::LockMode::None, true, 4};
    expect_tapu_placement(control, approved_opening_state(), false,
                          "The strict-JIT opening policy should not leak into the no-discard control.");
  }
  {
    sim::State state = approved_opening_state();
    state.bench = {sim::Pokemon{sim::Card::LatiasEx, 0}, sim::Pokemon{sim::Card::Oricorio, 0},
                   sim::Pokemon{sim::Card::MawileGX, 0}, sim::Pokemon{sim::Card::DialgaGX, 0},
                   sim::Pokemon{sim::Card::RegidragoV, 0}};
    expect_tapu_placement(strict, std::move(state), false,
                          "A full Bench must block setup placement.");
  }
}

}  // namespace

int main() {
  test_approved_opening_setup_benches_redundant_tapu();
  test_seed_one_downstream_policy_characterization();
  test_opening_tapu_bench_controls();
  return 0;
}
