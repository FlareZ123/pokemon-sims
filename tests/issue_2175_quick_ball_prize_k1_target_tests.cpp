#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_knowledge(Engine& engine, const bool deck_seen,
                            const bool prizes_revealed) {
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool play_quick_ball_for_payload(Engine& engine) {
    return engine.play_quick_ball(true);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State live_payload_state(std::vector<sim::Card> deck) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::Dragapult};
  state.deck = std::move(deck);
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const char* label, const std::uint64_t seed)
      : scenario{label, sim::DciProfile::StrictJit, sim::LockMode::None, false, 4},
        recipe(sim::baseline_recipe()),
        rng(seed),
        engine(scenario, recipe, rng) {}
};

void expect_live_quick_ball(sim::Engine& engine, const char* message) {
  if (!sim::EngineTestAccess::play_quick_ball_for_payload(engine)) {
    throw std::runtime_error(message);
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.discard, sim::Card::Dragapult) ||
      !contains(after.discarded_this_turn, sim::Card::Dragapult) ||
      !sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error(
        "Quick Ball lost the strict-JIT Dragon payload after a legal search.");
  }
}

void test_prize_inspection_k1_uses_exact_live_basic() {
  Fixture fixture{"issue-2175-prize-k1-live-basic", 217501};
  sim::Engine& engine = fixture.engine;
  sim::State state = live_payload_state({sim::Card::MawileGX});
  state.prizes = {sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(engine, false, true);

  // Complete Prize inspection establishes K1, so the complementary deck is known.
  // Quick Ball may pay Dragapult ex and search the physically present Basic Mawile-GX:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/sm11-141
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/2175
  expect_live_quick_ball(
      engine, "Prize-inspection K1 rejected a known live Basic target.");
}

void test_deck_search_k1_keeps_exact_live_basic() {
  Fixture fixture{"issue-2175-deck-k1-live-basic", 217502};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(
      engine, live_payload_state({sim::Card::MawileGX}));
  sim::EngineTestAccess::set_knowledge(engine, true, false);

  // Deck-search K1 continues to inspect the real deck for Quick Ball's Basic target:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm11-141
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/2175
  expect_live_quick_ball(engine,
                         "Deck-search K1 rejected a known live Basic target.");
}

void test_k1_rejects_known_empty_basic_axis() {
  Fixture fixture{"issue-2175-k1-no-basic", 217503};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(
      engine, live_payload_state({sim::Card::Grass}));
  sim::EngineTestAccess::set_knowledge(engine, false, true);

  // A Trainer cannot be played when K1 proves its search will have no effect.
  // Search-card legality ruling:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/2175
  if (sim::EngineTestAccess::play_quick_ball_for_payload(engine)) {
    throw std::runtime_error(
        "Prize-inspection K1 allowed Quick Ball with no Basic in deck.");
  }
}

void test_true_k0_retains_recipe_plausibility() {
  Fixture fixture{"issue-2175-true-k0", 217504};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(
      engine, live_payload_state({sim::Card::MawileGX}));
  sim::EngineTestAccess::set_knowledge(engine, false, false);

  // Before legal inspection, the simulator keeps fixed-list Basic targets plausible:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/2175
  expect_live_quick_ball(engine,
                         "True K0 lost the documented recipe-plausibility path.");
}

}  // namespace

int main() {
  test_prize_inspection_k1_uses_exact_live_basic();
  test_deck_search_k1_keeps_exact_live_basic();
  test_k1_rejects_known_empty_basic_axis();
  test_true_k0_retains_recipe_plausibility();
  return 0;
}
