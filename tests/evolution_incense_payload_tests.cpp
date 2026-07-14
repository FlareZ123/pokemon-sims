#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_evolution_incense(Engine& engine, const bool permit_payload) {
    return engine.play_evolution_incense(permit_payload);
  }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static bool play_ultra_ball(Engine& engine, const bool permit_payload) {
    return engine.play_ultra_ball(permit_payload);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

struct Fixture {
  sim::Scenario scenario{"evolution-incense-payload", sim::DciProfile::StrictJit,
                          sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{202};
  sim::Engine engine{scenario, recipe, rng};
};

void test_evolution_incense_fetches_jit_payload_for_mysterious_treasure() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
  state.discard = {sim::Card::ProfessorBurnet};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Evolution Incense may search the Stage 2 Mega Dragonite ex payload even after
  // K1 proves Regidrago VSTAR absent, because Mysterious Treasure remains a live
  // same-turn discard continuation:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sm6-113
  if (!sim::EngineTestAccess::play_evolution_incense(fixture.engine, true) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Evolution Incense should fetch Mega Dragonite ex for a live strict-JIT discard route.");
  }

  if (!sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, true)) {
    throw std::runtime_error("Mysterious Treasure should discard the Evolution Incense payload this turn.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  // Regidrago VSTAR can use an attack from a Dragon Pokémon in discard:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!contains(after.discard, sim::Card::MegaDragonite) ||
      !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Evolution Incense's fetched payload must remain in this turn's discard.");
  }
}

void test_duplicate_incense_pays_ultra_ball_after_payload_search() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::EvolutionIncense,
                sim::Card::UltraBall};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // One Evolution Incense searches the Evolution Dragon payload. The fetched payload
  // and the distinct surviving Incense are Ultra Ball's two other cards:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  if (!sim::EngineTestAccess::play_evolution_incense(fixture.engine, true)) {
    throw std::runtime_error("The first Evolution Incense should recognize the duplicate-Incense Ultra Ball route.");
  }
  if (!sim::EngineTestAccess::play_ultra_ball(fixture.engine, true)) {
    throw std::runtime_error("Ultra Ball should discard the fetched payload and remaining Evolution Incense.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (count(after.discard, sim::Card::EvolutionIncense) != 2 ||
      !contains(after.discard, sim::Card::MegaDragonite) ||
      !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The duplicate-Incense bridge must leave the payload in this turn's discard.");
  }
}

void test_single_incense_does_not_claim_unpayable_ultra_ball_route() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense, sim::Card::UltraBall};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Ultra Ball needs two other cards. With only the fetched payload beside Ultra Ball,
  // one Evolution Incense must be held because no distinct second cost survives:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  if (sim::EngineTestAccess::play_evolution_incense(fixture.engine, true)) {
    throw std::runtime_error("A single Evolution Incense must not claim an unpayable Ultra Ball continuation.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (count(after.hand, sim::Card::EvolutionIncense) != 1 ||
      count(after.hand, sim::Card::UltraBall) != 1 ||
      !contains(after.deck, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The unpayable bridge should preserve both Items and the payload target.");
  }
}

void test_evolution_incense_holds_when_k1_vstar_target_is_absent() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::EvolutionIncense};
  state.deck = {sim::Card::GoodraVstar, sim::Card::Dipplin};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Evolution Incense may search an Evolution Pokémon, but K1 proves the requested
  // Regidrago VSTAR axis target is absent. Goodra VSTAR and Dipplin are unrelated
  // fallback Evolutions after the Dragon payload axis is already satisfied:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  if (sim::EngineTestAccess::play_evolution_incense(fixture.engine, true)) {
    throw std::runtime_error("Evolution Incense should be held when K1 proves Regidrago VSTAR absent.");
  }

  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::EvolutionIncense) ||
      !contains(after.deck, sim::Card::GoodraVstar) || !contains(after.deck, sim::Card::Dipplin)) {
    throw std::runtime_error("The dead VSTAR-only search must preserve Incense and unrelated Evolutions.");
  }
}

void test_evolution_incense_uses_legal_post_search_fallbacks() {
  for (const sim::Card fallback : {sim::Card::MegaDragonite, sim::Card::Dragapult}) {
    Fixture fixture;
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
    state.hand = {sim::Card::EvolutionIncense};
    state.deck = {fallback};
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

    // Evolution Incense may search any Evolution Pokémon. Once the K0 search proves
    // Regidrago VSTAR absent, Stage 2 Mega Dragonite ex and Dragapult ex remain legal
    // fallback targets: https://api.pokemontcg.io/v2/cards/swsh1-163
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/sv6-130
    if (!sim::EngineTestAccess::play_evolution_incense(fixture.engine, false)) {
      throw std::runtime_error("Evolution Incense should resolve the K0 VSTAR search.");
    }
    const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
    if (!contains(after.hand, fallback) || !after.deck.empty()) {
      throw std::runtime_error("Evolution Incense should take the available legal Evolution fallback.");
    }
  }
}

}  // namespace

int main() {
  try {
    test_evolution_incense_fetches_jit_payload_for_mysterious_treasure();
    test_duplicate_incense_pays_ultra_ball_after_payload_search();
    test_single_incense_does_not_claim_unpayable_ultra_ball_route();
    test_evolution_incense_holds_when_k1_vstar_target_is_absent();
    test_evolution_incense_uses_legal_post_search_fallbacks();
    std::cout << "Evolution Incense payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
