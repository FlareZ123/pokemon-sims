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

sim::State k0_state() {
  sim::State state;
  state.turn = 3;
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

void seed_38_reaches_t4() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-2301 seed fixture is unavailable.");
  std::mt19937_64 rng{38};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // T3 Quick Ball establishes the Basic's evolution timer and the first legal
  // deck inspection. Only after K1 proves every T4 target may the route bank.
  // T4 Earthen Vessel discards the reserved Dragon as the current-turn payload;
  // Secret Box then spends only route-replaced resources and the Forretress line
  // completes GGF plus Tapu Lele-GX's Retreat Cost.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1, dynamic DCI, strict JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "Seed 38 did not improve to deterministic T4 readiness.");
  expect(trace_has(trace, "Quick Ball issue-2301 timer cost"),
         "Seed 38 did not spend one of two Dragons to establish the T3 timer.");
  expect(trace_has(trace, "T3 | BANK ROUTE"),
         "Seed 38 did not bank the K1-proven continuation.");
  expect(trace_has(trace, "Earthen Vessel issue-2301 T4 payload cost"),
         "Seed 38 did not use Vessel's Dragon discard as the T4 payload.");
  expect(trace_has(trace, "T4 | COMPLETE ROUTE") &&
             trace_has(trace, "T4 | READY"),
         "Seed 38 did not complete the proven T4 route.");
}

void k0_requires_two_payloads_and_no_lower_dci_cost() {
  auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-2301 K0 fixture is unavailable.");

  std::mt19937_64 rng{2301};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, k0_state(), false);

  // The K0 exception is allowed only because one Dragon can buy the evolution
  // timer while a second distinct Dragon remains protected for T4. Any ordinary
  // lower-DCI Quick Ball cost must preempt this exception.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(sim::EngineTestAccess::k0_available(engine),
         "The exact public K0 timer state was rejected.");

  sim::State one_payload = k0_state();
  one_payload.hand.erase(std::find(one_payload.hand.begin(), one_payload.hand.end(),
                                   sim::Card::GoodraVstar));
  sim::Engine one_payload_engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(one_payload_engine, std::move(one_payload), false);
  expect(!sim::EngineTestAccess::k0_available(one_payload_engine),
         "The route spent the final protected Dragon payload.");

  sim::State lower_dci = k0_state();
  lower_dci.hand.push_back(sim::Card::BattleVipPass);
  sim::Engine lower_dci_engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(lower_dci_engine, std::move(lower_dci), false);
  expect(!sim::EngineTestAccess::k0_available(lower_dci_engine),
         "The Dragon exception displaced a lower-DCI Quick Ball cost.");
}

void k1_requires_every_physical_target() {
  auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The issue-2301 K1 fixture is unavailable.");

  sim::State state = k0_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::QuickBall));
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::GoodraVstar));
  state.hand.push_back(sim::Card::RegidragoV);
  state.deck = {sim::Card::ForestSealStone, sim::Card::Dawn,
                sim::Card::Pineco, sim::Card::ForretressEx,
                sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};

  std::mt19937_64 rng{2302};
  sim::Engine complete(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(complete, state, true);
  // Once Quick Ball establishes K1, exact Prize/deck composition must prove every
  // target. This is the point where hidden-zone information becomes legal input.
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // K1 contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(sim::EngineTestAccess::k1_proven(complete),
         "The complete K1 continuation was not proven.");

  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::ForestSealStone), state.deck.end());
  sim::Engine missing_fss(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(missing_fss, std::move(state), true);
  expect(!sim::EngineTestAccess::k1_proven(missing_fss),
         "The route banked despite a missing Forest Seal Stone target.");
}
}  // namespace

int main() {
  try {
    seed_38_reaches_t4();
    k0_requires_two_payloads_and_no_lower_dci_cost();
    k1_requires_every_physical_target();
    std::cout << "Issue 2301 Pineco Quick Ball timer tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
