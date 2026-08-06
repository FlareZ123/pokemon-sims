#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
  static bool play_mysterious_treasure_for_payload(Engine& engine) {
    return engine.play_mysterious_treasure(true);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
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

sim::State live_payload_state(std::vector<sim::Card> deck) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dragapult};
  state.deck = std::move(deck);
  state.prizes = {sim::Card::Dipplin};
  return state;
}

void expect_live_appletun_search(sim::Engine& engine, const char* message) {
  if (!sim::EngineTestAccess::play_mysterious_treasure_for_payload(engine)) {
    throw std::runtime_error(message);
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Appletun) ||
      !contains(after.discard, sim::Card::MysteriousTreasure) ||
      !contains(after.discard, sim::Card::Dragapult) ||
      !contains(after.discarded_this_turn, sim::Card::Dragapult) ||
      !after.deck.empty() || !sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error(
        "Mysterious Treasure lost its legal Appletun search or strict-JIT cost.");
  }
}

void test_prize_inspection_k1_uses_exact_live_dragon() {
  Fixture fixture{"issue-2188-prize-k1-live-dragon", 218801};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(
      engine, live_payload_state({sim::Card::Appletun}));
  sim::EngineTestAccess::set_knowledge(engine, false, true);

  // Complete Prize inspection establishes K1. Mysterious Treasure may discard
  // Dragapult ex and search the exact physical Appletun Dragon target:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/2188
  expect_live_appletun_search(
      engine, "Prize-inspection K1 rejected a known live Dragon target.");
}

void test_deck_search_k1_keeps_exact_live_dragon() {
  Fixture fixture{"issue-2188-deck-k1-live-dragon", 218802};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(
      engine, live_payload_state({sim::Card::Appletun}));
  sim::EngineTestAccess::set_knowledge(engine, true, false);

  // Deck-search K1 continues to inspect the exact deck for the printed target class:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/2188
  expect_live_appletun_search(
      engine, "Deck-search K1 rejected a known live Dragon target.");
}

void test_k1_rejects_known_empty_target_class() {
  Fixture fixture{"issue-2188-k1-no-target", 218803};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(
      engine, live_payload_state({sim::Card::Grass}));
  sim::EngineTestAccess::set_knowledge(engine, false, true);

  // K1 cannot pay a search Item whose exact deck has no Psychic or Dragon Pokémon:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/2188
  if (sim::EngineTestAccess::play_mysterious_treasure_for_payload(engine)) {
    throw std::runtime_error(
        "Prize-inspection K1 allowed Mysterious Treasure with no legal target.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (after.hand.size() != 2 || after.deck.size() != 1 || !after.discard.empty()) {
    throw std::runtime_error(
        "Rejected Mysterious Treasure changed zones before target legality passed.");
  }
}

void test_true_k0_retains_recipe_plausibility() {
  Fixture fixture{"issue-2188-true-k0", 218804};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(
      engine, live_payload_state({sim::Card::Appletun}));
  sim::EngineTestAccess::set_knowledge(engine, false, false);

  // Before legal inspection, fixed-list Psychic or Dragon copies remain plausible:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/2188
  expect_live_appletun_search(
      engine, "True K0 lost fixed-list target plausibility.");
}

}  // namespace

int main() {
  test_prize_inspection_k1_uses_exact_live_dragon();
  test_deck_search_k1_keeps_exact_live_dragon();
  test_k1_rejects_known_empty_target_class();
  test_true_k0_retains_recipe_plausibility();
  return 0;
}
