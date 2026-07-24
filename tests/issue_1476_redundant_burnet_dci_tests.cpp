#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = false;
  }

  static std::optional<Card> choose_discard(const Engine& engine) {
    return engine.choose_discard(
        false, false, true, Card::MysteriousTreasure, false);
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

std::size_t trace_index(const sim::TraceLog& trace, const std::string& text) {
  const auto it = std::find_if(trace.lines.begin(), trace.lines.end(),
                               [&text](const std::string& line) {
                                 return line.find(text) != std::string::npos;
                               });
  return static_cast<std::size_t>(std::distance(trace.lines.begin(), it));
}

sim::State public_t1_route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0}};
  state.hand = {sim::Card::RegidragoVstar,
                sim::Card::MysteriousTreasure,
                sim::Card::QuickBall,
                sim::Card::ProfessorBurnet,
                sim::Card::BrilliantBlender,
                sim::Card::LatiasEx};
  return state;
}

std::optional<sim::Card> choose_from_state(
    const sim::LockMode lock, sim::State state, const std::uint64_t seed) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "The registered shell recipe is unavailable.");
  const sim::Scenario scenario{
      "issue-1476", sim::DciProfile::StrictJit, lock, true, 5};
  std::mt19937_64 rng(seed);
  sim::Engine engine(scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::choose_discard(engine);
}

void test_blender_dominates_burnet_on_complete_public_route() {
  // Held Blender covers Burnet's only modeled deck-payload function. The public
  // Tapu Lele-GX -> Crispin connector, K1 Oricorio route, in-place evolution,
  // Energy inventory, and two open Bench slots make Burnet legal dynamic-DCI fuel:
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1476
  expect(choose_from_state(sim::LockMode::None, public_t1_route_state(), 147601) ==
             sim::Card::ProfessorBurnet,
         "The complete public T1 route did not expose redundant Burnet as Treasure fuel.");
}

void test_route_boundaries_keep_burnet_protected() {
  sim::State no_blender = public_t1_route_state();
  no_blender.hand.erase(std::find(no_blender.hand.begin(), no_blender.hand.end(),
                                  sim::Card::BrilliantBlender));
  expect(!choose_from_state(sim::LockMode::None, std::move(no_blender), 147602),
         "Burnet became discardable without the dominating Blender outlet.");

  sim::State no_vstar = public_t1_route_state();
  no_vstar.hand.erase(std::find(no_vstar.hand.begin(), no_vstar.hand.end(),
                                sim::Card::RegidragoVstar));
  expect(!choose_from_state(sim::LockMode::None, std::move(no_vstar), 147603),
         "Burnet became discardable without the held VSTAR card axis.");

  sim::State no_oricorio_possible = public_t1_route_state();
  no_oricorio_possible.discard.push_back(sim::Card::Oricorio);
  expect(!choose_from_state(sim::LockMode::None, std::move(no_oricorio_possible),
                            147604),
         "Burnet became discardable without a publicly possible Oricorio connector.");

  sim::State crowded_bench = public_t1_route_state();
  crowded_bench.bench.push_back(sim::Pokemon{sim::Card::LatiasEx, 0});
  crowded_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 0});
  crowded_bench.bench.push_back(sim::Pokemon{sim::Card::CrobatV, 0});
  expect(!choose_from_state(sim::LockMode::None, std::move(crowded_bench), 147605),
         "Burnet became discardable without two open Bench slots.");

  // Item lock prevents both the initiating Treasure and the future Blender, so
  // Burnet retains its singleton protection:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/1476
  expect(!choose_from_state(sim::LockMode::FullItem, public_t1_route_state(),
                            147606),
         "Item lock incorrectly exposed Burnet as discard fuel.");
}

void test_seed_129_reaches_t3_without_k0_oracle_use() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1476 seed fixture is unavailable.");

  std::mt19937_64 rng(129);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Burnet pays Treasure's cost from the public K0 hand. Treasure establishes K1,
  // Tapu Lele-GX banks Crispin, and the post-evolution Quick Ball spends inert
  // Latias ex to fetch Oricorio. Vital Dance supplies the final Energy channel and
  // Blender supplies the same-turn T3 payload:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1476
  expect(outcome.first_ready_turn == 3,
         "Seed 129 did not improve from failure through T5 to strict-JIT T3 readiness.");
  expect(trace_contains(trace, "Professor Burnet (Mysterious Treasure cost)"),
         "Seed 129 did not spend redundant Burnet on the initiating Treasure.");
  expect(trace_contains(trace, "T1 | WONDER TAG") &&
             !trace_contains(trace, "T2 | WONDER TAG"),
         "Seed 129 did not preserve the single approved Wonder Tag connector.");
  expect(trace_contains(trace, "Latias ex (Quick Ball cost)"),
         "Seed 129 did not wait for evolution before spending inert Latias ex.");
  expect(trace_contains(trace, "Searched a Basic Pokemon: Oricorio") ||
             trace_contains(trace, "Searched a Basic Pokémon: Oricorio"),
         "Seed 129 did not search the K1-proven Oricorio connector.");
  expect(trace_contains(trace, "Vital Dance"),
         "Seed 129 did not resolve Oricorio's Energy search.");
  expect(trace_contains(trace, "T3 | READY"),
         "Seed 129 did not complete GGF and the same-turn Blender payload on T3.");

  const std::size_t cost = trace_index(
      trace, "Professor Burnet (Mysterious Treasure cost)");
  const std::size_t knowledge = trace_index(
      trace, "Mysterious Treasure: deck inspected");
  expect(cost < knowledge,
         "Seed 129 used deck knowledge before paying Mysterious Treasure's cost.");
}

}  // namespace

int main() {
  test_blender_dominates_burnet_on_complete_public_route();
  test_route_boundaries_keep_burnet_protected();
  test_seed_129_reaches_t3_without_k0_oracle_use();
  return 0;
}
