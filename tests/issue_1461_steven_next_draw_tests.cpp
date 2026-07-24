#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = deck_seen;
  }
  static bool play_steven_bank(Engine& engine) {
    return engine.play_steven_for_secret_box_bank();
  }
  static void run_secret_box_turn(Engine& engine) {
    engine.run_secret_box_turn();
  }
  static void set_direct_route(Engine& engine, const bool value) {
    engine.issue_1420_direct_t2_treasure_route_ = value;
  }
  static bool direct_route(const Engine& engine) {
    return engine.issue_1420_direct_t2_treasure_route_;
  }
  static bool used_exploding_energy(const Engine& engine) {
    return engine.outcome_.used_exploding_energy;
  }
  static bool ready(const Engine& engine) {
    return engine.active_is_vstar() && !engine.need_energy() &&
        !engine.need_payload();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State admission_state(const int grass_count, const int fire_count,
                           const bool third_treasure_target) {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure,
                sim::Card::RegidragoVstar, sim::Card::Fire};
  state.deck = {sim::Card::SecretBox, sim::Card::Crispin,
                sim::Card::Dragapult, sim::Card::RegidragoV};
  if (third_treasure_target) state.deck.push_back(sim::Card::MegaDragonite);
  state.deck.insert(state.deck.end(), static_cast<std::size_t>(grass_count),
                    sim::Card::Grass);
  state.deck.insert(state.deck.end(), static_cast<std::size_t>(fire_count),
                    sim::Card::Fire);
  // Keep the registered Pineco shell enabled while proving that the ordinary
  // Crispin route must stand on its own in these controls.
  state.discard = {sim::Card::Pineco, sim::Card::ForretressEx};
  return state;
}

void expect_crispin_admission(const int grass_count, const int fire_count,
                              const bool third_treasure_target,
                              const bool expected_direct) {
  const sim::Scenario scenario{"issue-1461-crispin-admission",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng(1461 + static_cast<std::uint64_t>(grass_count * 100 +
                                                       fire_count * 10 +
                                                       third_treasure_target));
  sim::Engine engine(scenario, sim::pineco_recipe(), rng);
  sim::EngineTestAccess::set_state(
      engine, admission_state(grass_count, fire_count, third_treasure_target),
      true);

  // The direct Crispin branch is bankable only when every legal next draw leaves
  // both Basic Energy types and a legal post-cost Mysterious Treasure target:
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // One-Energy Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Future-card oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#policy-versus-future-card-oracle
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1461
  expect(sim::EngineTestAccess::play_steven_bank(engine),
         "Steven's Resolve did not resolve in the Crispin admission fixture.");
  expect(sim::EngineTestAccess::direct_route(engine) == expected_direct,
         "The Crispin admission did not match the proven next-draw boundaries.");
}

sim::State forretress_admission_state(const int grass_count,
                                      const int fire_count) {
  sim::State state = admission_state(grass_count, fire_count, true);
  state.discard.clear();
  state.hand.push_back(sim::Card::ForretressEx);
  state.bench = {sim::Pokemon{sim::Card::Pineco, 1, 0, 0}};
  return state;
}

void expect_forretress_admission(const int grass_count, const int fire_count,
                                 const bool expected_direct) {
  const sim::Scenario scenario{"issue-1461-forretress-admission",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng(146100 +
                      static_cast<std::uint64_t>(grass_count * 10 + fire_count));
  sim::Engine engine(scenario, sim::pineco_recipe(), rng);
  sim::EngineTestAccess::set_state(
      engine, forretress_admission_state(grass_count, fire_count), true);

  // A final Fire may be drawn because prior-turn Pineco, held Forretress ex,
  // searchable Grass, held Fire, VSTAR, and Treasure preserve a complete T2 route.
  // A final Grass may not be drawn because neither Crispin nor Exploding Energy
  // can supply the second Grass attachment after that draw:
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // One-Energy Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Core draw, Ability, attachment, evolution, and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Observable-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#policy-versus-future-card-oracle
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1461
  expect(sim::EngineTestAccess::play_steven_bank(engine),
         "Steven's Resolve did not resolve in the Forretress admission fixture.");
  expect(sim::EngineTestAccess::direct_route(engine) == expected_direct,
         "The Forretress fallback admission did not match the proven draw boundary.");
}

void test_next_draw_admission_boundaries() {
  expect_crispin_admission(2, 2, true, true);
  expect_crispin_admission(1, 2, true, false);
  expect_crispin_admission(2, 1, true, false);
  expect_crispin_admission(2, 2, false, false);

  expect_forretress_admission(2, 1, true);
  expect_forretress_admission(1, 1, false);
}

void test_final_fire_draw_uses_forretress_fallback() {
  const sim::Scenario scenario{"strict-jit/go-second",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng(1461001);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, sim::pineco_recipe(), rng, &trace);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 1, 0, 0}};
  state.hand = {sim::Card::Crispin, sim::Card::MysteriousTreasure,
                sim::Card::RegidragoVstar, sim::Card::Dragapult,
                sim::Card::ForretressEx, sim::Card::Fire};
  state.deck = {sim::Card::Grass, sim::Card::Grass,
                sim::Card::RegidragoV, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state), true);
  sim::EngineTestAccess::set_direct_route(engine, true);

  // The mandatory draw has removed the final searchable Fire. Policy must inspect
  // that public K1 state before playing Crispin, preserve the held Fire attachment,
  // and continue through the already proved Forretress ex route:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // One-Energy Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Core draw, Supporter, Ability, attachment, evolution, and Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1461
  sim::EngineTestAccess::run_secret_box_turn(engine);
  expect(!sim::EngineTestAccess::direct_route(engine),
         "The final-Fire draw remained marked as a Crispin continuation.");
  expect(sim::EngineTestAccess::used_exploding_energy(engine),
         "The final-Fire draw did not use the proved Forretress fallback.");
  expect(sim::EngineTestAccess::ready(engine),
         "The Forretress fallback did not complete strict-JIT T2 readiness.");
}

void test_exact_paired_rng_stream_completes() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(deck != nullptr, "The registered Pineco deck is unavailable.");

  const std::vector<sim::Scenario> scenarios = sim::all_scenarios();
  const auto scenario = std::find_if(
      scenarios.begin(), scenarios.end(), [](const sim::Scenario& candidate) {
        return candidate.label ==
            "strict-jit-rulebox-ability-lock/go-second";
      });
  expect(scenario != scenarios.end(), "The exact issue scenario is unavailable.");

  constexpr std::uint64_t matrix_seed = 20260723;
  constexpr std::uint64_t seed_stride = 104729;
  constexpr std::uint64_t failing_trial = 56183;
  const std::size_t scenario_index = static_cast<std::size_t>(
      std::distance(scenarios.begin(), scenario));
  std::mt19937_64 rng(matrix_seed + seed_stride * scenario_index);

  // Replay the source-bound stream through the formerly failing trial. Every
  // ordinary draw must produce a completed game outcome instead of an exception:
  // Paired matrix contract: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#generate-the-paired-two-deck-matrices
  // Source-bound diagnostic: https://github.com/FlareZ123/pokemon-sims/actions/runs/30059715059
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1461
  for (std::uint64_t trial = 0; trial <= failing_trial; ++trial) {
    sim::Engine engine(*scenario, deck->recipe, rng);
    static_cast<void>(engine.run());
  }
}

}  // namespace

int main() {
  test_next_draw_admission_boundaries();
  test_final_fire_draw_uses_forretress_fallback();
  test_exact_paired_rng_stream_completes();
}
