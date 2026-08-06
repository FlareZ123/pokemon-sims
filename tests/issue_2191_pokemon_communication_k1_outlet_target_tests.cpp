// Focused K0/K1 continuation regression: https://github.com/FlareZ123/pokemon-sims/issues/2191
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
  static bool communication_outlet(Engine& engine, const Card returned,
                                   const Card fetched) {
    return engine.pokemon_communication_payload_outlet_after_exchange(
        returned, fetched, true, false);
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
      : scenario{label, sim::DciProfile::NoDiscardControl,
                 sim::LockMode::None, false, 4},
        recipe(sim::baseline_recipe()),
        rng(seed),
        engine(scenario, recipe, rng) {}
};

sim::State outlet_state(const sim::Card returned, const sim::Card item,
                        std::vector<sim::Card> deck) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, returned, item,
                sim::Card::Channeler};
  state.deck = std::move(deck);
  state.prizes = {sim::Card::Dipplin};
  return state;
}

bool mysterious_outlet(sim::Engine& engine) {
  return sim::EngineTestAccess::communication_outlet(
      engine, sim::Card::CrobatV, sim::Card::RegidragoV);
}

bool quick_outlet(sim::Engine& engine) {
  return sim::EngineTestAccess::communication_outlet(
      engine, sim::Card::ForretressEx, sim::Card::RegidragoVstar);
}

void test_prize_k1_mysterious_uses_exact_appletun() {
  Fixture fixture{"issue-2191-prize-k1-mysterious", 219101};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(
      engine, outlet_state(sim::Card::CrobatV, sim::Card::MysteriousTreasure,
                           {sim::Card::RegidragoV, sim::Card::Appletun}));
  sim::EngineTestAccess::set_knowledge(engine, false, true);

  // Communication returns Crobat V after fetching Regidrago V. Prize-derived K1
  // must recognize exact post-exchange Appletun as Mysterious Treasure's target:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/2191
  if (!mysterious_outlet(engine)) {
    throw std::runtime_error(
        "Prize-inspection K1 ignored exact Appletun after Communication.");
  }
}

void test_deck_k1_mysterious_uses_exact_appletun() {
  Fixture fixture{"issue-2191-deck-k1-mysterious", 219102};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(
      engine, outlet_state(sim::Card::CrobatV, sim::Card::MysteriousTreasure,
                           {sim::Card::RegidragoV, sim::Card::Appletun}));
  sim::EngineTestAccess::set_knowledge(engine, true, false);

  // Deck-search K1 uses the same exact post-exchange Dragon target:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/2191
  if (!mysterious_outlet(engine)) {
    throw std::runtime_error(
        "Deck-search K1 ignored exact Appletun after Communication.");
  }
}

void test_prize_k1_quick_uses_exact_mawile() {
  Fixture fixture{"issue-2191-prize-k1-quick", 219103};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(
      engine, outlet_state(sim::Card::ForretressEx, sim::Card::QuickBall,
                           {sim::Card::RegidragoVstar, sim::Card::MawileGX}));
  sim::EngineTestAccess::set_knowledge(engine, false, true);

  // Communication returns Stage 1 Forretress ex after fetching Regidrago VSTAR.
  // Prize-derived K1 must recognize exact post-exchange Basic Mawile-GX:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm11-141
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/2191
  if (!quick_outlet(engine)) {
    throw std::runtime_error(
        "Prize-inspection K1 ignored exact Mawile-GX after Communication.");
  }
}

void test_deck_k1_quick_uses_exact_mawile() {
  Fixture fixture{"issue-2191-deck-k1-quick", 219104};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(
      engine, outlet_state(sim::Card::ForretressEx, sim::Card::QuickBall,
                           {sim::Card::RegidragoVstar, sim::Card::MawileGX}));
  sim::EngineTestAccess::set_knowledge(engine, true, false);

  // Deck-search K1 uses the same exact post-exchange Basic target:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm11-141
  // https://github.com/FlareZ123/pokemon-sims/issues/2191
  if (!quick_outlet(engine)) {
    throw std::runtime_error(
        "Deck-search K1 ignored exact Mawile-GX after Communication.");
  }
}

void test_k1_rejects_exact_missing_later_targets() {
  Fixture mysterious_fixture{"issue-2191-k1-no-mysterious-target", 219105};
  sim::Engine& mysterious_engine = mysterious_fixture.engine;
  sim::EngineTestAccess::set_state(
      mysterious_engine,
      outlet_state(sim::Card::CrobatV, sim::Card::MysteriousTreasure,
                   {sim::Card::RegidragoV, sim::Card::Grass}));
  sim::EngineTestAccess::set_knowledge(mysterious_engine, false, true);

  Fixture quick_fixture{"issue-2191-k1-no-quick-target", 219106};
  sim::Engine& quick_engine = quick_fixture.engine;
  sim::EngineTestAccess::set_state(
      quick_engine,
      outlet_state(sim::Card::ForretressEx, sim::Card::QuickBall,
                   {sim::Card::RegidragoVstar, sim::Card::Grass}));
  sim::EngineTestAccess::set_knowledge(quick_engine, false, true);

  // Exact K1 rejects each paid continuation when its post-exchange deck has no
  // target in the later Item's printed class:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // https://github.com/FlareZ123/pokemon-sims/issues/2191
  if (mysterious_outlet(mysterious_engine) || quick_outlet(quick_engine)) {
    throw std::runtime_error(
        "K1 allowed a post-Communication search with no exact target.");
  }
}

void test_true_k0_retains_recipe_plausibility() {
  Fixture mysterious_fixture{"issue-2191-k0-mysterious", 219107};
  sim::Engine& mysterious_engine = mysterious_fixture.engine;
  sim::EngineTestAccess::set_state(
      mysterious_engine,
      outlet_state(sim::Card::CrobatV, sim::Card::MysteriousTreasure,
                   {sim::Card::RegidragoV, sim::Card::Grass}));
  sim::EngineTestAccess::set_knowledge(mysterious_engine, false, false);

  Fixture quick_fixture{"issue-2191-k0-quick", 219108};
  sim::Engine& quick_engine = quick_fixture.engine;
  sim::EngineTestAccess::set_state(
      quick_engine,
      outlet_state(sim::Card::ForretressEx, sim::Card::QuickBall,
                   {sim::Card::RegidragoVstar, sim::Card::Grass}));
  sim::EngineTestAccess::set_knowledge(quick_engine, false, false);

  // Before legal inspection, fixed-list target identities remain plausible:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/2191
  if (!mysterious_outlet(mysterious_engine) || !quick_outlet(quick_engine)) {
    throw std::runtime_error(
        "True K0 lost fixed-list post-Communication target plausibility.");
  }
}

}  // namespace

int main() {
  test_prize_k1_mysterious_uses_exact_appletun();
  test_deck_k1_mysterious_uses_exact_appletun();
  test_prize_k1_quick_uses_exact_mawile();
  test_deck_k1_quick_uses_exact_mawile();
  test_k1_rejects_exact_missing_later_targets();
  test_true_k0_retains_recipe_plausibility();
  return 0;
}
