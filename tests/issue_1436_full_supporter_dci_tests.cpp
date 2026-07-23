#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static std::optional<Card> choose_discard(
      const Engine& engine, const std::optional<Card> excluded) {
    return engine.choose_discard(false, false, true, excluded, false);
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::State singleton_supporter_state(const sim::Card supporter) {
  sim::State state;
  state.turn = 1;
  state.hand = {supporter};
  return state;
}

void test_only_full_supporter_lock_downgrades_singletons() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "The registered shell recipe is unavailable.");
  const auto choose = [deck](const sim::LockMode lock,
                             const sim::Card supporter,
                             const std::optional<sim::Card> excluded,
                             const std::uint64_t seed) {
    const sim::Scenario scenario{"issue-1436-control",
                                 sim::DciProfile::StrictJit, lock, false, 5};
    std::mt19937_64 rng(seed);
    sim::Engine engine(scenario, deck->recipe, rng);
    sim::EngineTestAccess::set_state(
        engine, singleton_supporter_state(supporter));
    return sim::EngineTestAccess::choose_discard(engine, excluded);
  };

  // Full Supporter lock makes these effects permanently unusable, while
  // unrelated locks and ordinary games retain singleton DCI protection:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/1436
  expect(choose(sim::LockMode::FullSupporter, sim::Card::Gladion,
                sim::Card::MysteriousTreasure, 143601) == sim::Card::Gladion,
         "Full Supporter lock did not expose singleton Gladion as Item fuel.");
  expect(choose(sim::LockMode::FullSupporter, sim::Card::ProfessorBurnet,
                sim::Card::QuickBall, 143602) == sim::Card::ProfessorBurnet,
         "Full Supporter lock did not expose singleton Burnet as Item fuel.");
  expect(!choose(sim::LockMode::None, sim::Card::Gladion,
                 sim::Card::MysteriousTreasure, 143603),
         "No-lock strict DCI stopped protecting singleton Gladion.");
  expect(!choose(sim::LockMode::FullItem, sim::Card::ProfessorBurnet,
                 sim::Card::QuickBall, 143604),
         "Item lock incorrectly downgraded singleton Burnet.");
  expect(!choose(sim::LockMode::FullRuleBoxAbility, sim::Card::Gladion,
                 sim::Card::MysteriousTreasure, 143605),
         "Rule Box Ability lock incorrectly downgraded singleton Gladion.");
  expect(!choose(sim::LockMode::FullSupporter, sim::Card::Gladion,
                 sim::Card::Gladion, 143606),
         "A card was permitted to pay its own printed discard cost.");
}

void test_seed_57_uses_dead_supporter_fuel() {
  const auto scenario = sim::scenario_by_label(
      "strict-jit-supporter-lock/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1436 fixture is unavailable.");

  std::mt19937_64 rng(57);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  engine.run();

  expect(trace_contains(trace, "Gladion (Quick Ball cost)"),
         "Seed 57 still protected permanently locked Gladion.");
  expect(trace_contains(trace, "T1 | PLAY ITEM | rules: R-QB-01"),
         "Seed 57 still passed instead of playing its legal search Item.");
  expect(trace_contains(trace, "T1 | BENCH | rules: R-GAME-BENCH | Regidrago V"),
         "Seed 57 did not convert the dead Supporter into a setup connector.");
}

}  // namespace

int main() {
  test_only_full_supporter_lock_downgrades_singletons();
  test_seed_57_uses_dead_supporter_fuel();
  return 0;
}
