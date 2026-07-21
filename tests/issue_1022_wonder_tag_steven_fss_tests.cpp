#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static bool t4_route(const Engine& engine) {
    return engine.wonder_tag_can_bank_steven_for_known_t4_fss_route();
  }
  static void mark_t4_route_banked(Engine& engine) {
    engine.issue_1022_banked_route_ = true;
  }
  static bool banked_t4_route(const Engine& engine) {
    return engine.banked_steven_has_known_t4_fss_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1}};
  state.hand = {sim::Card::Arven, sim::Card::BrilliantBlender,
                sim::Card::MegaDragonite, sim::Card::Grass};
  state.deck = {sim::Card::StevensResolve, sim::Card::RegidragoV,
                sim::Card::Crispin, sim::Card::Grass, sim::Card::Fire,
                sim::Card::ForestSealStone, sim::Card::RegidragoVstar,
                sim::Card::MysteriousTreasure};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  return sim::Engine(scenario, recipe, rng);
}

void test_exact_k1_route_selects_steven() {
  const sim::Scenario scenario{"issue-1022-positive", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1022};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, route_state());

  // Steven reserves the missing Regidrago V, Crispin, and Grass package. Held Arven
  // later finds Forest Seal Stone, which supplies Star Alchemy for the VSTAR axis:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1022
  expect(sim::EngineTestAccess::t4_route(engine),
         "The exact K1 missing-Basic Steven-FSS route must be admitted.");
}

void test_missing_route_components_reject_projection() {
  const sim::Scenario scenario{"issue-1022-components", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  for (const sim::Card missing : {sim::Card::ForestSealStone, sim::Card::RegidragoVstar,
                                  sim::Card::Fire}) {
    std::mt19937_64 rng{1023 + static_cast<unsigned>(missing)};
    sim::Engine engine = make_engine(scenario, rng);
    sim::State state = route_state();
    std::erase(state.deck, missing);
    sim::EngineTestAccess::set_state(engine, std::move(state));
    expect(!sim::EngineTestAccess::t4_route(engine),
           "A missing deterministic route component must reject the projection.");
  }

  std::mt19937_64 rng{2022};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = route_state();
  std::erase(state.hand, sim::Card::Arven);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  // Arven is the deterministic Tool bridge and cannot be replaced by an assumed draw:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/issues/1022
  expect(!sim::EngineTestAccess::t4_route(engine),
         "Missing held Arven must reject the deterministic route.");
}

void test_active_path_rejects_wonder_tag_projection() {
  const sim::Scenario scenario{"issue-1022-rulebox-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, true, 5};
  std::mt19937_64 rng{1024};
  sim::Engine engine = make_engine(scenario, rng);
  sim::EngineTestAccess::set_state(engine, route_state());

  // Active Path suppresses Tapu Lele-GX's Wonder Tag, which blocks the projection.
  // Forest Seal Stone remains usable through Path because Star Alchemy is an Ability
  // the Pokémon V can use rather than an Ability that Pokémon has:
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // TPCi Rules Team ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
  expect(!sim::EngineTestAccess::t4_route(engine),
         "Active Path must reject a route that starts with Wonder Tag.");
}

void test_removed_path_admits_projection_and_banked_continuation() {
  const sim::Scenario scenario{"issue-1195-path-removed", sim::DciProfile::StrictJit,
                               sim::LockMode::FullRuleBoxAbility, true, 5};
  std::mt19937_64 rng{1195};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State state = route_state();
  state.path_lock_removed = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Field Blower discards Path, restoring Wonder Tag. The remaining T4 Forest Seal
  // Stone route is legal because Star Alchemy is usable even through Path itself:
  // Field Blower: https://api.pokemontcg.io/v2/cards/sm2-125
  // Path to the Peak: https://api.pokemontcg.io/v2/cards/swsh6-148
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Forest Seal Stone ruling: https://compendium.pokegym.net/category/5-trainers/forest-seal-stone/
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
  expect(sim::EngineTestAccess::t4_route(engine),
         "A removed Path must admit the Wonder Tag Steven-FSS projection.");

  sim::State banked;
  banked.turn = 2;
  banked.path_lock_removed = true;
  banked.active = sim::Pokemon{sim::Card::Oricorio, 1};
  banked.active->grass = 1;
  banked.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1}};
  banked.hand = {sim::Card::StevensResolve, sim::Card::Arven,
                 sim::Card::BrilliantBlender, sim::Card::MegaDragonite};
  banked.deck = {sim::Card::RegidragoV, sim::Card::Crispin, sim::Card::Grass,
                 sim::Card::Fire, sim::Card::ForestSealStone,
                 sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(banked));
  sim::EngineTestAccess::mark_t4_route_banked(engine);
  expect(sim::EngineTestAccess::banked_t4_route(engine),
         "A removed Path must preserve the already banked Steven continuation.");
}

