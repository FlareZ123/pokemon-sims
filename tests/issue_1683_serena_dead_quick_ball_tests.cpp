#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool dead_quick_ball(const Engine& engine) {
    return engine.issue_1683_rulebox_locked_quick_ball_is_serena_cost();
  }
  static bool play_serena(Engine& engine) { return engine.play_serena(); }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::State locked_established_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1};
  state.bench = {
      sim::Pokemon{sim::Card::Oricorio, 1},
      sim::Pokemon{sim::Card::RegidragoV, 1, 0, 1},
      sim::Pokemon{sim::Card::RegidragoV, 2},
  };
  state.hand = {
      sim::Card::Serena,
      sim::Card::Lusamine,
      sim::Card::QuickBall,
      sim::Card::EarthenVessel,
  };
  state.deck = {
      sim::Card::Grass,
      sim::Card::RegidragoVstar,
      sim::Card::HisuianHeavyBall,
      sim::Card::FieldBlower,
      sim::Card::PathToPeak,
  };
  state.prizes = {
      sim::Card::Gladion,
      sim::Card::ProfessorBurnet,
      sim::Card::Crispin,
      sim::Card::Grass,
      sim::Card::RoseannesBackup,
      sim::Card::Fire,
  };
  state.discard = {sim::Card::TateLiza};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  return sim::Engine(scenario, sim::baseline_recipe(), rng, trace);
}

void test_locked_established_state_discards_quick_ball() {
  const sim::Scenario scenario{
      "issue-1683/positive", sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility, false, 5};
  std::mt19937_64 rng{1683};
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(scenario, rng, &trace);
  sim::EngineTestAccess::set_state(engine, locked_established_state());

  // Path to the Peak suppresses the Rule Box Abilities on Latias ex, Tapu Lele-GX,
  // and Crobat V. Two Regidrago V and Oricorio are already established, so a
  // projected payable Quick Ball has no setup-advancing Basic target. Serena may
  // therefore use the dead singleton as its mandatory discard and draw to five:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Core Supporter, Item, Ability, search, discard, and Bench procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Dynamic DCI policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1683
  expect(sim::EngineTestAccess::dead_quick_ball(engine),
         "The locked established state did not classify Quick Ball as setup-dead.");
  expect(sim::EngineTestAccess::play_serena(engine),
         "Serena did not use the setup-dead Quick Ball.");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count(after.discard, sim::Card::QuickBall) == 1,
         "Quick Ball did not pay Serena's mandatory discard.");
  expect(count(after.hand, sim::Card::Lusamine) == 1 &&
             count(after.hand, sim::Card::EarthenVessel) == 1,
         "Serena consumed a protected recovery or Energy connector.");
  expect(after.hand.size() == 5U,
         "Serena did not draw until five cards were held.");
  expect(trace_contains(trace, "Quick Ball (Serena chosen discard)"),
         "The trace did not record Quick Ball as Serena's chosen discard.");
}

void test_missing_second_regidrago_preserves_quick_ball() {
  const sim::Scenario scenario{
      "issue-1683/missing-regi", sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility, false, 5};
  std::mt19937_64 rng{1684};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = locked_established_state();
  state.bench.pop_back();
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The fallback is reserved for the confirmed two-Regidrago public state. Quick
  // Ball remains protected while the broader Basic board is still incomplete:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/issues/1683
  expect(!sim::EngineTestAccess::dead_quick_ball(engine),
         "A one-Regidrago state incorrectly marked Quick Ball setup-dead.");
  expect(!sim::EngineTestAccess::play_serena(engine),
         "Serena spent Quick Ball before the confirmed board was established.");
}

void test_unlocked_wonder_tag_route_preserves_quick_ball() {
  const sim::Scenario scenario{
      "issue-1683/unlocked", sim::DciProfile::StrictJit,
      sim::LockMode::None, false, 5};
  std::mt19937_64 rng{1685};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, locked_established_state());

  // Without Path-style Rule Box Ability lock, a searched Tapu Lele-GX may trigger
  // Wonder Tag when played from hand to the Bench. The singleton Quick Ball retains
  // connector value and must stay protected:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1683
  expect(!sim::EngineTestAccess::dead_quick_ball(engine),
         "An unlocked Wonder Tag state incorrectly marked Quick Ball setup-dead.");
  expect(!sim::EngineTestAccess::play_serena(engine),
         "Serena spent Quick Ball while Rule Box Abilities were available.");
}

void test_ordinary_lower_dci_cost_stays_ahead() {
  const sim::Scenario scenario{
      "issue-1683/ordinary-cost", sim::DciProfile::StrictJit,
      sim::LockMode::FullRuleBoxAbility, false, 5};
  std::mt19937_64 rng{1686};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = locked_established_state();
  state.hand.push_back(sim::Card::Dipplin);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Dipplin cannot enter play in this recipe because Applin is absent. That stable
  // dead-card DCI remains ahead of the conditional Quick Ball fallback:
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/1683
  expect(sim::EngineTestAccess::play_serena(engine),
         "Serena did not use the ordinary lower-DCI cost.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count(after.discard, sim::Card::Dipplin) == 1,
         "Dipplin did not remain ahead of the conditional Quick Ball fallback.");
  expect(count(after.hand, sim::Card::QuickBall) == 1,
         "Quick Ball was spent despite an ordinary lower-DCI cost.");
}

void test_seed_2413_uses_serena_then_vessel() {
  const auto scenario =
      sim::scenario_by_label("strict-jit-rulebox-ability-lock/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The source-bound issue-1683 seed fixture is unavailable.");

  std::mt19937_64 rng{2413};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  engine.run();

  // The exact CI reproduction must use Serena on T3, discard the conditionally
  // setup-dead Quick Ball, then use Earthen Vessel to establish the two Grass
  // attachments across T3 and T4 while preserving the live recovery cards:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Official Supporter, Item, Ability, attachment, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1683
  expect(trace_contains(trace, "T3 | DISCARD | rules: R-SERENA-01 | Quick Ball"),
         "Seed 2413 did not discard Quick Ball with Serena on T3.");
  expect(trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-SERENA-01"),
         "Seed 2413 did not use Serena on T3.");
  expect(trace_contains(trace, "T3 | Earthen Vessel | rules: R-EV-01; R-GAME-ITEM"),
         "Seed 2413 did not use Earthen Vessel after Serena.");
  expect(trace_contains(trace, "T3 | ATTACH | rules: R-GAME-ENERGY | Grass Energy"),
         "Seed 2413 did not attach the first searched Grass Energy on T3.");
  expect(trace_contains(trace, "T4 | ATTACH | rules: R-GAME-ENERGY | Grass Energy"),
         "Seed 2413 did not attach the second Grass Energy on T4.");
}

}  // namespace

int main() {
  try {
    test_locked_established_state_discards_quick_ball();
    test_missing_second_regidrago_preserves_quick_ball();
    test_unlocked_wonder_tag_route_preserves_quick_ball();
    test_ordinary_lower_dci_cost_stays_ahead();
    test_seed_2413_uses_serena_then_vessel();
    std::cout << "Issue 1683 Serena dead Quick Ball tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
