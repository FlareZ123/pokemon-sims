#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static State& state(Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) {
    return engine.outcome_;
  }
  static bool play_secret_box(Engine& engine) {
    return engine.play_secret_box();
  }
  static void run_secret_box_turn(Engine& engine) {
    engine.run_secret_box_turn();
  }
  static bool play_combo_ultra_ball(Engine& engine) {
    return engine.play_ultra_ball_for_forretress_combo();
  }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State complete_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::SecretBox, sim::Card::Grant,
                sim::Card::WishfulBaton, sim::Card::ErikasInvitation};
  state.deck = {
      sim::Card::MysteriousTreasure, sim::Card::ForestSealStone,
      sim::Card::Dawn, sim::Card::ForestOfVitality, sim::Card::Pineco,
      sim::Card::ForretressEx, sim::Card::Dragapult,
      sim::Card::RegidragoVstar, sim::Card::Fire,
      sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
      sim::Card::Grass, sim::Card::Grass,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode locks = sim::LockMode::None,
                   const sim::DciProfile dci = sim::DciProfile::StrictJit)
      : scenario{"issue-1118/exact", dci, locks, false, 5},
        recipe(sim::pineco_recipe()),
        rng(1118),
        trace{true, {}},
        engine(scenario, recipe, rng, &trace) {}
};

void test_registry_and_recipe_validation() {
  const auto& decks = sim::deck_registry();
  if (decks.size() != 2U || sim::deck_by_id("regidrago-shell") == nullptr ||
      sim::deck_by_id("regidrago-pineco") == nullptr ||
      sim::deck_by_id("regidrago-pineco-secret-box") == nullptr ||
      sim::deck_by_id("regidrago-pineco-blender") != nullptr) {
    throw std::runtime_error("The issue-1118 two-deck registry is incorrect.");
  }
  for (const sim::NamedDeck& deck : decks) {
    std::string error;
    if (!sim::validate_recipe(deck, &error)) {
      throw std::runtime_error("A registered issue-1118 recipe is invalid: " +
                               error);
    }
  }

  sim::NamedDeck short_deck{"short", sim::pineco_recipe()};
  --short_deck.recipe.front().second;
  if (sim::validate_recipe(short_deck)) {
    throw std::runtime_error("A 59-card recipe passed validation.");
  }
  sim::NamedDeck too_many{"too-many", sim::pineco_recipe()};
  too_many.recipe.emplace_back(sim::Card::Grant, 4);
  if (sim::validate_recipe(too_many)) {
    throw std::runtime_error("A five-copy non-Energy recipe passed validation.");
  }
  sim::NamedDeck two_aces{"two-aces", sim::pineco_recipe()};
  for (auto& [card, copies] : two_aces.recipe) {
    if (card == sim::Card::Grant) {
      card = sim::Card::BrilliantBlender;
      break;
    }
  }
  if (sim::validate_recipe(two_aces)) {
    throw std::runtime_error("A two-ACE-SPEC recipe passed validation.");
  }
}

void test_appletun_identity_and_shell_isolation() {
  if (sim::name(sim::Card::Appletun) != "Appletun sv8-140" ||
      sim::is_basic(sim::Card::Appletun) ||
      !sim::is_pokemon(sim::Card::Appletun) ||
      !sim::is_dragon_or_psychic(sim::Card::Appletun) ||
      !sim::is_payload(sim::Card::Appletun)) {
    throw std::runtime_error("Appletun sv8-140 classification is incorrect.");
  }
  const sim::NamedDeck* shell = sim::deck_by_id("regidrago-shell");
  const sim::NamedDeck* pineco = sim::deck_by_id("regidrago-pineco");
  const auto copies = [](const sim::DeckRecipe& recipe, const sim::Card card) {
    for (const auto& [candidate, count] : recipe) {
      if (candidate == card) return count;
    }
    return 0;
  };
  if (shell == nullptr || pineco == nullptr ||
      copies(shell->recipe, sim::Card::SecretBox) != 0 ||
      copies(shell->recipe, sim::Card::Pineco) != 0 ||
      copies(pineco->recipe, sim::Card::SecretBox) != 1 ||
      copies(pineco->recipe, sim::Card::Appletun) != 1 ||
      copies(pineco->recipe, sim::Card::BrilliantBlender) != 0) {
    throw std::runtime_error("Shell and Pineco recipe capabilities leaked.");
  }
}