void test_future_item_locks_reject_projection() {
  for (const sim::LockMode lock : {sim::LockMode::TurnTwoItem,
                                   sim::LockMode::FullItem,
                                   sim::LockMode::FullCombined}) {
    const sim::Scenario scenario{"issue-1195-item-lock", sim::DciProfile::StrictJit,
                                 lock, true, 5};
    std::mt19937_64 rng{1196 + static_cast<unsigned>(lock)};
    sim::Engine engine = make_engine(scenario, rng);
    sim::State state = route_state();
    state.path_lock_removed = true;
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // The projection needs Forest Seal Stone and Mysterious Treasure on T4, so both
    // scheduled and full Item locks remain hard blockers:
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Core Item procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1195
    expect(!sim::EngineTestAccess::t4_route(engine),
           "Future Item lock must reject the T4 Item-dependent projection.");
  }
}

void test_bench_capacity_and_faster_basic_route_are_preserved() {
  const sim::Scenario scenario{"issue-1022-capacity", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  std::mt19937_64 rng{1025};
  sim::Engine engine = make_engine(scenario, rng);
  sim::State full = route_state();
  while (full.bench.size() < 5) {
    full.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 1});
  }
  sim::EngineTestAccess::set_state(engine, std::move(full));
  // Regidrago V must have a legal Bench slot before the route may spend Wonder Tag:
  // https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1022
  expect(!sim::EngineTestAccess::t4_route(engine),
         "A full Bench must reject the missing-Basic projection.");

  std::mt19937_64 rng2{1026};
  sim::Engine faster = make_engine(scenario, rng2);
  sim::State held_basic = route_state();
  held_basic.hand.push_back(sim::Card::RegidragoV);
  sim::EngineTestAccess::set_state(faster, std::move(held_basic));
  // A held Basic already establishes the missing axis and keeps the broader selector
  // free to choose a faster route: https://api.pokemontcg.io/v2/cards/swsh12-135
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1022
  expect(!sim::EngineTestAccess::t4_route(faster),
         "A held Regidrago V must suppress the missing-Basic Steven projection.");
}

void test_seed_293_reaches_turn_four() {
  const sim::Scenario scenario{"strict-jit/go-first", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{293};
  sim::TraceLog trace;
  trace.enabled = true;
  sim::Engine engine(scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Exact source-bound regression. Wonder Tag banks Steven, the opening Grass pays
  // Oricorio's future Retreat Cost, Steven establishes the Basic package, and T4
  // Arven plus Forest Seal Stone reaches the VSTAR before a current-turn Item cost
  // discards the Dragon payload:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1022
  expect(trace_contains(trace, "T1 | WONDER TAG | rules: R-TAPU-01 | Searched and revealed Steven's Resolve"),
         "Seed 293 must bank Steven instead of Crispin.");
  expect(trace_contains(trace, "T1 | ATTACH | rules: R-GAME-ENERGY | Grass Energy manually to Oricorio"),
         "Seed 293 must reserve Oricorio's retreat payment on T1.");
  expect(trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-STEVEN-01"),
         "Seed 293 must resolve the banked Steven on T2.");
  expect(trace_contains(trace, "T4 | STAR ALCHEMY | rules: R-FSS-02; R-VSTAR-01"),
         "Seed 293 must use Forest Seal Stone on T4.");
  expect(trace_contains(trace, "T4 | RETREAT | rules: R-GAME-RETREAT | Paid Oricorio's one-Energy Retreat Cost"),
         "Seed 293 must pay Oricorio's Retreat Cost on T4.");
  expect(trace_contains(trace, "T4 | READY"),
         "Seed 293 must reach readiness on T4.");
  expect(outcome.ready_by_4 && outcome.first_ready_turn == 4,
         "Seed 293 must first become ready on T4.");
}
}  // namespace

int main() {
  test_exact_k1_route_selects_steven();
  test_missing_route_components_reject_projection();
  test_active_path_rejects_wonder_tag_projection();
  test_removed_path_admits_projection_and_banked_continuation();
  test_future_item_locks_reject_projection();
  test_bench_capacity_and_faster_basic_route_are_preserved();
  test_seed_293_reaches_turn_four();
  return 0;
}
