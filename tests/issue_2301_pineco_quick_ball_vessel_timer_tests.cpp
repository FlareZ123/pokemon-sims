#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = false;
  }
  static bool k0_available(Engine& engine) {
    return engine.issue_2301_quick_ball_timer_cost().has_value();
  }
  static bool k1_proven(Engine& engine) {
    return engine.issue_2301_k1_t4_route_proven();
  }
  static void set_bank(Engine& engine, const int ready_turn) {
    engine.issue_2301_banked_ready_turn_ = ready_turn;
  }
  static bool finish_bank(Engine& engine) {
    return engine.finish_issue_2301_t3_bank();
  }
  static bool complete_bank(Engine& engine) {
    return engine.complete_issue_2301_banked_t4_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
    return line.find(needle) != std::string::npos;
  });
}

sim::Scenario route_scenario(const sim::DciProfile dci, const bool going_first,
                             const int max_turn) {
  return sim::Scenario{"issue-3269", dci, sim::LockMode::None,
                       going_first, max_turn};
}

sim::State k0_state(const int turn = 3) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.stadium = sim::Stadium::ForestOfVitality;
  state.hand = {sim::Card::QuickBall, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::SecretBox,
                sim::Card::GoodraVstar, sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoV, sim::Card::ForestSealStone,
                sim::Card::Dawn, sim::Card::Pineco,
                sim::Card::ForretressEx, sim::Card::RegidragoVstar,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire};
  return state;
}

sim::State k1_state(const int turn) {
  sim::State state = k0_state(turn);
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::QuickBall));
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::GoodraVstar));
  state.hand.push_back(sim::Card::RegidragoV);
  state.deck = {sim::Card::ForestSealStone, sim::Card::Dawn,
                sim::Card::Pineco, sim::Card::ForretressEx,
                sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::State completion_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.stadium = sim::Stadium::ForestOfVitality;
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, turn - 1, 1, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::EarthenVessel, sim::Card::SecretBox,
                sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::ForestSealStone, sim::Card::Dawn,
                sim::Card::Pineco, sim::Card::ForretressEx,
                sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  return state;
}

void seed_38_reaches_t4() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-2301 seed fixture is unavailable.");
  std::mt19937_64 rng{38};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Quick Ball establishes the Basic's evolution timer and K1 one turn before
  // readiness. Earthen Vessel discards the reserved Dragon on the ready turn;
  // Secret Box and the Forest/Dawn/Forretress continuation then complete GGF.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Relative Bench/evolution procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K0/K1, dynamic DCI, shared same-ready JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Generalization bug: https://github.com/FlareZ123/pokemon-sims/issues/3269
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "Seed 38 did not preserve deterministic T4 readiness.");
  expect(trace_has(trace, "Quick Ball issue-2301 timer cost"),
         "Seed 38 did not spend one of two Dragons to establish the timer.");
  expect(trace_has(trace, "BANK ROUTE"),
         "Seed 38 did not bank the K1-proven continuation.");
  expect(trace_has(trace, "Earthen Vessel issue-2301 ready-turn payload cost"),
         "Seed 38 did not use Vessel's Dragon discard as the ready-turn payload.");
  expect(trace_has(trace, "COMPLETE ROUTE") && trace_has(trace, "READY"),
         "Seed 38 did not complete the proven ready-turn route.");
}

void relative_timer_and_k1_coordinates() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(deck != nullptr, "The issue-3269 deck fixture is unavailable.");

  struct Coordinate {
    sim::DciProfile dci;
    bool going_first;
    int setup_turn;
    int ready_turn;
  };
  const Coordinate coordinates[] = {
      {sim::DciProfile::StrictJit, false, 1, 2},
      {sim::DciProfile::StrictJit, true, 2, 3},
      {sim::DciProfile::MatchupFlexJit, false, 1, 2},
      {sim::DciProfile::MatchupFlexJit, true, 2, 3},
      {sim::DciProfile::StrictJit, false, 3, 4},
  };

  // The printed cards create a relative timer: a Basic benched this turn becomes
  // evolution-eligible on a later turn, while Vessel supplies the same-ready-turn
  // Apex Dragon payload. Seat identity and absolute T3/T4 coordinates are witnesses,
  // not rules. https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/3269
  for (const Coordinate& coordinate : coordinates) {
    std::mt19937_64 rng{3269U + static_cast<unsigned>(coordinate.setup_turn)};
    const sim::Scenario scenario = route_scenario(
        coordinate.dci, coordinate.going_first, coordinate.ready_turn);

    sim::Engine k0_engine(scenario, deck->recipe, rng);
    sim::EngineTestAccess::set_state(k0_engine, k0_state(coordinate.setup_turn), false);
    expect(sim::EngineTestAccess::k0_available(k0_engine),
           "A legal relative K0 Quick Ball timer coordinate was rejected.");

    sim::Engine k1_engine(scenario, deck->recipe, rng);
    sim::EngineTestAccess::set_state(k1_engine, k1_state(coordinate.setup_turn), true);
    expect(sim::EngineTestAccess::k1_proven(k1_engine),
           "A legal relative K1 next-turn continuation was rejected.");
  }
}

