#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static Card choose_supporter_after_search_started(Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
  static bool should_play_steven(const Engine& engine) {
    return engine.should_play_steven();
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode locks, const std::uint64_t seed)
      : scenario{"issue-1351-no-discard-steven", sim::DciProfile::NoDiscardControl,
                 locks, true, 4},
        recipe{sim::baseline_recipe()},
        rng{seed},
        engine{scenario, recipe, rng} {}
};

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State turn_one_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::BrilliantBlender};
  state.deck = {
      sim::Card::StevensResolve,
      sim::Card::RegidragoVstar,
      sim::Card::LatiasEx,
      sim::Card::Grass,
      sim::Card::MegaDragonite,
      sim::Card::Arven,
  };
  return state;
}

sim::State turn_two_with_banked_payload() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0}};
  state.hand = {sim::Card::StevensResolve, sim::Card::Fire};
  state.deck = {
      sim::Card::RegidragoVstar,
      sim::Card::LatiasEx,
      sim::Card::Grass,
      sim::Card::Arven,
  };
  state.discard = {sim::Card::BrilliantBlender, sim::Card::MegaDragonite};
  return state;
}

void wonder_tag_banks_steven_before_no_control_blender() {
  Fixture fixture{sim::LockMode::None, 135101};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(engine, turn_one_state());

  // Wonder Tag may bank Steven on the first player's opening turn. Blender may then
  // bank the payload immediately under no-discard-control, while Steven reserves
  // VSTAR, Grass, and Latias on T2 for the deterministic T3 ready state:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // DCI/JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1351
  expect(sim::EngineTestAccess::choose_supporter_after_search_started(engine) ==
             sim::Card::StevensResolve,
         "Wonder Tag did not bank Steven for the no-discard T3 route");
}

void turn_two_steven_accepts_the_already_banked_payload() {
  Fixture fixture{sim::LockMode::None, 135102};
  sim::Engine& engine = fixture.engine;
  sim::EngineTestAccess::set_state(engine, turn_two_with_banked_payload());

  expect(sim::EngineTestAccess::should_play_steven(engine),
         "Banked payload did not preserve the T2 Steven continuation");
  sim::EngineTestAccess::run_turn(engine);

  const sim::State& state = sim::EngineTestAccess::state(engine);
  // The T1 Blender payload is public and complete. T2 attaches Fire before Steven,
  // then Steven reserves the three remaining T3 axes without requiring another
  // payload card or another copy of the singleton ACE SPEC:
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official attachment, Supporter, evolution, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1351
  expect(contains(state.hand, sim::Card::RegidragoVstar),
         "Steven did not reserve Regidrago VSTAR");
  expect(contains(state.hand, sim::Card::Grass),
         "Steven did not reserve the third Grass Energy");
  expect(contains(state.hand, sim::Card::LatiasEx),
         "Steven did not reserve Latias ex");
  expect(state.bench.front().grass == 1 && state.bench.front().fire == 1,
         "Fire was not attached before Steven ended T2");
  expect(state.turn_ended, "Steven's Resolve did not end T2");
}

void no_control_route_still_requires_a_real_banked_payload() {
  Fixture fixture{sim::LockMode::None, 135103};
  sim::Engine& engine = fixture.engine;
  sim::State state = turn_two_with_banked_payload();
  state.discard.clear();
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(!sim::EngineTestAccess::should_play_steven(engine),
         "Steven remained live without a held or banked payload route");
}

void exact_seed_reaches_turn_three() {
  const auto scenario = sim::scenario_by_label("no-discard-control/go-first");
  expect(scenario.has_value(), "Missing no-discard-control going-first scenario");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*scenario, recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();

  const auto has_line = [&trace](const std::string& first,
                                 const std::string& second) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&first, &second](const std::string& line) {
      return line.find(first) != std::string::npos &&
             line.find(second) != std::string::npos;
    });
  };

  // This exact source-bound seed previously chose Crispin and reached T4. The legal
  // K1 route banks Steven on T1, banks the payload with Blender, then reaches T3:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official rules: https://www.pokemon.com/us/pokemon-tcg/rules
  // Original strict-JIT route: https://github.com/FlareZ123/pokemon-sims/issues/946
  // Regression: https://github.com/FlareZ123/pokemon-sims/issues/1351
  expect(outcome.first_ready_turn == 3,
         "No-discard seed 1 did not reach the deterministic T3 state");
  expect(has_line("T1 | WONDER TAG", "Steven's Resolve"),
         "Seed 1 did not bank Steven on T1");
  expect(has_line("T1 | PLAY ITEM", "R-BLENDER-01"),
         "Seed 1 did not bank its payload with Blender on T1");
  expect(has_line("T2 | PLAY SUPPORTER", "Regidrago VSTAR, Grass Energy, Latias ex"),
         "Seed 1 did not search the complete Steven package on T2");
  expect(has_line("T3 | READY", "Active Regidrago VSTAR has GGF"),
         "Seed 1 did not reach readiness on T3");
}

}  // namespace

int main() {
  wonder_tag_banks_steven_before_no_control_blender();
  turn_two_steven_accepts_the_already_banked_payload();
  no_control_route_still_requires_a_real_banked_payload();
  exact_seed_reaches_turn_three();
  return 0;
}
