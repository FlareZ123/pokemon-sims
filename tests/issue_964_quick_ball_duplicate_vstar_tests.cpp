#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(false); }
};

}  // namespace sim

namespace {

int count_card(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::State duplicate_vstar_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0};
  state.hand = {
      sim::Card::QuickBall,
      sim::Card::RegidragoVstar,
      sim::Card::RegidragoVstar,
  };
  state.deck = {sim::Card::RegidragoV};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, const sim::DeckRecipe& recipe,
                        std::mt19937_64& rng, sim::State state) {
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_duplicate_vstar_pays_quick_ball_for_missing_regidrago_v() {
  const sim::Scenario scenario{"issue-964-positive", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{964};
  sim::Engine engine = make_engine(scenario, recipe, rng, duplicate_vstar_state());

  // Quick Ball may discard any other hand card and searches a Basic Pokémon. With
  // two public Regidrago VSTAR copies, one duplicate may pay the cost while the
  // other remains protected for the Regidrago V that the search establishes:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/964
  if (!sim::EngineTestAccess::play_quick_ball(engine)) {
    throw std::runtime_error("Duplicate VSTAR should make the missing-Regidrago Quick Ball route payable.");
  }
  const sim::State& state = sim::EngineTestAccess::state(engine);
  if (count_card(state.discard, sim::Card::QuickBall) != 1 ||
      count_card(state.discard, sim::Card::RegidragoVstar) != 1 ||
      count_card(state.hand, sim::Card::RegidragoVstar) != 1 ||
      count_card(state.hand, sim::Card::RegidragoV) != 1) {
    throw std::runtime_error("Quick Ball must discard one duplicate VSTAR, retain one, and fetch Regidrago V.");
  }
}

void test_singleton_vstar_remains_protected() {
  const sim::Scenario scenario{"issue-964-singleton", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{965};
  sim::State state = duplicate_vstar_state();
  state.hand.pop_back();
  sim::Engine engine = make_engine(scenario, recipe, rng, std::move(state));

  // The fallback requires two held VSTAR copies. A singleton remains UDP/DCI-zero
  // for this route because spending it would remove the evolution card axis:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/964
  if (sim::EngineTestAccess::play_quick_ball(engine)) {
    throw std::runtime_error("A singleton Regidrago VSTAR must not pay Quick Ball.");
  }
}

void test_full_bench_blocks_duplicate_vstar_route() {
  const sim::Scenario scenario{"issue-964-full-bench", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{966};
  sim::State state = duplicate_vstar_state();
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 0},
      sim::Pokemon{sim::Card::Oricorio, 0},
      sim::Pokemon{sim::Card::DialgaGX, 0},
      sim::Pokemon{sim::Card::MawileGX, 0},
      sim::Pokemon{sim::Card::LatiasEx, 0},
  };
  sim::Engine engine = make_engine(scenario, recipe, rng, std::move(state));

  // A Basic searched by Quick Ball still needs legal Bench capacity before it can
  // establish the modeled Regidrago V axis:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/964
  if (sim::EngineTestAccess::play_quick_ball(engine)) {
    throw std::runtime_error("A full Bench must block the duplicate-VSTAR Regidrago route.");
  }
}

void test_missing_regidrago_target_blocks_duplicate_vstar_route() {
  const sim::Scenario scenario{"issue-964-no-target", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{967};
  sim::State state = duplicate_vstar_state();
  state.hand.push_back(sim::Card::TapuLeleGX);
  state.deck = {sim::Card::Grass};
  sim::Engine engine = make_engine(scenario, recipe, rng, std::move(state));

  // An inspected deck with no Basic Pokémon target cannot legally resolve Quick
  // Ball's search, so no discard cost may be paid:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/964
  if (sim::EngineTestAccess::play_quick_ball(engine)) {
    throw std::runtime_error("Quick Ball must not spend a duplicate VSTAR when no Basic target exists.");
  }
}

void test_existing_lower_dci_cost_keeps_priority() {
  const sim::Scenario scenario{"issue-964-selector-order", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{968};
  sim::State state = duplicate_vstar_state();
  state.hand.push_back(sim::Card::QuickBall);
  sim::Engine engine = make_engine(scenario, recipe, rng, std::move(state));

  // A second Quick Ball is already recognized as lower-DCI legal fuel. The new
  // VSTAR fallback runs only after the ordinary selector finds no cost:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/964
  if (!sim::EngineTestAccess::play_quick_ball(engine)) {
    throw std::runtime_error("The existing duplicate-Quick-Ball route should remain payable.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (count_card(after.discard, sim::Card::QuickBall) != 2 ||
      count_card(after.discard, sim::Card::RegidragoVstar) != 0 ||
      count_card(after.hand, sim::Card::RegidragoVstar) != 2) {
    throw std::runtime_error("Ordinary lower-DCI duplicate Item fuel must remain ahead of VSTAR.");
  }
}

}  // namespace

int main() {
  try {
    test_duplicate_vstar_pays_quick_ball_for_missing_regidrago_v();
    test_singleton_vstar_remains_protected();
    test_full_bench_blocks_duplicate_vstar_route();
    test_missing_regidrago_target_blocks_duplicate_vstar_route();
    test_existing_lower_dci_cost_keeps_priority();
    std::cout << "Issue 964 Quick Ball duplicate-VSTAR tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
