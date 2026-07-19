#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static std::optional<Card> bridge_payload(const Engine& engine) {
    return engine.late_steven_payload_treasure_bridge_payload();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State bridge_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::StevensResolve, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite, sim::Card::RegidragoV,
                sim::Card::Grass};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return sim::Engine(scenario, recipe, rng);
}

void test_exact_payload_treasure_bridge_is_admitted() {
  const sim::Scenario scenario{"issue-1023-positive", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1023};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, bridge_state());

  // Steven searches the Dragon that will pay Mysterious Treasure next turn, while a
  // second legal Psychic or Dragon target remains in deck:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/issues/1023
  expect(sim::EngineTestAccess::bridge_payload(engine) == sim::Card::MegaDragonite,
         "The exact K1 payload-to-Treasure bridge must select Mega Dragonite ex.");
}

void test_item_lock_rejects_next_turn_treasure() {
  const sim::Scenario scenario{"issue-1023-lock", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::TurnTwoItem, true, 5};
  std::mt19937_64 rng{1024};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, bridge_state());

  // The route requires a legal Mysterious Treasure play on the following turn:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/1023
  expect(!sim::EngineTestAccess::bridge_payload(engine),
         "Modeled Item lock must reject the bridge.");
}

void test_treasure_must_retain_a_search_target() {
  const sim::Scenario scenario{"issue-1023-target", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1025};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = bridge_state();
  state.deck = {sim::Card::MegaDragonite, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // After Steven removes the payload, Mysterious Treasure must still have a legal
  // Psychic or Dragon search target: https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/1023
  expect(!sim::EngineTestAccess::bridge_payload(engine),
         "A targetless future Treasure must not justify Steven.");
}

void test_unresolved_energy_rejects_payload_only_bridge() {
  const sim::Scenario scenario{"issue-1023-energy", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1026};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = bridge_state();
  state.active->fire = 0;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // This narrow route applies only after Regidrago VSTAR already pays GGF:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1023
  expect(!sim::EngineTestAccess::bridge_payload(engine),
         "An unresolved Energy axis must reject the payload-only bridge.");
}

void test_held_payload_preserves_faster_current_turn_route() {
  const sim::Scenario scenario{"issue-1023-faster", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1027};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = bridge_state();
  state.hand.push_back(sim::Card::Dragapult);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A held Dragon can already pay Mysterious Treasure this turn, so Steven's
  // turn-ending bridge is slower: https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1023
  expect(!sim::EngineTestAccess::bridge_payload(engine),
         "A faster current-turn payload route must outrank Steven.");
}

void test_seed_184_reaches_turn_four() {
  const sim::Scenario scenario{"matchup-flex-jit/go-first",
                               sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{184};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Exact source-bound regression: Steven supplies Mega Dragonite ex on T3, then
  // Mysterious Treasure discards it on T4 and readiness is reached that turn:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1023
  expect(trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-STEVEN-01"),
         "Seed 184 must play Steven on T3.");
  expect(trace_contains(trace, "T4 | DISCARD | rules: R-MT-01 | Mega Dragonite ex"),
         "Seed 184 must discard the searched payload with Treasure on T4.");
  expect(trace_contains(trace, "T4 | READY"),
         "Seed 184 must reach readiness on T4.");
  expect(outcome.ready_by_4 && outcome.first_ready_turn == 4,
         "Seed 184 must first become ready on T4.");
}
}  // namespace

int main() {
  test_exact_payload_treasure_bridge_is_admitted();
  test_item_lock_rejects_next_turn_treasure();
  test_treasure_must_retain_a_search_target();
  test_unresolved_energy_rejects_payload_only_bridge();
  test_held_payload_preserves_faster_current_turn_route();
  test_seed_184_reaches_turn_four();
  return 0;
}
