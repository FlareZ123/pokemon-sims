#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void establish_deck_k1(Engine& engine) {
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = false;
  }
  static void establish_prize_k1(Engine& engine) {
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = true;
  }
  static void clear_k1(Engine& engine) {
    engine.deck_seen_ = false;
    engine.prizes_revealed_ = false;
  }
  static bool prizes_known(const Engine& engine) {
    return engine.prizes_known();
  }
  static bool route_without_backup(Engine& engine) {
    return engine.issue_1749_existing_regi_ready_route_without_backup();
  }
  static void play_basics(Engine& engine) { engine.play_basics_from_hand(); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine(sim::Scenario& scenario, sim::DeckRecipe& recipe,
                        std::mt19937_64& rng) {
  return sim::Engine(scenario, recipe, rng);
}

void install_complete_route(sim::Engine& engine) {
  auto& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0,
                              sim::Tool::ForestSealStone};
  state.bench = {sim::Pokemon{sim::Card::Oricorio, 1},
                 sim::Pokemon{sim::Card::LatiasEx, 1}};
  state.hand = {sim::Card::Gladion, sim::Card::Fire,
                sim::Card::RegidragoVstar, sim::Card::RegidragoV};
  state.deck = {sim::Card::BrilliantBlender, sim::Card::MegaDragonite,
                sim::Card::Dragapult};
  sim::EngineTestAccess::establish_deck_k1(engine);
}

bool exact_route_with(const auto& mutate) {
  sim::Scenario scenario{"issue-1749-unit", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1749};
  sim::Engine engine = make_engine(scenario, recipe, rng);
  install_complete_route(engine);
  mutate(engine, sim::EngineTestAccess::state(engine));
  return sim::EngineTestAccess::route_without_backup(engine);
}

void expect_complete_route_preserves_backup(sim::Engine& engine,
                                            const char* route_message) {
  expect(sim::EngineTestAccess::route_without_backup(engine), route_message);
  sim::EngineTestAccess::play_basics(engine);
  const auto& state = sim::EngineTestAccess::state(engine);
  expect(std::count(state.hand.begin(), state.hand.end(), sim::Card::RegidragoV) == 1,
         "The complete route still spent the held backup Regidrago V.");
  expect(std::count_if(state.bench.begin(), state.bench.end(), [](const sim::Pokemon& pokemon) {
           return pokemon.card == sim::Card::RegidragoV;
         }) == 0,
         "The complete route still added a redundant Benched Regidrago V.");
}

void test_deck_search_k1_suppresses_backup() {
  sim::Scenario scenario{"issue-1749-unit", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1749};
  sim::Engine engine = make_engine(scenario, recipe, rng);
  install_complete_route(engine);

  // The existing Active Regidrago V already has a K1-complete route through
  // Star Alchemy, the held Fire Energy, Regidrago VSTAR, and Brilliant Blender.
  // Keeping the second Regidrago V in hand preserves one card and one Bench slot:
  // Regidrago V / Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Official Bench, attachment, evolution, Item, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Repository K1 and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original route bug: https://github.com/FlareZ123/pokemon-sims/issues/1749
  expect(sim::EngineTestAccess::prizes_known(engine),
         "Deck inspection did not establish K1.");
  expect_complete_route_preserves_backup(
      engine, "The deck-search K1 completion route was not recognized.");
}

void test_prize_inspection_k1_suppresses_backup() {
  sim::Scenario scenario{"issue-2068-prize-k1", sim::DciProfile::MatchupFlexJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{2068};
  sim::Engine engine = make_engine(scenario, recipe, rng);
  install_complete_route(engine);
  sim::EngineTestAccess::establish_prize_k1(engine);

  // Hisuian Heavy Ball exposes the complete face-down Prize set. With a fixed
  // deck list, that proves the same remaining Blender and Dragon inventory as a
  // resolved deck search, while the board and hand already complete every axis:
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Regidrago V / Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Forest Seal Stone / Star Alchemy: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Repository K1 and resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed provenance bug: https://github.com/FlareZ123/pokemon-sims/issues/2068
  expect(sim::EngineTestAccess::prizes_known(engine),
         "Complete Prize inspection did not establish K1.");
  expect_complete_route_preserves_backup(
      engine, "The Prize-inspection K1 completion route was not recognized.");
}

void test_post_attachment_and_post_search_route_stays_suppressed() {
  expect(exact_route_with([](sim::Engine&, sim::State& state) {
           state.vstar_power_used = true;
           state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                        sim::Card::BrilliantBlender), state.deck.end());
           state.hand.push_back(sim::Card::BrilliantBlender);
           state.manual_energy_used = true;
           state.active->fire = 1;
           state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Fire));
         }),
         "The route reopened redundant placement after Star Alchemy and attachment.");
}

void test_incomplete_routes_keep_backup_placement_available() {
  expect(!exact_route_with([](sim::Engine& engine, sim::State&) {
           sim::EngineTestAccess::clear_k1(engine);
         }), "K0 incorrectly suppressed backup Regidrago placement.");
  expect(!exact_route_with([](sim::Engine&, sim::State& state) {
           state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                        sim::Card::BrilliantBlender), state.deck.end());
         }), "A missing Blender route incorrectly suppressed backup placement.");
  expect(!exact_route_with([](sim::Engine&, sim::State& state) {
           state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::Fire));
         }), "A missing final Energy incorrectly suppressed backup placement.");
  expect(!exact_route_with([](sim::Engine&, sim::State& state) {
           state.active->entered_turn = state.turn;
         }), "A newly played Active incorrectly projected same-turn evolution.");
  expect(!exact_route_with([](sim::Engine&, sim::State& state) {
           state.manual_energy_used = true;
         }), "An exhausted attachment incorrectly completed a missing Energy axis.");
}

void test_seed_20260728_preserves_the_redundant_regidrago() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  const auto* deck = sim::crobat_modeling_deck_by_id("crobat1-heavy-ball");
  expect(scenario.has_value() && deck != nullptr,
         "Issue-1749 source-bound seed fixture unavailable.");
  std::mt19937_64 rng{20260728};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  expect(outcome.first_ready_turn == 2,
         "Seed 20260728 no longer reaches the established T2 ready state.");
  expect(std::none_of(trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
           return line.find("T2 | BENCH | rules: R-GAME-BENCH | Regidrago V from hand") !=
               std::string::npos;
         }), "Seed 20260728 still benches the redundant second Regidrago V.");
  expect(std::any_of(trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
           return line.find("T2 | READY") != std::string::npos;
         }), "Seed 20260728 lost its legal T2 completion.");
}
}  // namespace

int main() {
  test_deck_search_k1_suppresses_backup();
  test_prize_inspection_k1_suppresses_backup();
  test_post_attachment_and_post_search_route_stays_suppressed();
  test_incomplete_routes_keep_backup_placement_available();
  test_seed_20260728_preserves_the_redundant_regidrago();
}
