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

  // K1 proves a deck-resident Dragon can be selected by Professor Burnet. Burnet
  // and Serena both complete the same strict-JIT payload axis this turn, so the
  // repository's resource-preservation priority keeps Serena and held Dialga-GX:
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter/search/discard procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Strict JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(engine);
  expect(contains(engine.state().discard, sim::Card::ProfessorBurnet),
         "K1 did not prefer Professor Burnet for the equal-turn payload finish.");
  expect(contains(engine.state().hand, sim::Card::Serena),
         "The stronger K1 route did not preserve Serena.");
  expect(contains(engine.state().hand, sim::Card::DialgaGX),
         "The stronger K1 route did not preserve the held Dragon.");
  expect(contains(engine.state().discard, sim::Card::Dragapult) ||
             contains(engine.state().discard, sim::Card::GoodraVstar),
         "Professor Burnet did not establish a deck-resident Dragon payload.");
}

void test_k0_keeps_observable_serena_route() {
  std::mt19937_64 rng{2409};
  sim::Engine engine = make_engine(strict_second(), rng, payload_only_state(),
                                   false, false);

  // K0 cannot assume a Burnet target is in the hidden deck. Serena has the public
  // held-Dragon route and remains the deterministic legal completion:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(engine);
  expect(contains(engine.state().discard, sim::Card::Serena),
         "K0 stopped using the observable Serena route.");
  expect(contains(engine.state().discard, sim::Card::DialgaGX),
         "K0 Serena did not discard the public Dragon payload.");
}

void test_missing_burnet_or_deck_payload_keeps_serena() {
  std::mt19937_64 rng{2410};

  sim::State no_burnet = payload_only_state();
  no_burnet.hand.erase(std::find(no_burnet.hand.begin(), no_burnet.hand.end(),
                                 sim::Card::ProfessorBurnet));
  sim::Engine without_burnet = make_engine(strict_second(), rng, std::move(no_burnet));
  // Serena remains required when Burnet is absent: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(without_burnet);
  expect(contains(without_burnet.state().discard, sim::Card::Serena),
         "Absent Burnet suppressed the legal Serena route.");

  sim::State no_deck_payload = payload_only_state();
  no_deck_payload.deck = {sim::Card::RegidragoV, sim::Card::Fire, sim::Card::Grass};
  sim::Engine without_target = make_engine(strict_second(), rng, std::move(no_deck_payload));
  // Burnet may search only cards that actually remain in deck; K1 therefore keeps
  // Serena when no modeled Dragon payload is available:
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(without_target);
  expect(contains(without_target.state().discard, sim::Card::Serena),
         "A K1-dead Burnet route displaced Serena.");
}

void test_missing_held_dragon_keeps_burnet_and_supporter_lock_blocks_both() {
  std::mt19937_64 rng{2411};
  sim::State no_held_dragon = payload_only_state();
  no_held_dragon.hand.erase(std::find(no_held_dragon.hand.begin(),
                                      no_held_dragon.hand.end(),
                                      sim::Card::DialgaGX));
  sim::Engine burnet_only = make_engine(strict_second(), rng,
                                        std::move(no_held_dragon));
  // Without a held Dragon, Serena cannot supply the payload discard while Burnet
  // can still search and discard a deck Dragon:
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(burnet_only);
  expect(contains(burnet_only.state().discard, sim::Card::ProfessorBurnet),
         "Burnet-only payload state lost its legal route.");

  sim::Engine locked = make_engine(strict_second_supporter_lock(), rng,
                                   payload_only_state());
  // Supporter lock blocks both candidate Supporters: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Lock model boundary: https://github.com/FlareZ123/pokemon-sims/issues/2408
  sim::EngineTestAccess::choose_supporter(locked);
  expect(!locked.state().supporter_used,
         "Supporter lock allowed the Burnet/Serena selector to play a Supporter.");
}

void test_live_item_outlet_still_preserves_supporter_action() {
  std::mt19937_64 rng{2412};
  sim::State state = payload_only_state();
  state.hand.push_back(sim::Card::MysteriousTreasure);
  state.hand.push_back(sim::Card::ChaoticSwell);
  sim::Engine engine = make_engine(strict_second(), rng, std::move(state));

  // A payable Mysterious Treasure can discard the held Dragon and finish the same
  // axis while preserving the entire Supporter action, so #1341 remains higher:
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

void test_registered_seed_19_uses_burnet_on_t2() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "Registered regidrago-shell deck is unavailable.");
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  expect(scenario.has_value(), "Registered strict-jit/go-second scenario is unavailable.");
  std::mt19937_64 rng{19};
  sim::TraceLog trace{true, {}, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // The source-bound reproduction reaches T2 under K1. The equal-turn route should
  // use Burnet, preserve Serena and Dialga-GX, then reach the same earliest T2 READY:
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earliest/resource policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2408
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
    test_k0_keeps_observable_serena_route();
    test_missing_burnet_or_deck_payload_keeps_serena();
    test_missing_held_dragon_keeps_burnet_and_supporter_lock_blocks_both();
    test_live_item_outlet_still_preserves_supporter_action();
    test_registered_seed_19_uses_burnet_on_t2();
    std::cout << "Issue 2408 Burnet-before-Serena tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
