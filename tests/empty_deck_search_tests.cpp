#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static TrialOutcome& outcome(Engine& engine) { return engine.outcome_; }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool bench_from_hand(Engine& engine, const Card card, const bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
  static bool play_crispin(Engine& engine) { return engine.play_crispin(); }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario{"empty-deck-search", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{322};
  sim::TraceLog trace{true, {}};
  sim::Engine engine;

  explicit Fixture(sim::State state)
      : engine(scenario, recipe, rng, &trace) {
    sim::EngineTestAccess::set_state(engine, std::move(state));
  }
};

void test_search_item_does_not_pay_its_cost() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Dipplin};
  Fixture fixture(std::move(state));

  // A deck search cannot begin with zero cards, and a Trainer known to have no
  // effect cannot be played. Preserve both the Item and its discard cost:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  if (sim::EngineTestAccess::play_mysterious_treasure(fixture.engine, false)) {
    throw std::runtime_error("Mysterious Treasure must be held when the deck is empty.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.hand.size() != 2U || !after.discard.empty()) {
    throw std::runtime_error("An illegal empty-deck search must not spend the Item or discard cost.");
  }
}

void test_star_alchemy_preserves_the_vstar_power() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::ForestSealStone};
  Fixture fixture(std::move(state));

  // Star Alchemy is an optional deck search. With zero cards, it cannot be used and
  // must preserve the shared one-per-game VSTAR Power resource:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
  if (sim::EngineTestAccess::use_fss(fixture.engine)) {
    throw std::runtime_error("Star Alchemy must not activate when the deck is empty.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.vstar_power_used || sim::EngineTestAccess::outcome(fixture.engine).used_fss) {
    throw std::runtime_error("A rejected Star Alchemy search must preserve the VSTAR Power.");
  }
}

void test_optional_entry_search_is_declined_after_legal_bench() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Oricorio};
  Fixture fixture(std::move(state));

  // Playing Oricorio to the Bench remains legal. Vital Dance says "you may search",
  // so its optional search is declined when the deck has zero cards:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://compendium.pokegym.net/category/7-gameplay/searching-deck-or-discard/
  if (!sim::EngineTestAccess::bench_from_hand(fixture.engine, sim::Card::Oricorio, true)) {
    throw std::runtime_error("Oricorio should still be legally Benched from hand.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.bench.size() != 1U || after.bench.front().card != sim::Card::Oricorio ||
      !after.hand.empty()) {
    throw std::runtime_error("The empty-deck ruling must suppress only Vital Dance, not the Bench play.");
  }
  if (fixture.trace.lines.empty() ||
      fixture.trace.lines.back().find("ABILITY DECLINED") == std::string::npos) {
    throw std::runtime_error("The declined optional search should be explicit in the trace.");
  }
}

void test_search_supporter_does_not_consume_the_supporter_slot() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Crispin};
  Fixture fixture(std::move(state));

  // Crispin cannot be played when its deck search cannot begin:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  if (sim::EngineTestAccess::play_crispin(fixture.engine)) {
    throw std::runtime_error("Crispin must be held when the deck is empty.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (after.supporter_used || after.hand.size() != 1U || !after.discard.empty()) {
    throw std::runtime_error("A rejected empty-deck Supporter must preserve the Supporter slot and card.");
  }
}

}  // namespace

int main() {
  try {
    test_search_item_does_not_pay_its_cost();
    test_star_alchemy_preserves_the_vstar_power();
    test_optional_entry_search_is_declined_after_legal_bench();
    test_search_supporter_does_not_consume_the_supporter_slot();
    std::cout << "empty-deck search tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
