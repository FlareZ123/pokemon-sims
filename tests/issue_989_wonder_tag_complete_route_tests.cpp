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
    engine.deck_seen_ = true;
  }
  static bool needs_tapu_connector(Engine& engine) {
    return engine.needs_tapu_connector();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool benched(const sim::State& state, const sim::Card card) {
  return std::any_of(state.bench.begin(), state.bench.end(),
                     [card](const sim::Pokemon& pokemon) {
                       return pokemon.card == card;
                     });
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void expect_trace_skips_unused_crispin(const sim::Scenario& scenario,
                                       const std::uint64_t seed,
                                       const int ready_turn) {
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{seed};
  sim::Engine engine(scenario, recipe, rng);
  const sim::TrialOutcome outcome = engine.run();
  const sim::State& state = engine.state();

  // These exact main-branch witnesses previously spent Tapu Lele-GX and Wonder Tag
  // for Crispin even though a held Basic Energy/manual attachment plus held payload,
  // evolution, and promotion resources already formed the shortest complete route:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/989
  expect(outcome.first_ready_turn == ready_turn,
         "The corrected route must preserve the earliest ready turn.");
  expect(!benched(state, sim::Card::TapuLeleGX),
         "The redundant Tapu Lele-GX must remain off the Bench.");
  expect(!contains(state.hand, sim::Card::Crispin),
         "Unused Crispin must not be searched into hand.");
}

sim::State held_burnet_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 3, 0,
                              sim::Tool::ForestSealStone};
  state.bench = {sim::Pokemon{sim::Card::Oricorio, 1, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::RegidragoVstar,
                sim::Card::Fire, sim::Card::TapuLeleGX};
  state.deck = {sim::Card::Crispin, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool needs_tapu(sim::State state, const sim::LockMode lock) {
  const sim::Scenario scenario{"issue-989", sim::DciProfile::StrictJit,
                               lock, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{989};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::needs_tapu_connector(engine);
}

void test_three_full_trace_routes_skip_redundant_crispin() {
  expect_trace_skips_unused_crispin(
      sim::Scenario{"strict-jit-turn2-item-lock/go-second",
                    sim::DciProfile::StrictJit,
                    sim::LockMode::TurnTwoItem, false, 4},
      25, 2);
  expect_trace_skips_unused_crispin(
      sim::Scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                    sim::LockMode::None, true, 4},
      107, 3);
  expect_trace_skips_unused_crispin(
      sim::Scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                    sim::LockMode::None, true, 5},
      122, 4);
}

void test_item_lock_burnet_route_suppresses_wonder_tag() {
  expect(!needs_tapu(held_burnet_state(), sim::LockMode::TurnTwoItem),
         "Held Burnet, VSTAR, Fire, and manual attachment must suppress Crispin.");
}

void test_missing_manual_energy_preserves_wonder_tag() {
  sim::State state = held_burnet_state();
  std::erase(state.hand, sim::Card::Fire);
  expect(needs_tapu(std::move(state), sim::LockMode::TurnTwoItem),
         "Wonder Tag must remain live when Crispin is required for Energy.");
}

void test_used_manual_attachment_preserves_wonder_tag() {
  sim::State state = held_burnet_state();
  state.manual_energy_used = true;
  expect(needs_tapu(std::move(state), sim::LockMode::TurnTwoItem),
         "Wonder Tag must remain live after the manual attachment is spent.");
}

void test_missing_payload_route_preserves_wonder_tag() {
  sim::State state = held_burnet_state();
  std::erase(state.hand, sim::Card::ProfessorBurnet);
  expect(needs_tapu(std::move(state), sim::LockMode::TurnTwoItem),
         "Wonder Tag must remain live when no payload route completes this turn.");
}

void test_fresh_regidrago_preserves_wonder_tag() {
  sim::State state = held_burnet_state();
  state.active->entered_turn = state.turn;
  expect(needs_tapu(std::move(state), sim::LockMode::TurnTwoItem),
         "Wonder Tag must remain live when Regidrago V cannot evolve this turn.");
}

}  // namespace

int main() {
  try {
    test_three_full_trace_routes_skip_redundant_crispin();
    test_item_lock_burnet_route_suppresses_wonder_tag();
    test_missing_manual_energy_preserves_wonder_tag();
    test_used_manual_attachment_preserves_wonder_tag();
    test_missing_payload_route_preserves_wonder_tag();
    test_fresh_regidrago_preserves_wonder_tag();
    std::cout << "Issue 989 Wonder Tag complete-route tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
