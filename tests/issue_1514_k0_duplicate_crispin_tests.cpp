#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool known = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = known;
    engine.prizes_revealed_ = known;
  }

  static bool hold_tapu(const Engine& engine) {
    return engine.issue_1514_hold_tapu_until_public_search();
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

const sim::NamedDeck& shell_deck() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "The registered Regidrago shell is unavailable.");
  return *deck;
}

sim::State public_k0_state(const bool regidrago_prized) {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.hand = {sim::Card::QuickBall, sim::Card::MysteriousTreasure,
                sim::Card::QuickBall, sim::Card::TapuLeleGX,
                sim::Card::Gladion, sim::Card::StevensResolve,
                sim::Card::Crispin};
  if (regidrago_prized) {
    state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoVstar};
    state.prizes = {sim::Card::RegidragoV};
  } else {
    state.deck = {sim::Card::RegidragoV, sim::Card::Fire,
                  sim::Card::RegidragoVstar};
    state.prizes = {sim::Card::Grass};
  }
  return state;
}

bool hold_from_state(sim::State state, const sim::LockMode lock,
                     const bool known, const std::uint64_t seed) {
  const sim::Scenario scenario{"issue-1514-public", sim::DciProfile::NoDiscardControl,
                               lock, false, 5};
  std::mt19937_64 rng{seed};
  sim::Engine engine(scenario, shell_deck().recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), known);
  return sim::EngineTestAccess::hold_tapu(engine);
}

void test_identical_public_k0_states_choose_search_first() {
  // These states have identical public zones. One hides Regidrago V in the deck and
  // one hides it in Prizes. The K0 action must be identical because Quick Ball's
  // legal cost and the possibility of an unseen Regidrago V are public facts:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Core search and Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Future-card-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1514
  expect(hold_from_state(public_k0_state(false), sim::LockMode::None,
                         false, 151401),
         "The public search-first action was not selected with Regidrago in deck.");
  expect(hold_from_state(public_k0_state(true), sim::LockMode::None,
                         false, 151402),
         "The K0 action changed when Regidrago moved to the hidden Prize zone.");
}

void test_public_boundaries_keep_live_wonder_tag_routes() {
  sim::State no_crispin = public_k0_state(false);
  no_crispin.hand.erase(std::find(no_crispin.hand.begin(), no_crispin.hand.end(),
                                  sim::Card::Crispin));
  expect(!hold_from_state(std::move(no_crispin), sim::LockMode::None,
                          false, 151403),
         "Tapu was held without a duplicate held Crispin route.");

  sim::State no_quick_ball = public_k0_state(false);
  no_quick_ball.hand.erase(
      std::remove(no_quick_ball.hand.begin(), no_quick_ball.hand.end(),
                  sim::Card::QuickBall),
      no_quick_ball.hand.end());
  expect(!hold_from_state(std::move(no_quick_ball), sim::LockMode::None,
                          false, 151404),
         "Tapu was held without a legal deck-search Item.");

  sim::State known = public_k0_state(false);
  expect(!hold_from_state(std::move(known), sim::LockMode::None,
                          true, 151405),
         "The K0 preflight remained active after K1 was established.");

  expect(!hold_from_state(public_k0_state(false), sim::LockMode::FullItem,
                          false, 151406),
         "Item lock admitted the Quick Ball search-first route.");

  sim::State regi_in_play = public_k0_state(false);
  regi_in_play.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 0});
  // A Benched Regidrago V has established the modeled Regidrago axis under the
  // core Bench procedure, so the K0 search-first hold has no remaining purpose:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1514
  expect(!hold_from_state(std::move(regi_in_play), sim::LockMode::None,
                          false, 151407),
         "Tapu was held after the Regidrago axis was already in play.");
}

struct SeedResult {
  sim::TrialOutcome outcome;
  sim::TraceLog trace;
};

SeedResult run_seed(const std::string& scenario_label,
                    const std::uint64_t seed) {
  const auto scenario = sim::scenario_by_label(scenario_label);
  expect(scenario.has_value(), "The registered issue-1514 scenario is unavailable.");
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, shell_deck().recipe, rng, &trace);
  return {engine.run(), std::move(trace)};
}

void test_seed_33_searches_first_and_reaches_t3() {
  const SeedResult result = run_seed("no-discard-control/go-second", 33);

  // Quick Ball establishes K1 before the Tapu decision. Held Crispin covers the
  // Energy Supporter, while K1 lets held Gladion use the known prized Grass Energy:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Knowledge and connector policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1514
  expect(result.outcome.first_ready_turn == 3 && !result.outcome.setup_failed,
         "Seed 33 did not improve from T4 to the legal T3 route.");
  expect(trace_contains(result.trace, "T1 | HOLD TAPU LELE-GX") &&
             trace_contains(result.trace, "T1 | PLAY ITEM") &&
             trace_contains(result.trace, "Quick Ball") &&
             trace_contains(result.trace, "T3 | READY"),
         "Seed 33 did not record the public search-first T3 route.");
  expect(!trace_contains(result.trace, "T1 | WONDER TAG"),
         "Seed 33 still spent the T1 Wonder Tag on duplicate Crispin.");
}

void test_distinct_tapu_routes_remain_live() {
  const SeedResult seed_104 = run_seed("strict-jit/go-first", 104);
  // Quick Ball into Tapu remains live when Crispin is not already held:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // Existing route: https://github.com/FlareZ123/pokemon-sims/issues/962
  expect(seed_104.outcome.first_ready_turn == 3 &&
             trace_contains(seed_104.trace, "WONDER TAG") &&
             trace_contains(seed_104.trace, "Crispin"),
         "The distinct seed-104 Tapu-Crispin route was suppressed.");

  const SeedResult seed_43 = run_seed("strict-jit/go-first", 43);
  // The established Treasure-Tapu connector must preserve its T2 deadline:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // Existing route: https://github.com/FlareZ123/pokemon-sims/issues/1209
  expect(seed_43.outcome.first_ready_turn == 2 &&
             trace_contains(seed_43.trace, "T2 | WONDER TAG") &&
             trace_contains(seed_43.trace, "T2 | READY"),
         "The distinct seed-43 Treasure-Tapu route was suppressed.");
}
}  // namespace

int main() {
  test_identical_public_k0_states_choose_search_first();
  test_public_boundaries_keep_live_wonder_tag_routes();
  test_seed_33_searches_first_and_reaches_t3();
  test_distinct_tapu_routes_remain_live();
  return 0;
}
