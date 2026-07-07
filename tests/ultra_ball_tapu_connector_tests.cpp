#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static bool play_quick_ball(Engine& engine, const bool permit_payload) {
    return engine.play_quick_ball(permit_payload);
  }
  static bool play_ultra_ball(Engine& engine, const bool permit_payload) {
    return engine.play_ultra_ball(permit_payload);
  }
  static bool bench_from_hand(Engine& engine, const Card card, const bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_ultra_ball_prefers_tapu_over_irrelevant_fallback() {
  const sim::Scenario scenario{"ultra-ball-tapu", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2718};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
  state.hand = {sim::Card::UltraBall, sim::Card::Dipplin, sim::Card::RegidragoV,
                sim::Card::RegidragoVstar};
  state.deck = {sim::Card::LatiasEx, sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball can search for any Pokémon after discarding two other hand cards:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  if (!sim::EngineTestAccess::play_ultra_ball(engine, false)) {
    throw std::runtime_error("Ultra Ball should take the live Tapu Lele-GX connector.");
  }

  const sim::State& after_ball = sim::EngineTestAccess::state(engine);
  if (!contains(after_ball.hand, sim::Card::TapuLeleGX) || contains(after_ball.hand, sim::Card::LatiasEx)) {
    throw std::runtime_error("Ultra Ball must select Tapu Lele-GX before an irrelevant Latias ex fallback.");
  }

  // Wonder Tag resolves when Tapu Lele-GX is played from hand onto the Bench and
  // can search the live Crispin Supporter: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX, true)) {
    throw std::runtime_error("Tapu Lele-GX should have Bench space for Wonder Tag.");
  }
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Crispin)) {
    throw std::runtime_error("Wonder Tag should complete the Ultra Ball to Crispin connector.");
  }
}

void test_duplicate_search_items_form_current_turn_payload_chain() {
  const sim::Scenario scenario{"duplicate-search-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};

  {
    std::mt19937_64 rng{531};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
    state.hand = {sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure, sim::Card::Dipplin};
    state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
    state.discard = {sim::Card::ProfessorBurnet};
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // Mysterious Treasure discards one hand card before searching a Psychic or Dragon Pokémon:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    if (!sim::EngineTestAccess::play_mysterious_treasure(engine, true) ||
        !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::MegaDragonite)) {
      throw std::runtime_error("The first Mysterious Treasure should find Mega Dragonite ex after paying Dipplin.");
    }

    // The separate second Item pays its own discard cost with the fetched Dragon.
    // Apex Dragon can use an attack from a Dragon Pokémon in the discard pile:
    // https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/swsh12-136
    if (!sim::EngineTestAccess::play_mysterious_treasure(engine, true)) {
      throw std::runtime_error("The second Mysterious Treasure should discard the fetched current-turn payload.");
    }
    const sim::State& after = sim::EngineTestAccess::state(engine);
    if (!contains(after.discard, sim::Card::MegaDragonite) ||
        !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
      throw std::runtime_error("Mega Dragonite ex must remain in this turn's discard after the second Mysterious Treasure.");
    }
  }

  {
    std::mt19937_64 rng{532};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
    state.hand = {sim::Card::QuickBall, sim::Card::QuickBall, sim::Card::Dipplin};
    state.deck = {sim::Card::TapuLeleGX, sim::Card::DialgaGX};
    state.discard = {sim::Card::ProfessorBurnet};
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // Quick Ball discards another hand card before searching a Basic Pokémon:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    if (!sim::EngineTestAccess::play_quick_ball(engine, true) ||
        !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::DialgaGX)) {
      throw std::runtime_error("The first Quick Ball should find Dialga-GX after paying Dipplin.");
    }

    // The second Quick Ball can discard that separately searched Basic Dragon payload.
    // Regidrago VSTAR source: https://api.pokemontcg.io/v2/cards/swsh12-136
    if (!sim::EngineTestAccess::play_quick_ball(engine, true)) {
      throw std::runtime_error("The second Quick Ball should discard the fetched current-turn payload.");
    }
    const sim::State& after = sim::EngineTestAccess::state(engine);
    if (!contains(after.discard, sim::Card::DialgaGX) ||
        !contains(after.discarded_this_turn, sim::Card::DialgaGX)) {
      throw std::runtime_error("Dialga-GX must remain in this turn's discard after the second Quick Ball.");
    }
  }

  {
    std::mt19937_64 rng{533};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
    state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
    state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
    state.hand = {sim::Card::UltraBall, sim::Card::UltraBall, sim::Card::RegidragoV,
                  sim::Card::Grass, sim::Card::Grass, sim::Card::Grass};
    state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
    state.discard = {sim::Card::ProfessorBurnet};
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // Ultra Ball discards two other hand cards before searching any Pokémon:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    if (!sim::EngineTestAccess::play_ultra_ball(engine, true) ||
        !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::MegaDragonite)) {
      throw std::runtime_error("The first Ultra Ball should find Mega Dragonite ex after paying two legal costs.");
    }

    // A distinct second Ultra Ball pays two more costs, one of which is the fetched
    // Dragon payload. Apex Dragon can use a Dragon attack from discard:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146 https://api.pokemontcg.io/v2/cards/swsh12-136
    if (!sim::EngineTestAccess::play_ultra_ball(engine, true)) {
      throw std::runtime_error("The second Ultra Ball should discard the fetched current-turn payload.");
    }
    const sim::State& after = sim::EngineTestAccess::state(engine);
    if (!contains(after.discard, sim::Card::MegaDragonite) ||
        !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
      throw std::runtime_error("Mega Dragonite ex must remain in this turn's discard after the second Ultra Ball.");
    }
  }
}

}  // namespace

int main() {
  try {
    test_ultra_ball_prefers_tapu_over_irrelevant_fallback();
    test_duplicate_search_items_form_current_turn_payload_chain();
    std::cout << "ultra ball and duplicate payload connector tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}