#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_state(Engine& engine, State state, const bool k1 = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool run_search_step(Engine& engine) {
    return engine.run_search_items_one_step(false);
  }
  static bool knows_deck(const Engine& engine) { return engine.deck_seen_; }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State prelock_state(const int payload_copies = 2,
                         const bool include_energy = true) {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0};
  state.hand = {sim::Card::EarthenVessel};
  for (int copy = 0; copy < payload_copies; ++copy) {
    state.hand.push_back(sim::Card::MegaDragonite);
  }
  if (include_energy) {
    state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoV,
                  sim::Card::RegidragoVstar};
  } else {
    state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                  sim::Card::LatiasEx};
  }
  state.prizes = {sim::Card::QuickBall, sim::Card::MysteriousTreasure,
                  sim::Card::Grass, sim::Card::Grass,
                  sim::Card::RegidragoV, sim::Card::RegidragoV};
  return state;
}

sim::Engine make_engine(sim::Scenario& scenario, std::mt19937_64& rng) {
  static const sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  return sim::Engine(scenario, recipe, rng);
}

void test_exact_public_route_spends_one_duplicate_and_establishes_k1() {
  sim::Scenario scenario{"issue-1757-exact", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 5};
  std::mt19937_64 rng{1757};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, prelock_state());

  // Earthen Vessel may discard one other card and search up to two Basic Energy.
  // Keeping the second identical Mega Dragonite ex protects the later payload axis,
  // while this legal T1 search establishes K1 before the scheduled Item lock:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, deck-search, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1, DCI, and scheduled-lock specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1757
  expect(sim::EngineTestAccess::run_search_step(engine),
         "The legal pre-lock Vessel route was not played.");
  const auto& state = sim::EngineTestAccess::state(engine);
  expect(sim::EngineTestAccess::knows_deck(engine),
         "The legal Vessel search did not establish K1.");
  expect(std::count(state.hand.begin(), state.hand.end(),
                    sim::Card::MegaDragonite) == 1,
         "The route did not preserve exactly one duplicate payload copy.");
  expect(std::count(state.discard.begin(), state.discard.end(),
                    sim::Card::MegaDragonite) == 1,
         "The route did not use the redundant payload as its realistic cost.");
  expect(std::count(state.hand.begin(), state.hand.end(), sim::Card::Grass) == 1 &&
             std::count(state.hand.begin(), state.hand.end(), sim::Card::Fire) == 1,
         "The route did not take both live Basic Energy targets.");
}

void test_singleton_payload_is_protected() {
  sim::Scenario scenario{"issue-1757-singleton", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 5};
  std::mt19937_64 rng{17570};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, prelock_state(1));

  // A sole payload is UDP/high-value here and cannot be converted into a speculative
  // information action under strict JIT: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Earthen Vessel cost: https://api.pokemontcg.io/v2/cards/sv4-163
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/1757
  expect(!sim::EngineTestAccess::run_search_step(engine),
         "The override spent the only protected payload copy.");
}

void test_known_empty_energy_targets_hold_vessel() {
  sim::Scenario scenario{"issue-1757-no-energy", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 5};
  std::mt19937_64 rng{17571};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, prelock_state(2, false), true);

  // Earthen Vessel searches Basic Energy from the deck, so K1 proof that no target
  // remains makes the discard strategically dominated: https://api.pokemontcg.io/v2/cards/sv4-163
  // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/1757
  expect(!sim::EngineTestAccess::run_search_step(engine),
         "The override spent Vessel with no searchable Energy.");
}

void test_unscheduled_or_already_live_lock_does_not_use_override() {
  for (const auto lock : {sim::LockMode::None, sim::LockMode::FullItem}) {
    sim::Scenario scenario{"issue-1757-lock-control", sim::DciProfile::StrictJit,
                           lock, false, 5};
    std::mt19937_64 rng{17572 + static_cast<unsigned>(lock)};
    sim::Engine engine = make_engine(scenario, rng);
    sim::EngineTestAccess::set_state(engine, prelock_state());

    // The override is limited to the use-or-lose T1 window before a scheduled T2
    // Item lock: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#turn-2-item-lock
    // Official Item procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/1757
    expect(!sim::EngineTestAccess::run_search_step(engine),
           "The pre-lock override leaked into another lock profile.");
  }
}

void test_direct_basic_search_keeps_priority() {
  sim::Scenario scenario{"issue-1757-direct-route", sim::DciProfile::StrictJit,
                         sim::LockMode::TurnTwoItem, false, 5};
  std::mt19937_64 rng{17573};
  sim::Engine engine = make_engine(scenario, rng);
  auto state = prelock_state();
  state.hand.push_back(sim::Card::BattleVipPass);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Battle VIP Pass directly establishes up to two Basic Pokémon on the opening
  // turn and therefore remains ahead of the information/Energy compression route:
  // Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Earliest complete route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug scope: https://github.com/FlareZ123/pokemon-sims/issues/1757
  expect(sim::EngineTestAccess::run_search_step(engine),
         "The direct Regidrago search route did not resolve.");
  const auto& after = sim::EngineTestAccess::state(engine);
  expect(std::count(after.hand.begin(), after.hand.end(), sim::Card::EarthenVessel) == 1,
         "The pre-lock Vessel override incorrectly preempted a direct Basic search.");
  expect(std::count(after.discard.begin(), after.discard.end(),
                    sim::Card::BattleVipPass) == 1,
         "The higher-priority direct Basic search did not spend Battle VIP Pass.");
  expect(std::count(after.discard.begin(), after.discard.end(),
                    sim::Card::MegaDragonite) == 0,
         "The direct route incorrectly consumed a protected payload copy.");
}

void test_registered_seed_57721_uses_vessel_before_lock() {
  const auto scenario = sim::scenario_by_label(
      "strict-jit-turn2-item-lock/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "Issue-1757 registered fixture is unavailable.");
  std::mt19937_64 rng{57721};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  static_cast<void>(engine.run());

  // Exact source-bound reproduction: https://github.com/FlareZ123/pokemon-sims/issues/1757
  // Earthen Vessel and duplicate payload data: https://api.pokemontcg.io/v2/cards/sv4-163 https://api.pokemontcg.io/v2/cards/me2pt5-152
  expect(std::any_of(trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
           return line.find("T1 | DECK KNOWLEDGE") != std::string::npos &&
               line.find("issue-1757 pre-lock route") != std::string::npos;
         }), "Seed 57721 still forfeits its legal T1 deck inspection.");
  expect(std::any_of(trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
           return line.find("T1 | DISCARD") != std::string::npos &&
               line.find("issue-1757 duplicate payload cost") != std::string::npos;
         }), "Seed 57721 did not use the redundant duplicate cost.");
}
}  // namespace

int main() {
  test_exact_public_route_spends_one_duplicate_and_establishes_k1();
  test_singleton_payload_is_protected();
  test_known_empty_energy_targets_hold_vessel();
  test_unscheduled_or_already_live_lock_does_not_use_override();
  test_direct_basic_search_keeps_priority();
  test_registered_seed_57721_uses_vessel_before_lock();
}
