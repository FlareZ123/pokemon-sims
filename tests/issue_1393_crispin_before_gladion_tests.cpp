#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state_with_knowledge(Engine& engine, State state,
                                       const bool deck_seen,
                                       const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static void set_state(Engine& engine, State state) {
    set_state_with_knowledge(engine, std::move(state), true, true);
  }
  static void choose_supporter(Engine& engine) {
    engine.choose_supporter_issue_1152();
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1393_held_crispin_completion_available();
  }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static const State& state(const Engine& engine) { return engine.state_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State exact_state(const bool include_payload_outlet = true,
                       const bool include_fire = true) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::ForestSealStone};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None},
                 sim::Pokemon{sim::Card::RegidragoV, 2, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::QuickBall, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::Gladion,
                sim::Card::MegaDragonite};
  if (include_payload_outlet) {
    state.hand.push_back(sim::Card::BrilliantBlender);
  }
  state.deck = {sim::Card::Grass, sim::Card::Oricorio,
                sim::Card::Dragapult, sim::Card::DialgaGX};
  if (include_fire) state.deck.push_back(sim::Card::Fire);
  state.prizes = {sim::Card::GoodraVstar};
  return state;
}

void test_seed_61_reaches_turn_two() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1393 seed fixture is unavailable.");
  std::mt19937_64 rng{61};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  if (outcome.first_ready_turn != 2) {
    for (const auto& line : trace.lines) std::cerr << line << '\n';
  }
  expect(outcome.first_ready_turn == 2,
         "Seed 61 must use held Crispin and reach readiness on turn two.");
}

void test_exact_route_holds_fss_and_plays_crispin() {
  const sim::Scenario scenario{"issue-1393", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1393};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_state());

  // Held Crispin plus its unused manual attachment completes GGF, while Brilliant
  // Blender supplies the same-turn Dragon payload. Forest Seal Stone must remain
  // unused and Gladion must remain in hand:
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1393
  expect(sim::EngineTestAccess::route_available(engine),
         "Exact state must prove the held Crispin completion route.");
  expect(!sim::EngineTestAccess::use_fss(engine),
         "Star Alchemy must hold when held Crispin already proves the T2 route.");
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(result.supporter_used, "Crispin must consume the Supporter play.");
  expect(std::find(result.discard.begin(), result.discard.end(),
                   sim::Card::Crispin) != result.discard.end(),
         "Crispin must be the Supporter played.");
  expect(std::find(result.hand.begin(), result.hand.end(),
                   sim::Card::Gladion) != result.hand.end(),
         "Gladion must remain in hand when it does not advance the earliest route.");
}

void test_issue_2768_shared_jit_profiles() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  for (const sim::DciProfile profile : {sim::DciProfile::StrictJit,
                                        sim::DciProfile::MatchupFlexJit}) {
    const sim::Scenario scenario{"issue-2768-jit", profile,
                                 sim::LockMode::None, false, 4};
    std::mt19937_64 rng{2768};
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_state(engine, exact_state());
    // Both registered current-turn JIT profiles have the same payload timing.
    // Crispin completes GGF while held Brilliant Blender supplies the payload;
    // Gladion remains the slower competing Supporter.
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Shared JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2768
    expect(sim::EngineTestAccess::route_available(engine),
           "Both current-turn JIT profiles must admit the held-Crispin route.");
  }

  const sim::Scenario control{"issue-2768-control",
                              sim::DciProfile::NoDiscardControl,
                              sim::LockMode::None, false, 4};
  std::mt19937_64 control_rng{2769};
  sim::Engine control_engine(control, recipe, control_rng);
  sim::EngineTestAccess::set_state(control_engine, exact_state());
  expect(!sim::EngineTestAccess::route_available(control_engine),
         "NoDiscardControl must remain outside the current-turn JIT route.");
}

void test_prize_inspection_alone_establishes_k1() {
  const sim::Scenario scenario{"issue-1918", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1918};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);

  // Gladion or Hisuian Heavy Ball exposes the complete Prize zone. The fixed
  // recipe and public zones then establish K1 without a separate deck search:
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1918
  sim::EngineTestAccess::set_state_with_knowledge(
      engine, exact_state(), false, true);
  expect(sim::EngineTestAccess::route_available(engine),
         "Full Prize inspection alone must permit the K1 Crispin route.");

  sim::EngineTestAccess::set_state_with_knowledge(
      engine, exact_state(), false, false);
  expect(!sim::EngineTestAccess::route_available(engine),
         "The route must remain unavailable at K0.");
}

void test_missing_fire_preserves_gladion_fallback() {
  const sim::Scenario scenario{"issue-1393-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  std::mt19937_64 rng{1394};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, exact_state(true, false));
  sim::EngineTestAccess::choose_supporter(engine);
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(std::find(result.hand.begin(), result.hand.end(),
                   sim::Card::GoodraVstar) != result.hand.end(),
         "Gladion must remain the fallback when Crispin cannot complete GGF.");
  expect(std::find(result.prizes.begin(), result.prizes.end(),
                   sim::Card::Gladion) != result.prizes.end(),
         "Gladion must exchange itself into the Prize cards in the fallback control.");
}
}  // namespace

int main() {
  try {
    test_exact_route_holds_fss_and_plays_crispin();
    test_issue_2768_shared_jit_profiles();
    test_prize_inspection_alone_establishes_k1();
    test_seed_61_reaches_turn_two();
    test_missing_fire_preserves_gladion_fallback();
    std::cout << "Issue 1393 Crispin-before-Gladion tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
