#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void install_committed_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
    engine.issue_4391_arven_blender_fss_commit_turn_ = engine.state_.turn;
  }

  static bool run_search_step(Engine& engine) {
    return engine.run_search_items_one_step(true);
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

void committed_blender_preempts_quick_ball() {
  const sim::Scenario scenario{"issue-4391-connector-dominance",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{4391};
  sim::Engine engine{scenario, recipe, rng};

  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {
      sim::Card::BrilliantBlender,
      sim::Card::QuickBall,
      sim::Card::Dragapult,
      sim::Card::ChaoticSwell,
  };
  state.deck = {
      sim::Card::Dragapult,
      sim::Card::GoodraVstar,
      sim::Card::RegidragoV,
  };
  state.supporter_used = true;
  state.vstar_power_used = true;
  sim::EngineTestAccess::install_committed_state(engine, std::move(state));

  // Arven has already committed the turn's Item/Tool search to Brilliant Blender
  // plus Forest Seal Stone, and Star Alchemy has supplied the VSTAR. With payload
  // now the exact missing axis, Blender searches the deck payload directly. Quick
  // Ball would pay an extra held-Dragon discard for the same ready turn.
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Connector priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/4391
  expect(sim::EngineTestAccess::run_search_step(engine),
         "The committed search step did not execute.");
  const sim::State& result = sim::EngineTestAccess::state(engine);
  expect(contains(result.discard, sim::Card::BrilliantBlender),
         "Brilliant Blender was not used for the committed payload channel.");
  expect(contains(result.discard, sim::Card::Dragapult),
         "Brilliant Blender did not discard the deck-resident Dragapult.");
  expect(contains(result.hand, sim::Card::Dragapult),
         "The held Dragon was consumed by a dominated paid-search line.");
  expect(contains(result.hand, sim::Card::QuickBall),
         "Quick Ball was consumed despite the committed Blender connector.");
}

void exact_seed_42_uses_blender_after_star_alchemy() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  expect(scenario.has_value(), "Missing strict-jit/go-second scenario.");

  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{42};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*scenario, recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();

  // This is the original CI witness. Arven selects Blender + Forest Seal Stone,
  // Star Alchemy selects Regidrago VSTAR, and the corrected selector must use the
  // committed Blender channel instead of discarding the held Dragapult to Quick Ball.
  // Evidence issue: https://github.com/FlareZ123/pokemon-sims/issues/4391
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  expect(outcome.first_ready_turn == 3,
         "Seed 42 no longer reaches the established T3 ready window.");
  expect(trace_contains(trace, "T3 | PLAY ITEM") &&
             trace_contains(trace, "R-BLENDER-01"),
         "Seed 42 did not use Brilliant Blender on the committed T3 route.");
  expect(!trace_contains(trace, "Dragapult ex (Quick Ball cost)"),
         "Seed 42 still discarded the held Dragapult to Quick Ball.");
}

}  // namespace

int main() {
  try {
    committed_blender_preempts_quick_ball();
    exact_seed_42_uses_blender_after_star_alchemy();
    std::cout << "Issue 4391 Blender connector-dominance tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
