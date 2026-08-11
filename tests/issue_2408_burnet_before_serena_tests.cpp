#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static void choose_supporter(Engine& engine) { engine.choose_supporter(); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

const sim::Scenario& strict_second() {
  static const sim::Scenario scenario{"issue-2408", sim::DciProfile::StrictJit,
                                      sim::LockMode::None, false, 5};
  return scenario;
}

const sim::Scenario& strict_second_supporter_lock() {
  static const sim::Scenario scenario{"issue-2408", sim::DciProfile::StrictJit,
                                      sim::LockMode::FullSupporter, false, 5};
  return scenario;
}

sim::State payload_only_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::Serena,
                sim::Card::DialgaGX, sim::Card::Grass};
  state.deck = {sim::Card::Dragapult, sim::Card::GoodraVstar,
                sim::Card::RegidragoV, sim::Card::Fire,
                sim::Card::Grass};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = true,
                        const bool prizes_revealed = false) {
  sim::Engine engine(scenario, sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen,
                                   prizes_revealed);
  return engine;
}

void test_k1_prefers_burnet_and_preserves_serena_and_dragon() {
  std::mt19937_64 rng{2408};
  sim::Engine engine = make_engine(strict_second(), rng, payload_only_state());

  // Both Supporters finish the same strict-JIT payload axis. Burnet searches the
  // known deck and preserves Serena plus the held Dragon as higher-value resources.
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter/search/discard procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, JIT, and resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(engine);
  expect(contains(engine.state().discard, sim::Card::ProfessorBurnet),
         "K1 did not prefer Professor Burnet for the equal-turn payload finish.");
  expect(contains(engine.state().hand, sim::Card::Serena),
         "The stronger K1 route did not preserve Serena.");
  expect(contains(engine.state().hand, sim::Card::DialgaGX),
         "The stronger K1 route did not preserve the held Dragon.");
}

void test_live_item_outlet_preserves_supporter_action() {
  std::mt19937_64 rng{2412};
  sim::State state = payload_only_state();
  state.hand.push_back(sim::Card::MysteriousTreasure);
  state.hand.push_back(sim::Card::ChaoticSwell);
  sim::Engine engine = make_engine(strict_second(), rng, std::move(state));

  // A payable Treasure can discard the held Dragon and complete the payload axis,
  // so the Item route preserves the entire Supporter action under #1341.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Existing boundary: https://github.com/FlareZ123/pokemon-sims/issues/1341
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(engine);
  expect(!engine.state().supporter_used,
         "A live Item payload outlet stopped preserving the Supporter action.");
}

void test_k0_keeps_observable_serena_route() {
  std::mt19937_64 rng{2409};
  sim::Engine engine = make_engine(strict_second(), rng, payload_only_state(),
                                   false, false);
  // K0 cannot assume a Burnet target is in hidden deck. Serena has the public held
  // Dragon route: https://api.pokemontcg.io/v2/cards/swsh12-164
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  sim::EngineTestAccess::choose_supporter(engine);
  expect(contains(engine.state().discard, sim::Card::Serena),
         "K0 stopped using the observable Serena route.");
  expect(contains(engine.state().discard, sim::Card::DialgaGX),
         "K0 Serena did not discard the public Dragon payload.");
}

void test_dead_item_and_supporter_lock_boundaries() {
  std::mt19937_64 rng{2410};
  sim::State dead_item = payload_only_state();
  dead_item.hand.push_back(sim::Card::QuickBall);
  dead_item.deck = {sim::Card::Dragapult, sim::Card::GoodraVstar};
  sim::Engine dead = make_engine(strict_second(), rng, std::move(dead_item));
  // Quick Ball cannot be played without a Basic target, so it cannot displace the
  // legal Burnet route merely because it is in hand:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  sim::EngineTestAccess::choose_supporter(dead);
  expect(contains(dead.state().discard, sim::Card::ProfessorBurnet),
         "A target-dead Item incorrectly suppressed Professor Burnet.");

  sim::Engine locked = make_engine(strict_second_supporter_lock(), rng,
                                   payload_only_state());
  // Supporter lock blocks both Supporters: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  sim::EngineTestAccess::choose_supporter(locked);
  expect(!locked.state().supporter_used,
         "Supporter lock allowed the Burnet/Serena selector to play a Supporter.");
}

void test_registered_seed_19_uses_burnet_on_t2() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "Registered regidrago-shell deck is unavailable.");
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  expect(scenario.has_value(), "Registered strict-jit/go-second scenario is unavailable.");
  std::mt19937_64 rng{19};
  sim::TraceLog trace{true, {}, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Source-bound reproduction keeps the earliest T2 ready turn while choosing the
  // resource-preserving equal-turn Supporter route.
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earliest/resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  expect(outcome.first_ready_turn == 2,
         "Registered seed 19 lost its earliest T2 ready turn.");
  expect(trace_contains(trace, "Professor Burnet"),
         "Registered seed 19 did not use Professor Burnet on the T2 finish.");
  expect(trace_contains(trace, "T2 | READY"),
         "Registered seed 19 did not reach T2 READY.");
}
}  // namespace

int main() {
  try {
    test_k1_prefers_burnet_and_preserves_serena_and_dragon();
    test_live_item_outlet_preserves_supporter_action();
    test_k0_keeps_observable_serena_route();
    test_dead_item_and_supporter_lock_boundaries();
    test_registered_seed_19_uses_burnet_on_t2();
    std::cout << "Issue 2408 Burnet-before-Serena tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