void test_complete_secret_box_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, complete_route_state());
  sim::EngineTestAccess::run_secret_box_turn(fixture.engine);
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  const sim::TrialOutcome& outcome =
      sim::EngineTestAccess::outcome(fixture.engine);
  if (!state.active || state.active->card != sim::Card::RegidragoVstar ||
      state.active->grass < 2 || state.active->fire < 1 ||
      !contains(state.discard, sim::Card::Dragapult) ||
      !contains(state.discard, sim::Card::ForretressEx) ||
      !contains(state.discard, sim::Card::Pineco) ||
      !outcome.used_secret_box || !outcome.used_exploding_energy ||
      !outcome.used_fss) {
    throw std::runtime_error(
        "The exact Secret Box, Dawn, Forest, Treasure, FSS, and Forretress "
        "route did not complete.");
  }
}

void test_secret_box_cost_reservation_and_dci() {
  Fixture insufficient;
  sim::State state = complete_route_state();
  state.hand = {sim::Card::SecretBox, sim::Card::Grant,
                sim::Card::WishfulBaton};
  sim::EngineTestAccess::set_state(insufficient.engine, state);
  if (sim::EngineTestAccess::play_secret_box(insufficient.engine) ||
      sim::EngineTestAccess::outcome(insufficient.engine)
              .secret_box_cost_blocked != 1U) {
    throw std::runtime_error("Secret Box played with fewer than three costs.");
  }

  Fixture protected_cost;
  state = complete_route_state();
  state.hand = {sim::Card::SecretBox, sim::Card::Grant,
                sim::Card::WishfulBaton, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(protected_cost.engine, state);
  if (sim::EngineTestAccess::play_secret_box(protected_cost.engine) ||
      sim::EngineTestAccess::outcome(protected_cost.engine)
              .secret_box_cost_blocked != 1U) {
    throw std::runtime_error(
        "Strict DCI spent a protected VSTAR as the third Secret Box cost.");
  }

  Fixture reserved_treasure_cost;
  sim::EngineTestAccess::set_state(reserved_treasure_cost.engine,
                                   complete_route_state());
  sim::EngineTestAccess::run_secret_box_turn(reserved_treasure_cost.engine);
  const sim::State& final_state =
      sim::EngineTestAccess::state(reserved_treasure_cost.engine);
  if (!contains(final_state.discard, sim::Card::Dragapult) ||
      !contains(final_state.discard, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error(
        "Secret Box failed to reserve Dragapult for Treasure's extra cost.");
  }
}

void expect_seeded_route(const std::uint64_t seed,
                         const std::vector<std::string>& required_lines) {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("The seeded issue-1118 route fixture is unavailable.");
  }
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  if (outcome.first_ready_turn != 2) {
    throw std::runtime_error("A reviewed issue-1118 seed lost T2 readiness: " +
                             std::to_string(seed));
  }
  const auto has_line = [&trace](const std::string& required) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&required](const std::string& line) {
      return line.find(required) != std::string::npos;
    });
  };
  for (const std::string& required : required_lines) {
    if (!has_line(required)) {
      throw std::runtime_error("Reviewed issue-1118 seed " +
                               std::to_string(seed) +
                               " lost trace evidence: " + required);
    }
  }
}

