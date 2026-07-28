#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = false;
  }
  static bool available(const Engine& engine) {
    return engine.issue_1711_k0_treasure_turo_crispin_available();
  }
  static bool play(Engine& engine) {
    return engine.play_issue_1711_k0_treasure_turo_crispin_route();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

sim::State exact_t2_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::ProfessorTuro,
                sim::Card::RegidragoVstar, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::MegaDragonite,
                sim::Card::MegaDragonite, sim::Card::Appletun};
  state.deck = {sim::Card::RegidragoV, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::Dragapult};
  return state;
}

sim::Scenario exact_scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1711", sim::DciProfile::StrictJit,
                       lock, false, 3};
}

void test_registered_seed_reaches_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "Issue 1711 registered fixture is unavailable.");
  std::mt19937_64 rng{1337};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The legal K0 Treasure action spends one duplicated payload, establishes K1,
  // then Turo, replayed Wonder Tag, Crispin, and Vessel complete the earliest T3:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1 and dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1711
  expect(outcome.first_ready_turn == 3,
         "Pineco seed 1337 did not reach its earliest legal T3 state.");
  expect(trace_contains(trace, "Mysterious Treasure issue-1711") &&
             trace_contains(trace, "Professor Turo returned Active Tapu") &&
             trace_contains(trace, "Wonder Tag to search Crispin") &&
             trace_contains(trace, "T3 | READY"),
         "The source-bound trace omitted a required issue-1711 route step.");
}

void test_repeated_payload_and_k0_gates() {
  std::mt19937_64 rng{17110};
  sim::Engine engine(exact_scenario(), sim::pineco_recipe(), rng);
  sim::State state = exact_t2_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::Appletun));
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::available(engine),
         "A duplicated identity without a distinct surviving payload passed DCI.");

  sim::EngineTestAccess::set_state(engine, exact_t2_state(), true);
  expect(!sim::EngineTestAccess::available(engine),
         "The K0-only route ran after deck knowledge already existed.");
}

void test_lock_and_bench_gates() {
  std::mt19937_64 locked_rng{17111};
  sim::Engine locked(exact_scenario(sim::LockMode::FullRuleBoxAbility),
                     sim::pineco_recipe(), locked_rng);
  sim::EngineTestAccess::set_state(locked, exact_t2_state());
  expect(!sim::EngineTestAccess::available(locked),
         "Wonder Tag route ignored Rule Box Ability lock.");

  std::mt19937_64 bench_rng{17112};
  sim::Engine occupied(exact_scenario(), sim::pineco_recipe(), bench_rng);
  sim::State state = exact_t2_state();
  state.bench.push_back(sim::Pokemon{sim::Card::Pineco, 1});
  sim::EngineTestAccess::set_state(occupied, state);
  expect(!sim::EngineTestAccess::available(occupied),
         "The narrow promotion route accepted an occupied Bench state.");
}

void test_k1_missing_crispin_falls_back_after_legal_search() {
  std::mt19937_64 rng{17113};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(exact_scenario(), sim::pineco_recipe(), rng, &trace);
  sim::State state = exact_t2_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::Crispin));
  sim::EngineTestAccess::set_state(engine, state);
  expect(!sim::EngineTestAccess::play(engine),
         "The complete continuation was reported after K1 proved Crispin absent.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(std::count(after.discard.begin(), after.discard.end(),
                    sim::Card::MysteriousTreasure) == 1 &&
             std::count(after.discard.begin(), after.discard.end(),
                        sim::Card::MegaDragonite) == 1 &&
             std::count(after.hand.begin(), after.hand.end(),
                        sim::Card::ProfessorTuro) == 1,
         "The legal K0 search did not preserve the unresolved Turo continuation.");
}
}  // namespace

int main() {
  try {
    test_registered_seed_reaches_t3();
    test_repeated_payload_and_k0_gates();
    test_lock_and_bench_gates();
    test_k1_missing_crispin_falls_back_after_legal_search();
    std::cout << "Issue 1711 K0 Treasure-Turo-Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
