#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }

  static const State& state(const Engine& engine) { return engine.state_; }
  static State& mutable_state(Engine& engine) { return engine.state_; }
  static bool should_bank_tate(const Engine& engine) {
    return engine.fss_should_bank_tate_for_next_turn_blender_route();
  }
  static Card fss_target(const Engine& engine) {
    return engine.fss_target_after_search_started();
  }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool evolve_best_regi(Engine& engine) { return engine.evolve_best_regi(); }
  static bool play_tate_switch(Engine& engine) { return engine.play_tate_switch(); }
  static bool play_blender(Engine& engine) { return engine.play_brilliant_blender(); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State banked_tate_state() {
  sim::State state;
  state.turn = 4;
  state.supporter_used = true;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 2, 2, 1,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::RegidragoVstar, sim::Card::BrilliantBlender,
                sim::Card::QuickBall, sim::Card::Dragapult};
  state.deck = {sim::Card::TateLiza, sim::Card::ProfessorBurnet,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.prizes = {sim::Card::LatiasEx, sim::Card::Crispin,
                  sim::Card::Arven, sim::Card::GoodraVstar,
                  sim::Card::Fire, sim::Card::Dipplin};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, sim::State state,
                        std::uint64_t seed) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  static std::mt19937_64 rng;
  rng.seed(seed);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_k1_state(engine, std::move(state));
  return engine;
}

void test_exact_k1_state_banks_tate_and_completes_t5() {
  const sim::Scenario scenario{"issue-1139-positive", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  sim::Engine engine = make_engine(scenario, banked_tate_state(), 113900);

  // Arven has consumed T4's Supporter play. Star Alchemy may search Tate & Liza
  // for T5, because the prior-turn GGF Regidrago V can evolve from the held VSTAR,
  // Tate can switch it Active, and held Brilliant Blender supplies the distinct
  // same-turn Dragon payload without using the T5 Supporter slot:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core turn, evolution, Supporter, Item, switch, and search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Earliest legal route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // T5 diagnostic recovery: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/T5_FAILURE_POLICY.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1139
  expect(sim::EngineTestAccess::should_bank_tate(engine),
         "The exact K1 state must admit the next-turn Tate plus Blender route.");
  expect(sim::EngineTestAccess::fss_target(engine) == sim::Card::TateLiza,
         "Star Alchemy must choose Tate instead of redundant Professor Burnet.");
  expect(sim::EngineTestAccess::use_fss(engine),
         "Star Alchemy must search the known Tate target.");
  expect(contains(sim::EngineTestAccess::state(engine).hand, sim::Card::TateLiza),
         "Tate must enter hand on T4.");
  expect(sim::EngineTestAccess::evolve_best_regi(engine),
         "The prior-turn GGF Regidrago V must evolve on T4.");

  sim::State& next = sim::EngineTestAccess::mutable_state(engine);
  next.turn = 5;
  next.supporter_used = false;
  next.retreat_used = false;
  next.turn_ended = false;
  next.discarded_this_turn.clear();

  expect(sim::EngineTestAccess::play_tate_switch(engine),
         "Tate must promote the evolved GGF Regidrago VSTAR on T5.");
  expect(sim::EngineTestAccess::play_blender(engine),
         "Held Blender must supply the T5 strict-JIT payload.");
  expect(sim::EngineTestAccess::state(engine).active &&
             sim::EngineTestAccess::state(engine).active->card == sim::Card::RegidragoVstar,
         "The powered Regidrago VSTAR must be Active.");
  expect(sim::EngineTestAccess::payload_ready(engine),
         "The payload must be in discard on the T5 ready turn.");
}

void expect_blocked(sim::State state, const sim::Scenario& scenario,
                    const std::uint64_t seed, const char* message) {
  sim::Engine engine = make_engine(scenario, std::move(state), seed);
  expect(!sim::EngineTestAccess::should_bank_tate(engine), message);
}

void test_exact_route_controls() {
  const sim::Scenario live{"issue-1139-controls", sim::DciProfile::StrictJit,
                           sim::LockMode::None, true, 5};

  sim::State no_tate = banked_tate_state();
  std::erase(no_tate.deck, sim::Card::TateLiza);
  expect_blocked(std::move(no_tate), live, 113901,
                 "A known unavailable Tate target must reject the route.");

  sim::State no_blender = banked_tate_state();
  std::erase(no_blender.hand, sim::Card::BrilliantBlender);
  expect_blocked(std::move(no_blender), live, 113902,
                 "The route requires held Brilliant Blender.");

  sim::State no_payload = banked_tate_state();
  std::erase(no_payload.deck, sim::Card::MegaDragonite);
  std::erase(no_payload.hand, sim::Card::Dragapult);
  no_payload.prizes.push_back(sim::Card::MegaDragonite);
  expect_blocked(std::move(no_payload), live, 113903,
                 "A known unavailable deck payload must reject Blender.");

  sim::State incomplete_energy = banked_tate_state();
  incomplete_energy.bench.front().grass = 1;
  expect_blocked(std::move(incomplete_energy), live, 113904,
                 "The banked route requires complete GGF before T5.");

  sim::State same_turn_regi = banked_tate_state();
  same_turn_regi.bench.front().entered_turn = 4;
  expect_blocked(std::move(same_turn_regi), live, 113905,
                 "A Regidrago V that entered this turn cannot justify the evolution route.");

  sim::State active_regi = banked_tate_state();
  active_regi.active = sim::Pokemon{sim::Card::RegidragoV, 2, 2, 1,
                                    sim::Tool::ForestSealStone};
  active_regi.bench.clear();
  expect_blocked(std::move(active_regi), live, 113906,
                 "An already-Active Regidrago route must not spend Star Alchemy on Tate.");

  sim::State live_latias = banked_tate_state();
  live_latias.deck.push_back(sim::Card::LatiasEx);
  expect_blocked(std::move(live_latias), live, 113907,
                 "A live current-turn Latias promotion must outrank banked Tate.");

  sim::State full_bench_latias = banked_tate_state();
  full_bench_latias.deck.push_back(sim::Card::LatiasEx);
  full_bench_latias.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  full_bench_latias.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  full_bench_latias.bench.push_back(sim::Pokemon{sim::Card::MawileGX, 1});
  full_bench_latias.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 1});
  sim::Engine full_bench_engine = make_engine(live, std::move(full_bench_latias), 113911);
  // A deck-resident Latias is not a live current-turn route when the Bench is full.
  // Tate can still switch the evolved attacker next turn without needing Bench space:
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Bench and switch procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/1139
  expect(sim::EngineTestAccess::should_bank_tate(full_bench_engine),
         "A full Bench must make the deck-resident Latias route unavailable.");

  const sim::Scenario item_lock{"issue-1139-item-lock", sim::DciProfile::StrictJit,
                                sim::LockMode::FullItem, true, 5};
  expect_blocked(banked_tate_state(), item_lock, 113908,
                 "Item lock must reject the promised Blender continuation.");

  const sim::Scenario no_horizon{"issue-1139-horizon", sim::DciProfile::StrictJit,
                                 sim::LockMode::None, true, 4};
  expect_blocked(banked_tate_state(), no_horizon, 113909,
                 "The route must fit inside the modeled horizon.");

  sim::State no_vstar = banked_tate_state();
  std::erase(no_vstar.hand, sim::Card::RegidragoVstar);
  expect_blocked(std::move(no_vstar), live, 113910,
                 "The next-turn route requires the VSTAR card already held.");
}

