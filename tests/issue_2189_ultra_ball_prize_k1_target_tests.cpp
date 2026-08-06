#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void set_knowledge(Engine& engine, const bool deck_seen,
                            const bool prizes_revealed) {
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool ultra_ball_has_legal_target(const Engine& engine) {
    return engine.ultra_ball_has_legal_target();
  }
};

}  // namespace sim

namespace {

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

sim::State target_state(std::vector<sim::Card> deck) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::UltraBall, sim::Card::Dragapult, sim::Card::Fire};
  state.deck = std::move(deck);
  state.prizes = {sim::Card::Dipplin};
  return state;
}

void test_prize_inspection_k1_uses_exact_live_pokemon() {
  Fixture fixture{"issue-2189-prize-k1-live-pokemon", 218901};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(engine, target_state({sim::Card::Appletun}));
  sim::EngineTestAccess::set_knowledge(engine, false, true);

  // Complete Prize inspection establishes K1. Ultra Ball may search any exact
  // physical Pokémon, including an Appletun absent from the selected recipe:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/2189
  if (!sim::EngineTestAccess::ultra_ball_has_legal_target(engine)) {
    throw std::runtime_error(
        "Prize-inspection K1 rejected a known live Pokémon target.");
  }
}

void test_deck_search_k1_keeps_exact_live_pokemon() {
  Fixture fixture{"issue-2189-deck-k1-live-pokemon", 218902};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(engine, target_state({sim::Card::Appletun}));
  sim::EngineTestAccess::set_knowledge(engine, true, false);

  // Deck-search K1 continues to inspect the exact deck for any Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/2189
  if (!sim::EngineTestAccess::ultra_ball_has_legal_target(engine)) {
    throw std::runtime_error(
        "Deck-search K1 rejected a known live Pokémon target.");
  }
}

void test_k1_rejects_known_empty_pokemon_axis() {
  Fixture fixture{"issue-2189-k1-no-pokemon", 218903};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(engine, target_state({sim::Card::Grass}));
  sim::EngineTestAccess::set_knowledge(engine, false, true);

  // K1 cannot pay Ultra Ball when the exact deck contains no Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/2189
  if (sim::EngineTestAccess::ultra_ball_has_legal_target(engine)) {
    throw std::runtime_error(
        "Prize-inspection K1 allowed Ultra Ball with no Pokémon target.");
  }
}

void test_true_k0_retains_recipe_plausibility() {
  Fixture fixture{"issue-2189-true-k0", 218904};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(engine, target_state({sim::Card::Grass}));
  sim::EngineTestAccess::set_knowledge(engine, false, false);

  // Before legal inspection, unaccounted fixed-list Pokémon remain plausible:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/2189
  if (!sim::EngineTestAccess::ultra_ball_has_legal_target(engine)) {
    throw std::runtime_error("True K0 lost fixed-list target plausibility.");
  }
}

}  // namespace

int main() {
  test_prize_inspection_k1_uses_exact_live_pokemon();
  test_deck_search_k1_keeps_exact_live_pokemon();
  test_k1_rejects_known_empty_pokemon_axis();
  test_true_k0_retains_recipe_plausibility();
  return 0;
}
