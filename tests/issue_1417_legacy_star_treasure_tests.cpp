#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star_issue_1417(); }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(true);
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

std::vector<sim::Card> treasure_route_deck() {
  // Legacy Star removes cards from the vector back. Regidrago V remains as a legal
  // Mysterious Treasure target after seven cards enter discard:
  // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  return {sim::Card::RegidragoV, sim::Card::ChaoticSwell,
          sim::Card::PathToPeak, sim::Card::ErikasInvitation,
          sim::Card::Guzma, sim::Card::Channeler,
          sim::Card::FieldBlower, sim::Card::GoodraVstar};
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode locks, const int max_turn = 4)
      : scenario{"issue-1417", sim::DciProfile::StrictJit, locks, true, max_turn},
        recipe(sim::baseline_recipe()), rng(1417), engine(scenario, recipe, rng) {}
};

void seed_future_treasure_state(sim::Engine& engine) {
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.manual_energy_used = true;
  state.hand = {sim::Card::Grass, sim::Card::Dragapult};
  state.discard = {sim::Card::MysteriousTreasure};
  state.deck = treasure_route_deck();
  sim::EngineTestAccess::set_deck_seen(engine);
}

void test_recovers_and_holds_treasure_for_t3_payload() {
  Fixture fixture(sim::LockMode::None);
  sim::Engine& engine = fixture.engine;
  seed_future_treasure_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);

  // Legacy Star may recover Mysterious Treasure. The held Grass is next turn's
  // manual attachment, and Treasure can then discard held Dragapult ex on the same
  // turn GGF becomes complete:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1417
  const bool used = sim::EngineTestAccess::use_legacy_star(engine);
  if (!used || !contains(state.hand, sim::Card::MysteriousTreasure)) {
    std::string cards;
    for (const sim::Card card : state.hand) {
      if (!cards.empty()) cards += ", ";
      cards += sim::name(card);
    }
    throw std::runtime_error("Legacy Star future Treasure failure: used=" +
                             std::string(used ? "true" : "false") +
                             "; hand=" + cards);
  }
  if (sim::EngineTestAccess::play_mysterious_treasure(engine)) {
    throw std::runtime_error("Recovered Treasure must remain held during the Legacy Star turn.");
  }

  state.turn = 3;
  state.manual_energy_used = false;
  state.discarded_this_turn.clear();
  if (!sim::EngineTestAccess::attach_manual(engine) ||
      !sim::EngineTestAccess::play_mysterious_treasure(engine)) {
    throw std::runtime_error("The next-turn attachment and Treasure payload route should resolve.");
  }
  if (!state.active || state.active->grass != 2 || state.active->fire != 1 ||
      !contains(state.discarded_this_turn, sim::Card::Dragapult) ||
      !contains(state.hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("The route must complete GGF, discard Dragapult ex on T3, and search legally.");
  }
}

void test_rejects_future_treasure_under_item_lock() {
  Fixture fixture(sim::LockMode::FullItem);
  sim::Engine& engine = fixture.engine;
  seed_future_treasure_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);

  // Item lock prevents Mysterious Treasure from being played from hand:
  // https://api.pokemontcg.io/v2/cards/me2pt5-16
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1417
  if (!sim::EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The control should still resolve Legacy Star's top-seven effect.");
  }
  if (contains(state.hand, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("Legacy Star must not recover an Item-locked future outlet.");
  }
}

void test_rejects_future_treasure_without_held_payload() {
  Fixture fixture(sim::LockMode::None);
  sim::Engine& engine = fixture.engine;
  seed_future_treasure_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.hand = {sim::Card::Grass};

  // Mysterious Treasure requires another hand card as its discard cost, and the
  // proposed route specifically needs a held Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://github.com/FlareZ123/pokemon-sims/issues/1417
  if (!sim::EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The control should still resolve Legacy Star's random payload line.");
  }
  if (contains(state.hand, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("Legacy Star must not recover Treasure without a held payload cost.");
  }
}

void test_rejects_future_treasure_when_two_attachments_are_missing() {
  Fixture fixture(sim::LockMode::None);
  sim::Engine& engine = fixture.engine;
  seed_future_treasure_state(engine);
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1,
                              sim::Tool::None};

  // One manual Energy attachment per turn cannot complete two missing Grass Energy
  // by the immediate next-turn payload window:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1417
  if (!sim::EngineTestAccess::use_legacy_star(engine)) {
    throw std::runtime_error("The control should still resolve Legacy Star's random payload line.");
  }
  if (contains(state.hand, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("Legacy Star must not recover the one-turn Treasure route when two attachments remain.");
  }
}

void test_exact_seed_389_reaches_t3() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("The registered seed-389 fixture is unavailable.");
  }

  std::mt19937_64 rng{389};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The exact public route must recover Treasure on T2, hold it until the T3
  // attachment completes GGF, then discard Dragapult ex during T3:
  // Legacy Star: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Official turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1417
  if (outcome.first_ready_turn != 3) {
    throw std::runtime_error("The corrected seed-389 route did not reach T3.");
  }
  const auto contains_line = [&trace](const std::string& needle) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&needle](const std::string& line) {
                         return line.find(needle) != std::string::npos;
                       });
  };
  if (!contains_line("Recovered Mysterious Treasure for the next-turn strict-JIT Dragon payload") ||
      !contains_line("T3 | DISCARD") ||
      !contains_line("Dragapult ex (Mysterious Treasure cost)") ||
      !contains_line("T3 | READY")) {
    throw std::runtime_error("The exact seed-389 trace omitted the Treasure payload continuation.");
  }
}

}  // namespace

int main() {
  try {
    test_recovers_and_holds_treasure_for_t3_payload();
    test_rejects_future_treasure_under_item_lock();
    test_rejects_future_treasure_without_held_payload();
    test_rejects_future_treasure_when_two_attachments_are_missing();
    test_exact_seed_389_reaches_t3();
    std::cout << "issue 1417 Legacy Star Treasure tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
