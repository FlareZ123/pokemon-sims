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
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1878_vessel_quick_ball_tapu_crispin_route_available();
  }
  static bool complete_route(Engine& engine) {
    return engine.complete_issue_1878_vessel_quick_ball_tapu_crispin_route();
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

sim::Scenario strict(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1878", sim::DciProfile::StrictJit, lock, false, 5};
}

sim::State base_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1};
  state.hand = {sim::Card::EarthenVessel, sim::Card::QuickBall,
                sim::Card::Dragapult, sim::Card::StevensResolve,
                sim::Card::BrilliantBlender};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::RegidragoV};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true,
                        const bool prizes_revealed = false) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen,
                                   prizes_revealed);
  return engine;
}

void test_exact_route_and_knowledge_boundaries() {
  std::mt19937_64 rng(1878);
  const sim::Scenario scenario = strict();
  sim::Engine engine = make_engine(scenario, rng, base_state());

  // Vessel spends Steven's Resolve because Crispin consumes the only Supporter
  // action, Quick Ball discards Dragapult ex as the same-turn payload, and Wonder
  // Tag obtains Crispin for the final Grass attachment.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, DCI/JIT, Supporter contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1878
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact issue-1878 route was unavailable.");
  expect(sim::EngineTestAccess::complete_route(engine),
         "The exact issue-1878 route did not complete.");
  expect(engine.state().active->grass == 2 && engine.state().active->fire == 1,
         "The issue-1878 route did not complete GGF.");
  expect(std::find(engine.state().discarded_this_turn.begin(),
                   engine.state().discarded_this_turn.end(),
                   sim::Card::Dragapult) != engine.state().discarded_this_turn.end(),
         "Quick Ball did not create the current-turn Dragon payload.");
  expect(std::find(engine.state().discard.begin(), engine.state().discard.end(),
                   sim::Card::StevensResolve) != engine.state().discard.end(),
         "The route-replaced Supporter did not pay Earthen Vessel.");

  sim::Engine prize_only_k1 =
      make_engine(scenario, rng, base_state(), false, true);
  expect(sim::EngineTestAccess::route_available(prize_only_k1),
         "Full Prize inspection K1 did not admit the route.");

  sim::Engine k0 = make_engine(scenario, rng, base_state(), false, false);
  expect(!sim::EngineTestAccess::route_available(k0),
         "K0 admitted the public-K1 route.");
}

void test_lock_resource_and_target_boundaries() {
  std::mt19937_64 rng(241);

  sim::Engine item_locked =
      make_engine(strict(sim::LockMode::FullItem), rng, base_state());
  expect(!sim::EngineTestAccess::route_available(item_locked),
         "Item lock admitted the route.");

  sim::Engine supporter_locked =
      make_engine(strict(sim::LockMode::FullSupporter), rng, base_state());
  expect(!sim::EngineTestAccess::route_available(supporter_locked),
         "Supporter lock admitted Crispin.");

  sim::Engine ability_locked =
      make_engine(strict(sim::LockMode::FullRuleBoxAbility), rng, base_state());
  expect(!sim::EngineTestAccess::route_available(ability_locked),
         "Rule Box Ability lock admitted Wonder Tag.");

  sim::State full_bench = base_state();
  full_bench.bench.assign(5, sim::Pokemon{sim::Card::RegidragoV, 1});
  sim::Engine benched = make_engine(strict(), rng, std::move(full_bench));
  expect(!sim::EngineTestAccess::route_available(benched),
         "A full Bench admitted the Tapu route.");

  sim::State insufficient_grass = base_state();
  insufficient_grass.deck.erase(std::find(insufficient_grass.deck.begin(),
                                           insufficient_grass.deck.end(),
                                           sim::Card::Grass));
  sim::Engine low_grass =
      make_engine(strict(), rng, std::move(insufficient_grass));
  expect(!sim::EngineTestAccess::route_available(low_grass),
         "Too few searchable Grass Energy admitted the route.");

  sim::State missing_fire = base_state();
  missing_fire.deck.erase(std::find(missing_fire.deck.begin(),
                                    missing_fire.deck.end(), sim::Card::Fire));
  sim::Engine no_fire = make_engine(strict(), rng, std::move(missing_fire));
  expect(!sim::EngineTestAccess::route_available(no_fire),
         "Crispin was admitted without a searchable Fire Energy.");

  sim::State no_safe_vessel_cost = base_state();
  no_safe_vessel_cost.hand.erase(
      std::find(no_safe_vessel_cost.hand.begin(),
                no_safe_vessel_cost.hand.end(), sim::Card::StevensResolve));
  sim::Engine no_cost =
      make_engine(strict(), rng, std::move(no_safe_vessel_cost));
  expect(!sim::EngineTestAccess::route_available(no_cost),
         "The route was admitted without a route-replaced Vessel cost.");

  sim::State no_payload = base_state();
  no_payload.hand.erase(std::find(no_payload.hand.begin(), no_payload.hand.end(),
                                  sim::Card::Dragapult));
  sim::Engine missing_payload =
      make_engine(strict(), rng, std::move(no_payload));
  expect(!sim::EngineTestAccess::route_available(missing_payload),
         "The route was admitted without a held Dragon payload.");
}

void test_registered_seed_241_reaches_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1878 fixture is unavailable.");

  std::mt19937_64 rng(241);
  sim::TraceLog trace{true, {}, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  expect(outcome.first_ready_turn == 3,
         "Registered seed 241 did not improve from T4 to T3.");
  expect(trace_contains(trace, "Earthen Vessel") &&
             trace_contains(trace, "Quick Ball") &&
             trace_contains(trace, "WONDER TAG") &&
             trace_contains(trace, "Crispin") &&
             trace_contains(trace, "T3 | READY"),
         "Registered seed 241 did not execute the complete T3 route.");
}
}  // namespace

int main() {
  try {
    test_exact_route_and_knowledge_boundaries();
    test_lock_resource_and_target_boundaries();
    test_registered_seed_241_reaches_t3();
    std::cout << "Issue 1878 Vessel-Quick Ball-Tapu-Crispin tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
