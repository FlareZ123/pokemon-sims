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
  static void set_state(Engine& engine, State state, const bool deck_seen) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static bool play_steven_bank(Engine& engine) {
    return engine.play_steven_for_secret_box_bank();
  }
  static bool advance_forretress(Engine& engine) {
    return engine.advance_forretress_combo();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

void test_seed35_uses_direct_t2_route() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario.has_value() && deck != nullptr,
         "The registered seed-35 fixture is unavailable.");

  std::mt19937_64 rng(35);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Steven may search up to three cards. Crispin plus the manual attachment
  // completes GGF, and Mysterious Treasure legally discards the searched Dragon
  // while retaining another legal target. The same T2 deadline must preserve
  // Secret Box and the Forretress ex self-Knock-Out:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#secret-box-and-pineco-route-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/1420
  expect(outcome.first_ready_turn == 2,
         "Seed 35 lost strict-JIT T2 readiness.");
  expect(trace_contains(trace, "banked Crispin and Dragapult ex"),
         "Steven did not choose the direct Treasure route.");
  expect(trace_contains(trace, "Dragapult ex (Mysterious Treasure cost)"),
         "Mysterious Treasure did not establish the T2 payload.");
  expect(!trace_contains(trace, "T2 | PLAY ITEM | rules: R-SECRET-BOX-01"),
         "The direct route still spent Secret Box.");
  expect(!trace_contains(trace, "T2 | EXPLODING ENERGY"),
         "The direct route still used the Forretress self-Knock-Out.");
  expect(!trace_contains(trace,
                         "T2 | EVOLVE | rules: R-FORRETRESS-01"),
         "The direct route still spent Forretress ex.");
  expect(!trace_contains(trace,
                         "T2 | BENCH | rules: R-GAME-BENCH | Regidrago V from hand"),
         "The direct route still spent the spare Regidrago V and Bench slot.");

  const sim::State& final_state = engine.state();
  expect(final_state.bench.size() == 2 &&
             final_state.bench[0].card == sim::Card::TapuLeleGX &&
             final_state.bench[1].card == sim::Card::Pineco,
         "The direct route did not preserve the original two-Pokemon Bench.");
  expect(contains(final_state.hand, sim::Card::ForretressEx) &&
             contains(final_state.hand, sim::Card::RegidragoV),
         "The direct route did not preserve Forretress ex and the spare Regidrago V.");
}

sim::State direct_route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.hand = {
      sim::Card::StevensResolve,
      sim::Card::MysteriousTreasure,
      sim::Card::RegidragoVstar,
      sim::Card::Fire,
      sim::Card::Pineco,
      sim::Card::ForretressEx,
  };
  state.deck = {
      sim::Card::SecretBox,
      sim::Card::Crispin,
      sim::Card::Dragapult,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::RegidragoV,
      sim::Card::MegaDragonite,
  };
  return state;
}

void test_steven_search_establishes_k1_and_requires_every_axis() {
  const sim::Scenario scenario{"issue-1420-controls", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  const sim::DeckRecipe recipe = sim::pineco_recipe();

  std::mt19937_64 inspect_rng(142001);
  sim::Engine inspect_engine(scenario, recipe, inspect_rng);
  sim::EngineTestAccess::set_state(inspect_engine, direct_route_state(), false);
  expect(sim::EngineTestAccess::play_steven_bank(inspect_engine),
         "Steven's legal deck inspection did not resolve.");
  expect(contains(sim::EngineTestAccess::state(inspect_engine).hand,
                  sim::Card::Crispin) &&
             contains(sim::EngineTestAccess::state(inspect_engine).hand,
                      sim::Card::Dragapult) &&
             !contains(sim::EngineTestAccess::state(inspect_engine).hand,
                       sim::Card::SecretBox),
         "Steven did not use its own deck inspection to confirm the direct route.");

  const auto expect_fallback = [&scenario, &recipe](
      sim::State state, const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng(seed);
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    expect(sim::EngineTestAccess::play_steven_bank(engine),
           "A guarded Secret Box fallback did not resolve.");
    expect(contains(sim::EngineTestAccess::state(engine).hand,
                    sim::Card::SecretBox),
           message);
  };

  sim::State missing_treasure = direct_route_state();
  missing_treasure.hand.erase(std::find(
      missing_treasure.hand.begin(), missing_treasure.hand.end(),
      sim::Card::MysteriousTreasure));
  expect_fallback(std::move(missing_treasure), 142002,
                  "Missing Treasure still selected the direct route.");

  sim::State missing_vstar = direct_route_state();
  missing_vstar.hand.erase(std::find(missing_vstar.hand.begin(),
                                     missing_vstar.hand.end(),
                                     sim::Card::RegidragoVstar));
  expect_fallback(std::move(missing_vstar), 142003,
                  "Missing VSTAR still selected the direct route.");

  sim::State missing_fire = direct_route_state();
  missing_fire.hand.erase(std::find(missing_fire.hand.begin(),
                                    missing_fire.hand.end(), sim::Card::Fire));
  expect_fallback(std::move(missing_fire), 142004,
                  "Missing held Fire still selected the direct route.");

  sim::State missing_crispin = direct_route_state();
  missing_crispin.deck.erase(std::find(missing_crispin.deck.begin(),
                                       missing_crispin.deck.end(),
                                       sim::Card::Crispin));
  expect_fallback(std::move(missing_crispin), 142005,
                  "Missing Crispin still selected the direct route.");

  sim::State missing_payload = direct_route_state();
  missing_payload.deck.erase(std::find(missing_payload.deck.begin(),
                                       missing_payload.deck.end(),
                                       sim::Card::Dragapult));
  expect_fallback(std::move(missing_payload), 142006,
                  "Missing Dragapult still selected the direct route.");

  sim::State no_remaining_target = direct_route_state();
  no_remaining_target.deck.erase(std::find(no_remaining_target.deck.begin(),
                                            no_remaining_target.deck.end(),
                                            sim::Card::MegaDragonite));
  no_remaining_target.deck.erase(std::find(no_remaining_target.deck.begin(),
                                            no_remaining_target.deck.end(),
                                            sim::Card::RegidragoV));
  expect_fallback(std::move(no_remaining_target), 142007,
                  "Treasure with no post-cost target selected the direct route.");

  sim::State missing_prior_grass = direct_route_state();
  missing_prior_grass.active->grass = 0;
  expect_fallback(std::move(missing_prior_grass), 142008,
                  "Missing prior Grass still selected the direct route.");
}

void test_item_lock_preserves_forretress_fallback() {
  const sim::Scenario scenario{"issue-1420-item-lock",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::TurnTwoItem, false, 5};
  const sim::DeckRecipe recipe = sim::pineco_recipe();
  std::mt19937_64 rng(142003);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = direct_route_state();
  sim::EngineTestAccess::set_state(engine, std::move(state), true);

  expect(sim::EngineTestAccess::play_steven_bank(engine),
         "The scheduled-lock Secret Box fallback did not resolve.");
  expect(contains(sim::EngineTestAccess::state(engine).hand,
                  sim::Card::SecretBox),
         "Scheduled Item lock selected an unusable Treasure route.");
}

}  // namespace

int main() {
  try {
    test_seed35_uses_direct_t2_route();
    test_steven_search_establishes_k1_and_requires_every_axis();
    test_item_lock_preserves_forretress_fallback();
    std::cout << "Issue 1420 Steven direct-Treasure tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
