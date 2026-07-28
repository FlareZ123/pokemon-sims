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
  }
  static bool crobat_pending(Engine& engine) {
    return engine.issue_1751_crobat_pending();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

std::size_t trace_index(const sim::TraceLog& trace,
                        const std::string& needle) {
  const auto found = std::find_if(
      trace.lines.begin(), trace.lines.end(),
      [&needle](const std::string& line) {
        return line.find(needle) != std::string::npos;
      });
  if (found == trace.lines.end()) {
    throw std::runtime_error("Expected issue-1751 trace line missing: " + needle);
  }
  return static_cast<std::size_t>(std::distance(trace.lines.begin(), found));
}

sim::CrobatModelingDeck serena_swap_recipe() {
  // The requested experimental list changes only Serena into Crobat V. Keeping it
  // outside the registered modeling list preserves the repository's public variant
  // surface while making the regression fully reproducible:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1751
  return sim::make_crobat_modeling_deck("issue-1751-crobat1-serena",
                                        {sim::Card::Serena});
}

void test_seed_2154_compresses_before_crobat_and_reaches_t2() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::CrobatModelingDeck deck = serena_swap_recipe();
  expect(scenario.has_value(), "Issue-1751 seed-2154 scenario unavailable.");

  std::mt19937_64 rng{2154};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck.recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Search and Bench actions that deterministically reduce the hand must precede
  // Crobat V. Dark Asset then draws three before Steven's Resolve ends the turn:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Official action-order procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1751
  const std::size_t heavy_ball = trace_index(trace, "T1 | PLAY ITEM | rules: R-HEAVYBALL-01");
  const std::size_t quick_ball = trace_index(trace, "T1 | PLAY ITEM | rules: R-QB-01");
  const std::size_t tapu = trace_index(trace, "T1 | BENCH | rules: R-GAME-BENCH | Tapu Lele-GX");
  const std::size_t crobat = trace_index(trace, "T1 | BENCH | rules: R-GAME-BENCH | Crobat V");
  const std::size_t dark_asset = trace_index(trace, "T1 | DARK ASSET");
  const std::size_t steven = trace_index(trace, "T1 | PLAY SUPPORTER | rules: R-STEVEN-01");
  expect(heavy_ball < quick_ball && quick_ball < tapu && tapu < crobat &&
             crobat < dark_asset && dark_asset < steven,
         "Seed 2154 did not exhaust deterministic compression before Dark Asset.");
  expect(trace_index(trace, "Drew 3 card(s) until the hand contained six cards") <
             trace.lines.size(),
         "Seed 2154 did not gain the three-card Dark Asset draw.");
  expect(outcome.first_ready_turn == 2 && outcome.ready_by_2,
         "Seed 2154 did not convert the prior setup failure into T2 readiness.");
}

void test_seed_2014_uses_arven_blender_before_crobat() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::CrobatModelingDeck deck = serena_swap_recipe();
  expect(scenario.has_value(), "Issue-1751 seed-2014 scenario unavailable.");

  std::mt19937_64 rng{2014};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck.recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The non-ending Arven line may finish its attachment, Tool, and Blender actions
  // before Crobat V. Star Alchemy remains after Dark Asset because searching a card
  // would inflate the pre-draw hand:
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Crobat V: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1751
  const std::size_t attach = trace_index(trace, "T3 | ATTACH | rules: R-GAME-ENERGY");
  const std::size_t arven = trace_index(trace, "T3 | PLAY SUPPORTER | rules: R-ARVEN-01");
  const std::size_t stone = trace_index(trace, "T3 | ATTACH TOOL");
  const std::size_t blender = trace_index(trace, "T3 | PLAY ITEM | rules: R-BLENDER-01");
  const std::size_t crobat = trace_index(trace, "T3 | BENCH | rules: R-GAME-BENCH | Crobat V");
  const std::size_t dark_asset = trace_index(trace, "T3 | DARK ASSET");
  const std::size_t star = trace_index(trace, "T3 | STAR ALCHEMY");
  expect(attach < arven && arven < stone && stone < blender &&
             blender < crobat && crobat < dark_asset && dark_asset < star,
         "Seed 2014 lost the compression-first Arven/Blender/Crobat order.");
  expect(trace_index(trace, "Drew 3 card(s) until the hand contained six cards") <
             trace.lines.size(),
         "Seed 2014 did not draw three with Dark Asset.");
  expect(outcome.first_ready_turn == 3 && outcome.ready_by_3,
         "Seed 2014 lost its T3 readiness deadline.");
}

bool pending_with(const sim::LockMode lock, const auto& mutate) {
  sim::Scenario scenario{"issue-1751-unit", sim::DciProfile::StrictJit,
                         lock, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1751};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::CrobatV, sim::Card::Grass};
  state.deck = {sim::Card::Fire, sim::Card::RegidragoVstar,
                sim::Card::MegaDragonite};
  mutate(state);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::crobat_pending(engine);
}

void test_pending_window_negative_controls() {
  expect(pending_with(sim::LockMode::None, [](sim::State&) {}),
         "The live Crobat compression window was rejected.");
  expect(!pending_with(sim::LockMode::FullRuleBoxAbility, [](sim::State&) {}),
         "The Crobat window ignored Rule Box Ability lock.");
  expect(!pending_with(sim::LockMode::None, [](sim::State& state) {
           state.dark_asset_used = true;
         }), "The Crobat window ignored the once-per-turn Ability limit.");
  expect(!pending_with(sim::LockMode::None, [](sim::State& state) {
           state.deck.clear();
         }), "The Crobat window ignored an empty deck.");
  expect(!pending_with(sim::LockMode::None, [](sim::State& state) {
           while (state.bench.size() < 5U) {
             state.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1, 0, 0,
                                                sim::Tool::None});
           }
         }), "The Crobat window ignored a full Bench.");
  expect(!pending_with(sim::LockMode::None, [](sim::State& state) {
           state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                                       sim::Tool::None};
           state.discard.push_back(sim::Card::MegaDragonite);
           state.discarded_this_turn.push_back(sim::Card::MegaDragonite);
         }), "The Crobat window ran after every setup axis was complete.");
}
}  // namespace

int main() {
  test_seed_2154_compresses_before_crobat_and_reaches_t2();
  test_seed_2014_uses_arven_blender_before_crobat();
  test_pending_window_negative_controls();
}
