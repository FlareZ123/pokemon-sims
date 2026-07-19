#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_known_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }

  static bool prized_treasure_completes(const Engine& engine) {
    return engine.known_prized_mysterious_treasure_completes_current_turn();
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

void test_seed_19_uses_prized_treasure_for_t2() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing strict-JIT going-second scenario");

  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{19};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const auto outcome = engine.run();

  // Gladion may exchange itself for the known Prize, Mysterious Treasure pays one
  // held-card cost and searches a Dragon Pokémon, and the prior-turn Regidrago V may
  // evolve. The held Dialga-GX then satisfies the same-turn strict-JIT payload axis:
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Supporter, Item, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest current-turn route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1059
  expect(outcome.first_ready_turn == 2,
         "Seed 19 did not improve from the slower T3 Arven route to T2");
  expect(trace_contains(trace, "Exchanged Gladion for Mysterious Treasure"),
         "Gladion did not recover the known prized Mysterious Treasure");
  expect(trace_contains(trace, "Dialga-GX (Mysterious Treasure cost)"),
         "Mysterious Treasure did not discard the held strict-JIT payload");
  expect(trace_contains(trace,
                        "Searched a Psychic or Dragon Pokémon: Regidrago VSTAR"),
         "Mysterious Treasure did not search the known deck-resident VSTAR");
  expect(!trace_contains(trace, "Exchanged Gladion for Arven"),
         "The slower prized-Arven route still preempted current-turn completion");
}

sim::State exact_prized_treasure_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::Gladion, sim::Card::DialgaGX};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::RegidragoV,
                sim::Card::Arven};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::Arven,
                  sim::Card::LatiasEx, sim::Card::Channeler,
                  sim::Card::Crispin, sim::Card::RegidragoV};
  return state;
}

bool projected_route_completes(sim::State state,
                               const char* scenario_label = "strict-jit/go-second") {
  const auto scenario = sim::scenario_by_label(scenario_label);
  if (!scenario) throw std::runtime_error("Missing projection scenario");
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1059};
  sim::Engine engine(*scenario, recipe, rng);
  sim::EngineTestAccess::set_known_state(engine, std::move(state));
  return sim::EngineTestAccess::prized_treasure_completes(engine);
}

void test_projection_boundaries() {
  expect(projected_route_completes(exact_prized_treasure_state()),
         "Exact known prized-Treasure route should complete this turn");

  sim::State item_locked = exact_prized_treasure_state();
  expect(!projected_route_completes(std::move(item_locked),
                                    "strict-jit-turn2-item-lock/go-second"),
         "Prized Treasure must remain unavailable under turn-two Item lock");

  sim::State fresh_active = exact_prized_treasure_state();
  fresh_active.active->entered_turn = fresh_active.turn;
  expect(!projected_route_completes(std::move(fresh_active)),
         "A Regidrago V cannot evolve during the turn it entered play");

  sim::State no_payload = exact_prized_treasure_state();
  no_payload.hand = {sim::Card::Gladion, sim::Card::Grass};
  expect(!projected_route_completes(std::move(no_payload)),
         "Treasure recovery must not project readiness without a held payload cost");

  sim::State no_deck_vstar = exact_prized_treasure_state();
  no_deck_vstar.deck = {sim::Card::RegidragoV, sim::Card::Arven};
  expect(!projected_route_completes(std::move(no_deck_vstar)),
         "Treasure recovery must require a known legal VSTAR target in deck");

  sim::State direct_prized_vstar = exact_prized_treasure_state();
  direct_prized_vstar.prizes.back() = sim::Card::RegidragoVstar;
  expect(!projected_route_completes(std::move(direct_prized_vstar)),
         "Direct prized-VSTAR recovery must retain higher decision priority");
}

}  // namespace

int main() {
  test_seed_19_uses_prized_treasure_for_t2();
  test_projection_boundaries();
  return 0;
}
