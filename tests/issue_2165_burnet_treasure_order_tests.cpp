#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = false,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_2165_burnet_treasure_vstar_route_available();
  }
  static bool play_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
  static bool deck_seen(const Engine& engine) { return engine.deck_seen_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-2165", sim::DciProfile::StrictJit, lock, false, 5};
}

sim::State exact_public_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::ProfessorBurnet,
                sim::Card::UltraBall, sim::Card::Dragapult,
                sim::Card::LatiasEx, sim::Card::Fire};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

sim::Engine make_engine(const sim::Scenario& selected, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  if (deck == nullptr) throw std::runtime_error("Registered shell is unavailable.");
  return sim::Engine(selected, deck->recipe, rng, trace);
}

void test_treasure_precedes_burnet_and_establishes_k1() {
  std::mt19937_64 rng{2165};
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(scenario(), rng, &trace);
  sim::EngineTestAccess::set_state(engine, exact_public_state());

  // Mysterious Treasure's one-card cost has higher active-move realism here than
  // Ultra Ball's two-card cost. Spending the redundant Ultra Ball establishes the
  // missing VSTAR and K1 before Professor Burnet consumes the Supporter action:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swshp-SWSH167
  // Official Trainer and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1 and action ordering: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2165
  expect(sim::EngineTestAccess::route_available(engine),
         "The exact public Treasure-before-Burnet route was rejected.");
  expect(sim::EngineTestAccess::play_treasure(engine),
         "Mysterious Treasure did not resolve.");
  expect(contains(engine.state().hand, sim::Card::RegidragoVstar),
         "Mysterious Treasure did not search Regidrago VSTAR.");
  expect(contains(engine.state().discard, sim::Card::MysteriousTreasure) &&
             contains(engine.state().discard, sim::Card::UltraBall),
         "The legal Treasure card and redundant Ultra Ball cost were not discarded.");
  expect(contains(engine.state().hand, sim::Card::ProfessorBurnet) &&
             !engine.state().supporter_used,
         "The Treasure route consumed or lost the Turn 1 Supporter decision.");
  expect(sim::EngineTestAccess::deck_seen(engine),
         "The legal deck search did not establish K1.");
  expect(trace_contains(trace, "searched Regidrago VSTAR before the Turn 1 Supporter decision"),
         "The trace did not record the corrected action order.");
}

void test_legality_and_scope_boundaries() {
  std::mt19937_64 rng{2166};
  const auto rejected = [&rng](sim::State state, const sim::Scenario selected,
                               const char* message) {
    sim::Engine engine = make_engine(selected, rng);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  rejected(exact_public_state(), scenario(sim::LockMode::FullItem),
           "The route bypassed Item lock.");
  {
    sim::State state = exact_public_state();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::UltraBall),
                     state.hand.end());
    rejected(std::move(state), scenario(),
             "The route ignored its low-DCI Ultra Ball cost requirement.");
  }
  {
    sim::State state = exact_public_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::RegidragoVstar),
                     state.deck.end());
    rejected(std::move(state), scenario(),
             "The route ignored the absence of a publicly possible VSTAR target.");
  }
  {
    sim::State state = exact_public_state();
    state.supporter_used = true;
    rejected(std::move(state), scenario(),
             "The route expanded beyond the pre-Supporter ordering window.");
  }
}
}  // namespace

int main() {
  test_treasure_precedes_burnet_and_establishes_k1();
  test_legality_and_scope_boundaries();
  return 0;
}