void relative_bank_and_completion_coordinates() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(deck != nullptr, "The issue-3269 deck fixture is unavailable.");

  // These exercise the two downstream predicates that historically encoded T3
  // and T4 directly. The only time requirement is the banked next-turn identity
  // plus normal evolution legality. https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/me1-117
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://github.com/FlareZ123/pokemon-sims/issues/3269
  for (const int setup_turn : {1, 2, 4}) {
    std::mt19937_64 rng{3300U + static_cast<unsigned>(setup_turn)};
    const sim::Scenario scenario = route_scenario(
        sim::DciProfile::StrictJit, setup_turn == 2, setup_turn + 1);
    sim::Engine engine(scenario, deck->recipe, rng);
    sim::State state = k1_state(setup_turn);
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    sim::EngineTestAccess::set_bank(engine, setup_turn + 1);
    expect(sim::EngineTestAccess::finish_bank(engine),
           "The relative setup-turn bank finisher was rejected.");
  }

  for (const int ready_turn : {2, 3, 5}) {
    std::mt19937_64 rng{3400U + static_cast<unsigned>(ready_turn)};
    const sim::Scenario scenario = route_scenario(
        sim::DciProfile::StrictJit, ready_turn == 3, ready_turn);
    sim::Engine engine(scenario, deck->recipe, rng);
    sim::EngineTestAccess::set_state(engine, completion_state(ready_turn), true);
    sim::EngineTestAccess::set_bank(engine, ready_turn);
    expect(sim::EngineTestAccess::complete_bank(engine),
           "The relative banked ready-turn completion was rejected.");
  }
}

void k0_requires_two_payloads_and_no_lower_dci_cost() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(deck != nullptr, "The issue-2301 K0 fixture is unavailable.");
  const sim::Scenario scenario = route_scenario(
      sim::DciProfile::StrictJit, false, 4);

  std::mt19937_64 rng{2301};
  sim::Engine engine(scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, k0_state(), false);

  // One Dragon may buy the timer only while a second distinct Dragon remains
  // protected for the ready turn. Every ordinary lower-DCI cost wins first.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  expect(sim::EngineTestAccess::k0_available(engine),
         "The exact public K0 timer state was rejected.");

  sim::State one_payload = k0_state();
  one_payload.hand.erase(std::find(one_payload.hand.begin(), one_payload.hand.end(),
                                   sim::Card::GoodraVstar));
  sim::Engine one_payload_engine(scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(one_payload_engine, std::move(one_payload), false);
  expect(!sim::EngineTestAccess::k0_available(one_payload_engine),
         "The route spent the final protected Dragon payload.");

  sim::State lower_dci = k0_state();
  lower_dci.hand.push_back(sim::Card::BattleVipPass);
  sim::Engine lower_dci_engine(scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(lower_dci_engine, std::move(lower_dci), false);
  expect(!sim::EngineTestAccess::k0_available(lower_dci_engine),
         "The Dragon exception displaced a lower-DCI Quick Ball cost.");
}

void k1_requires_every_physical_target() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(deck != nullptr, "The issue-2301 K1 fixture is unavailable.");
  const sim::Scenario scenario = route_scenario(
      sim::DciProfile::StrictJit, false, 4);

  sim::State state = k1_state(3);
  std::mt19937_64 rng{2302};
  sim::Engine complete(scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(complete, state, true);
  // K1 is where exact Prize/deck composition becomes legal input.
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  expect(sim::EngineTestAccess::k1_proven(complete),
         "The complete K1 continuation was not proven.");

  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::ForestSealStone), state.deck.end());
  sim::Engine missing_fss(scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(missing_fss, std::move(state), true);
  expect(!sim::EngineTestAccess::k1_proven(missing_fss),
         "The route banked despite a missing Forest Seal Stone target.");
}
}  // namespace

int main() {
  try {
    seed_38_reaches_t4();
    relative_timer_and_k1_coordinates();
    relative_bank_and_completion_coordinates();
    k0_requires_two_payloads_and_no_lower_dci_cost();
    k1_requires_every_physical_target();
    std::cout << "Issue 2301/3269 Pineco relative timer tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