void test_reviewed_seeded_routes() {
  // Seed 35 proves strict DCI may spend Grant, Wishful Baton, and a
  // dynamically replaceable search Item, then use Dawn, Forest, Treasure,
  // and Forretress. The exact third cost is intentionally policy-flexible.
  expect_seeded_route(35, {
      "Grant (Secret Box cost)",
      "Wishful Baton (Secret Box cost)",
      "Secret Box discarded three other cards",
      "Dawn searched and revealed: Dragapult ex",
      "Forest of Vitality allowed the newly played Pineco",
      "Mysterious Treasure cost",
      "T2 | READY",
  });

  // Seed 18 proves an Arven-banked Box route may use Exploding Energy to fund
  // an Active Tapu Lele-GX retreat before the Treasure completion.
  expect_seeded_route(18, {
      "Arven banked the next-turn Secret Box route",
      "Used Grass Energy attached by Exploding Energy to pay",
      "Mysterious Treasure used the proven dead or payload cost",
      "T2 | READY",
  });

  // Seed 40 proves a legal deck inspection exposes prized Secret Box, Gladion
  // recovers it, and the next turn executes the strict-JIT Pineco chain.
  expect_seeded_route(40, {
      "Wonder Tag: deck inspected",
      "Gladion recovered the legally known prized Secret Box",
      "Secret Box discarded three other cards",
      "T2 | READY",
  });
}

void test_combo_ultra_ball_fallback_includes_appletun() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::UltraBall, sim::Card::Grant,
                sim::Card::WishfulBaton};
  state.deck = {sim::Card::Appletun};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Ultra Ball pays two cards, legally inspects the deck, and must take the only
  // remaining Pokémon, Appletun, after the preferred Pineco target is absent:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/1299
  if (!sim::EngineTestAccess::play_combo_ultra_ball(fixture.engine)) {
    throw std::runtime_error("The combo Ultra Ball route did not begin.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun)) {
    throw std::runtime_error(
        "The combo Ultra Ball fallback left the only legal Pokémon in deck.");
  }

  Fixture no_pokemon;
  state = sim::State{};
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::UltraBall, sim::Card::Grant,
                sim::Card::WishfulBaton};
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(no_pokemon.engine, std::move(state));
  if (!sim::EngineTestAccess::play_combo_ultra_ball(no_pokemon.engine) ||
      contains(sim::EngineTestAccess::state(no_pokemon.engine).hand,
               sim::Card::Appletun)) {
    throw std::runtime_error("The no-Pokémon combo fallback control diverged.");
  }
}

void test_route_lock_and_bench_controls() {
  Fixture no_bench;
  sim::State state = complete_route_state();
  for (int index = 0; index < 5; ++index) {
    state.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  }
  sim::EngineTestAccess::set_state(no_bench.engine, state);
  if (sim::EngineTestAccess::play_secret_box(no_bench.engine) ||
      sim::EngineTestAccess::outcome(no_bench.engine)
              .secret_box_bench_blocked != 1U) {
    throw std::runtime_error("Secret Box ignored a full Bench.");
  }

  Fixture first_turn;
  state = complete_route_state();
  state.turn = 1;
  sim::EngineTestAccess::set_state(first_turn.engine, state);
  if (sim::EngineTestAccess::play_secret_box(first_turn.engine)) {
    throw std::runtime_error("The first turn admitted the Forest evolution route.");
  }

  for (const sim::LockMode lock : {sim::LockMode::FullItem,
                                   sim::LockMode::FullRuleBoxAbility,
                                   sim::LockMode::FullCombined,
                                   sim::LockMode::FullSupporter}) {
    Fixture locked(lock);
    sim::EngineTestAccess::set_state(locked.engine, complete_route_state());
    if (sim::EngineTestAccess::play_secret_box(locked.engine)) {
      throw std::runtime_error("A lock admitted the complete Secret Box route.");
    }
  }
}

}  // namespace

int main() {
  test_registry_and_recipe_validation();
  test_appletun_identity_and_shell_isolation();
  test_complete_secret_box_route();
  test_secret_box_cost_reservation_and_dci();
  test_reviewed_seeded_routes();
  test_combo_ultra_ball_fallback_includes_appletun();
  test_route_lock_and_bench_controls();
}
