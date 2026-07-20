#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static std::optional<Card> final_surplus_cost(const Engine& engine) {
    return engine.quick_ball_final_surplus_energy_cost();
  }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(false); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State route_state(const bool manual_energy_used) {
  sim::State state;
  state.turn = 3;
  state.manual_energy_used = manual_energy_used;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1}};
  state.hand = {sim::Card::QuickBall, sim::Card::Grass,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::LatiasEx, sim::Card::MegaDragonite};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_unused_attachment_is_not_energy_demand() {
  const sim::Scenario scenario{"issue-1155", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{115500};
  sim::Engine engine = make_engine(scenario, rng, route_state(false));

  // The selected VSTAR already has GGF. Attachment availability alone gives the
  // singleton Grass no remaining setup role, while Quick Ball, Latias ex, and held
  // Blender complete the Active-position and same-turn payload axes:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1155
  expect(sim::EngineTestAccess::final_surplus_cost(engine) == sim::Card::Grass,
         "An unused attachment must not protect Energy with zero setup demand.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball must use the final surplus Grass while attachment is unused.");
  expect(std::find(engine.state().discard.begin(), engine.state().discard.end(),
                   sim::Card::Grass) != engine.state().discard.end(),
         "Quick Ball must discard the observable surplus Grass.");
}

void test_spent_attachment_route_remains_available() {
  const sim::Scenario scenario{"issue-1155-spent", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{115501};
  sim::Engine engine = make_engine(scenario, rng, route_state(true));

  // Preserve the merged #1146 route while removing only the historical attachment gate:
  // https://github.com/FlareZ123/pokemon-sims/issues/1146
  // https://github.com/FlareZ123/pokemon-sims/issues/1155
  expect(sim::EngineTestAccess::final_surplus_cost(engine) == sim::Card::Grass,
         "The spent-attachment positive route must remain available.");
}

void test_genuine_route_requirements_still_protect_energy() {
  const sim::Scenario scenario{"issue-1155-controls",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, false, 5};

  std::mt19937_64 rng1{115502};
  sim::State energy_incomplete = route_state(false);
  energy_incomplete.bench.front().grass = 1;
  sim::Engine engine1 = make_engine(scenario, rng1, std::move(energy_incomplete));
  expect(!sim::EngineTestAccess::final_surplus_cost(engine1),
         "Grass must remain protected while the selected VSTAR lacks GGF.");

  std::mt19937_64 rng2{115503};
  sim::State no_payload_route = route_state(false);
  no_payload_route.hand.erase(
      std::find(no_payload_route.hand.begin(), no_payload_route.hand.end(),
                sim::Card::BrilliantBlender));
  sim::Engine engine2 = make_engine(scenario, rng2, std::move(no_payload_route));
  expect(!sim::EngineTestAccess::final_surplus_cost(engine2),
         "Energy must remain protected when no same-turn payload route exists.");

  std::mt19937_64 rng3{115504};
  sim::State no_latias = route_state(false);
  no_latias.deck.erase(std::find(no_latias.deck.begin(), no_latias.deck.end(),
                                 sim::Card::LatiasEx));
  sim::Engine engine3 = make_engine(scenario, rng3, std::move(no_latias));
  expect(!sim::EngineTestAccess::final_surplus_cost(engine3),
         "Energy must remain protected when Latias ex is unavailable.");

  std::mt19937_64 rng4{115505};
  sim::State full_bench = route_state(false);
  while (full_bench.bench.size() < 5) {
    full_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  }
  sim::Engine engine4 = make_engine(scenario, rng4, std::move(full_bench));
  expect(!sim::EngineTestAccess::final_surplus_cost(engine4),
         "Energy must remain protected without Bench space for Latias ex.");

  std::mt19937_64 rng5{115506};
  sim::State retreat_spent = route_state(false);
  retreat_spent.retreat_used = true;
  sim::Engine engine5 = make_engine(scenario, rng5, std::move(retreat_spent));
  expect(!sim::EngineTestAccess::final_surplus_cost(engine5),
         "Energy must remain protected after the retreat action is spent.");

  const sim::Scenario ability_lock{"issue-1155-lock",
                                   sim::DciProfile::MatchupFlexJit,
                                   sim::LockMode::FullRuleBoxAbility, false, 5};
  std::mt19937_64 rng6{115507};
  sim::Engine engine6 = make_engine(ability_lock, rng6, route_state(false));
  expect(!sim::EngineTestAccess::final_surplus_cost(engine6),
         "Energy must remain protected while Skyliner is suppressed.");

  // These controls preserve the remaining Energy, payload, Bench, Ability, and
  // retreat gates documented by the confirmed issue and repository policy:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-76
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/1155
}
}  // namespace

int main() {
  try {
    test_unused_attachment_is_not_energy_demand();
    test_spent_attachment_route_remains_available();
    test_genuine_route_requirements_still_protect_energy();
    std::cout << "Issue 1155 unused-attachment Quick Ball tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