void test_seed_100_reaches_t4_via_quick_ball_tate() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  expect(scenario.has_value(), "Missing strict-JIT going-first scenario.");
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{100};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const bool discarded_payload_t4 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T4 | DISCARD") != std::string::npos &&
               line.find("R-QB-01") != std::string::npos &&
               line.find("Dragapult ex") != std::string::npos;
      });
  const bool searched_tate_t4 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T4 | WONDER TAG") != std::string::npos &&
               line.find("Tate & Liza") != std::string::npos;
      });
  const bool switched_t4 = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T4 | PLAY SUPPORTER") != std::string::npos &&
               line.find("R-TATE-01") != std::string::npos;
      });
  const bool spent_blender = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("R-BLENDER-01") != std::string::npos;
      });
  const bool preserved_resources = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("T4 | POLICY") != std::string::npos &&
               line.find("End:") != std::string::npos &&
               line.find("Arven") != std::string::npos &&
               line.find("Brilliant Blender") != std::string::npos;
      });

  // Once GGF, VSTAR, and the held Dragon are public on T4, Quick Ball can
  // discard Dragapult ex, search Tapu Lele-GX, and Wonder Tag for Tate & Liza.
  // Tate promotes the powered attacker that turn, reaching readiness one turn
  // earlier while preserving Arven and the singleton ACE SPEC:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core turn, Item, Ability, Supporter, and switch procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Original T5 recovery contract: https://github.com/FlareZ123/pokemon-sims/issues/1139
  // Resource-preservation fix: https://github.com/FlareZ123/pokemon-sims/issues/1343
  expect(outcome.first_ready_turn == 4,
         "Seed 100 must use the earliest legal T4 ready state.");
  expect(discarded_payload_t4,
         "Seed 100 must discard Dragapult with Quick Ball on T4.");
  expect(searched_tate_t4,
         "Seed 100 must use Wonder Tag for Tate & Liza on T4.");
  expect(switched_t4,
         "Seed 100 must use Tate switch mode on T4.");
  expect(!spent_blender,
         "Seed 100 must preserve Brilliant Blender.");
  expect(preserved_resources,
         "Seed 100 must preserve Arven and Brilliant Blender in the ready state.");
}

}  // namespace

int main() {
  test_exact_k1_state_banks_tate_and_completes_t5();
  test_exact_route_controls();
  test_seed_100_reaches_t4_via_quick_ball_tate();
  return 0;
}
