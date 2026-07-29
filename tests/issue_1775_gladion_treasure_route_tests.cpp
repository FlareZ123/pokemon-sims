#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = false;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1775_gladion_treasure_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1775_gladion_treasure_route();
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

sim::State exact_t3_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 2, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::Gladion, sim::Card::Serena,
                sim::Card::PathToPeak, sim::Card::EarthenVessel,
                sim::Card::StevensResolve, sim::Card::RoseannesBackup,
                sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::Fire, sim::Card::MegaDragonite};
  state.prizes = {sim::Card::MysteriousTreasure, sim::Card::RegidragoV,
                  sim::Card::HisuianHeavyBall, sim::Card::BrilliantBlender,
                  sim::Card::ProfessorBurnet, sim::Card::MegaDragonite};
  state.discard = {sim::Card::Crispin, sim::Card::Arven,
                   sim::Card::LatiasEx, sim::Card::Channeler};
  return state;
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1775", sim::DciProfile::StrictJit, lock, true, 5};
}

sim::Engine make_engine(const sim::Scenario& selected, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  if (deck == nullptr) throw std::runtime_error("Registered shell is unavailable.");
  return sim::Engine(selected, deck->recipe, rng, trace);
}

void test_exact_k1_route_completes_t3() {
  sim::Scenario selected = scenario();
  std::mt19937_64 rng{1775};
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(selected, rng, &trace);
  sim::EngineTestAccess::set_state(engine, exact_t3_state());

  // The legally inspected state exposes one complete route: Vessel discards the
  // route-replaced Steven, the manual Grass attachment completes GGF, Gladion takes
  // the known-prized Treasure, and Treasure discards Dragapult ex while searching
  // the evolution card. Brilliant Blender advances only the payload axis here:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, DCI, and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1775
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact K1 Gladion-Treasure route was rejected.");
  expect(sim::EngineTestAccess::complete_route(engine),
         "The exact K1 Gladion-Treasure route did not complete.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.supporter_used && after.manual_energy_used,
         "The corrected route did not consume its Supporter and attachment.");
  expect(after.active && after.active->card == sim::Card::RegidragoVstar &&
             after.active->grass == 2 && after.active->fire == 1,
         "The corrected route did not produce the Active GGF VSTAR.");
  expect(std::find(after.discarded_this_turn.begin(),
                   after.discarded_this_turn.end(),
                   sim::Card::Dragapult) != after.discarded_this_turn.end(),
         "Dragapult ex was not recorded as the strict-JIT payload.");
  expect(std::find(after.prizes.begin(), after.prizes.end(),
                   sim::Card::Gladion) != after.prizes.end(),
         "Gladion was not exchanged into the Prize cards.");
  expect(trace_contains(trace, "known prized Mysterious Treasure") &&
             trace_contains(trace, "current-turn payload"),
         "The corrected route trace omitted its K1 or strict-JIT action.");
}

void test_route_gates() {
  const auto rejected = [](sim::State state, sim::Scenario selected,
                           const bool k1, const std::uint64_t seed,
                           const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(selected, rng);
    sim::EngineTestAccess::set_state(engine, std::move(state), k1);
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  rejected(exact_t3_state(), scenario(), false, 17750,
           "The K1-only route was admitted at K0.");
  rejected(exact_t3_state(), scenario(sim::LockMode::FullItem), true, 17751,
           "The route bypassed Item lock.");
  {
    sim::State state = exact_t3_state();
    state.prizes.erase(std::remove(state.prizes.begin(), state.prizes.end(),
                                   sim::Card::MysteriousTreasure),
                       state.prizes.end());
    rejected(std::move(state), scenario(), true, 17752,
             "The route ignored the absence of prized Mysterious Treasure.");
  }
  {
    sim::State state = exact_t3_state();
    state.active->entered_turn = state.turn;
    rejected(std::move(state), scenario(), true, 17753,
             "A same-turn Regidrago V was treated as evolution-eligible.");
  }
  for (const sim::Card missing : {sim::Card::EarthenVessel,
                                  sim::Card::StevensResolve,
                                  sim::Card::Gladion,
                                  sim::Card::Dragapult}) {
    sim::State state = exact_t3_state();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(), missing),
                     state.hand.end());
    rejected(std::move(state), scenario(), true,
             17760 + static_cast<std::uint64_t>(missing),
             "The route ignored a missing required hand card.");
  }
}

void test_registered_seed_reaches_t3() {
  const auto selected = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value() && deck != nullptr,
         "The registered issue-1775 fixture is unavailable.");
  std::mt19937_64 rng{38};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*selected, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Seed 38 must replace Gladion -> Brilliant Blender plus Celestial Roar with
  // the deterministic Vessel -> Gladion -> Treasure -> VSTAR T3 route:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1775
  // Rebased compatibility checks: https://github.com/FlareZ123/pokemon-sims/issues/1795 https://github.com/FlareZ123/pokemon-sims/issues/1796
  expect(outcome.first_ready_turn == 3,
         "Registered seed 38 did not improve from T5 to T3 readiness.");
  expect(trace_contains(trace, "T3 | Earthen Vessel") &&
             trace_contains(trace, "T3 | ATTACH") &&
             trace_contains(trace, "T3 | PLAY SUPPORTER") &&
             trace_contains(trace, "known prized Mysterious Treasure") &&
             trace_contains(trace, "T3 | PLAY ITEM") &&
             trace_contains(trace, "T3 | EVOLVE") &&
             trace_contains(trace, "T3 | READY") &&
             !trace_contains(trace, "T3 | CELESTIAL ROAR"),
         "The seed-38 trace omitted a corrected-route action or retained Celestial Roar.");
}
}  // namespace

int main() {
  test_exact_k1_route_completes_t3();
  test_route_gates();
  test_registered_seed_reaches_t3();
}
