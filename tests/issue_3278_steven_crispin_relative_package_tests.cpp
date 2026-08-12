#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static State& state(Engine& engine) { return engine.state_; }
  static bool bank_available(const Engine& engine) {
    return engine.issue_1772_steven_t3_package_available();
  }
  static bool bank(Engine& engine) {
    return engine.play_issue_1772_steven_t3_package();
  }
  static bool completion_available(const Engine& engine) {
    return engine.issue_1772_t3_crispin_completion_available();
  }
  static void set_completion_turn(Engine& engine, const int turn) {
    engine.issue_1772_completion_turn_ = turn;
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None,
                       const int max_turn = 5) {
  return sim::Scenario{"issue-3278", sim::DciProfile::MatchupFlexJit,
                       lock, true, max_turn};
}

sim::State bank_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::DialgaGX, turn - 1};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, turn - 1, 0, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::StevensResolve, sim::Card::Gladion,
                sim::Card::EarthenVessel, sim::Card::RegidragoVstar,
                sim::Card::Crispin};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::LatiasEx,
                sim::Card::Dragapult, sim::Card::Arven};
  state.prizes = {sim::Card::BrilliantBlender};
  return state;
}

sim::State completion_state(const int turn) {
  sim::State state;
  state.turn = turn;
  state.active = sim::Pokemon{sim::Card::DialgaGX, turn - 2};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, turn - 2, 0, 1,
                              sim::Tool::None}};
  state.hand = {sim::Card::Gladion, sim::Card::EarthenVessel,
                sim::Card::RegidragoVstar, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::LatiasEx,
                sim::Card::Dragapult};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::Arven};
  state.prizes = {sim::Card::BrilliantBlender};
  return state;
}

sim::Engine engine_for(const sim::Scenario& selected, std::mt19937_64& rng) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  if (deck == nullptr) throw std::runtime_error("Registered shell unavailable");
  return sim::Engine(selected, deck->recipe, rng);
}

void advance_after_bank(sim::Engine& engine, const int next_turn) {
  sim::State& state = sim::EngineTestAccess::state(engine);
  state.turn = next_turn;
  state.turn_ended = false;
  state.supporter_used = false;
  state.manual_energy_used = false;
  state.discarded_this_turn.clear();
}

void test_historical_and_later_relative_pairs() {
  // Steven banks Grass, Latias ex, and a Dragon payload, then the immediately
  // following turn uses Crispin, a manual attachment, Earthen Vessel, evolution,
  // Skyliner, and Retreat. None of those printed effects names absolute T2 or T3.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Advanced procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // K1/JIT/priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original route / relative follow-up: https://github.com/FlareZ123/pokemon-sims/issues/1772 https://github.com/FlareZ123/pokemon-sims/issues/3278
  for (const int bank_turn : {2, 4}) {
    std::mt19937_64 rng(static_cast<std::uint64_t>(32780 + bank_turn));
    sim::Engine engine = engine_for(scenario(), rng);
    sim::EngineTestAccess::set_state(engine, bank_state(bank_turn));
    expect(sim::EngineTestAccess::bank_available(engine),
           "Legal relative Steven package was rejected");
    expect(sim::EngineTestAccess::bank(engine),
           "Legal relative Steven package did not bank");
    advance_after_bank(engine, bank_turn + 1);
    expect(sim::EngineTestAccess::completion_available(engine),
           "Immediately following banked completion was rejected");
  }
}

void test_completion_requires_fresh_bank_marker() {
  // The special completion must be provenance-bound to this exact Steven bank, so
  // an unrelated later hand state cannot activate the route.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Repository DCI/provenance model: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3278
  std::mt19937_64 rng(32781);
  sim::Engine engine = engine_for(scenario(), rng);
  sim::EngineTestAccess::set_state(engine, completion_state(5));
  expect(!sim::EngineTestAccess::completion_available(engine),
         "Completion activated without a prior bank");
  sim::EngineTestAccess::set_completion_turn(engine, 4);
  expect(!sim::EngineTestAccess::completion_available(engine),
         "Stale bank marker activated a later completion");
  sim::EngineTestAccess::set_completion_turn(engine, 5);
  expect(sim::EngineTestAccess::completion_available(engine),
         "Fresh relative bank marker did not admit completion");
}

void test_legality_and_resource_boundaries() {
  // Earthen Vessel must be legal on the projected completion turn, and Latias ex's
  // Skyliner must remain available. Supporter, Item, Ability, K1, evolution-age,
  // Bench, Retreat, Energy, and horizon controls therefore remain semantic gates.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Lock policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#scenario-lock-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3278
  const auto rejected_bank = [](sim::State state, const sim::Scenario selected,
                                const bool k1, const std::uint64_t seed,
                                const std::string& message) {
    std::mt19937_64 rng(seed);
    sim::Engine engine = engine_for(selected, rng);
    sim::EngineTestAccess::set_state(engine, std::move(state), k1);
    expect(!sim::EngineTestAccess::bank_available(engine), message);
  };

  rejected_bank(bank_state(2), scenario(sim::LockMode::FullSupporter), true,
                327820, "Supporter lock admitted Steven bank");
  rejected_bank(bank_state(2), scenario(sim::LockMode::FullItem), true,
                327821, "Item lock admitted Vessel continuation");
  rejected_bank(bank_state(1), scenario(sim::LockMode::TurnTwoItem), true,
                327822, "Projected persistent T2 Item lock admitted Vessel");
  rejected_bank(bank_state(2), scenario(sim::LockMode::FullRuleBoxAbility), true,
                327823, "Rule Box Ability lock admitted required Skyliner route");
  rejected_bank(bank_state(2), scenario(), false, 327824,
                "K0 admitted K1 Steven package");
  rejected_bank(bank_state(5), scenario(sim::LockMode::None, 5), true, 327825,
                "Exhausted next-turn horizon admitted bank");

  sim::State state = bank_state(2);
  state.bench.front().entered_turn = state.turn;
  rejected_bank(std::move(state), scenario(), true, 327826,
                "Same-turn Regidrago bypassed evolution age");
  state = bank_state(2);
  state.retreat_used = true;
  rejected_bank(std::move(state), scenario(), true, 327827,
                "Used Retreat admitted Skyliner continuation");
  state = bank_state(2);
  while (state.bench.size() < 5U) {
    state.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 1});
  }
  rejected_bank(std::move(state), scenario(), true, 327828,
                "Full Bench admitted Latias continuation");
  state = bank_state(2);
  state.bench.front().fire = 0;
  rejected_bank(std::move(state), scenario(), true, 327829,
                "Energy mismatch admitted prepared-Regidrago route");

  for (const sim::Card missing : {sim::Card::StevensResolve, sim::Card::Crispin,
                                  sim::Card::EarthenVessel,
                                  sim::Card::RegidragoVstar}) {
    state = bank_state(2);
    const auto it = std::find(state.hand.begin(), state.hand.end(), missing);
    state.hand.erase(it);
    rejected_bank(std::move(state), scenario(), true,
                  327830 + static_cast<std::uint64_t>(missing),
                  "Missing held route card admitted bank");
  }
}

void test_completion_item_and_supporter_locks() {
  for (const sim::LockMode lock : {sim::LockMode::FullItem,
                                   sim::LockMode::FullSupporter}) {
    std::mt19937_64 rng(327890 + static_cast<std::uint64_t>(lock));
    sim::Engine engine = engine_for(scenario(lock), rng);
    sim::EngineTestAccess::set_state(engine, completion_state(5));
    sim::EngineTestAccess::set_completion_turn(engine, 5);
    expect(!sim::EngineTestAccess::completion_available(engine),
           "Current completion legality lock was ignored");
  }
}

}  // namespace

int main() {
  test_historical_and_later_relative_pairs();
  test_completion_requires_fresh_bank_marker();
  test_legality_and_resource_boundaries();
  test_completion_item_and_supporter_locks();
  return 0;
}
