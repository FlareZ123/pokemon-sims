#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static std::optional<std::array<Card, 3>> secret_box_cost_plan(
      const Engine& engine) {
    return engine.secret_box_cost_plan();
  }
  static bool play_secret_box(Engine& engine) {
    return engine.play_secret_box();
  }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool plan_contains(const std::array<sim::Card, 3>& plan,
                   const sim::Card card) {
  return std::find(plan.begin(), plan.end(), card) != plan.end();
}

sim::State seed_35_pre_box_state() {
  sim::State state;
  state.turn = 2;
  state.active =
      sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1},
      sim::Pokemon{sim::Card::Pineco, 1},
  };
  state.hand = {
      sim::Card::MysteriousTreasure,
      sim::Card::RegidragoVstar,
      sim::Card::Fire,
      sim::Card::ForretressEx,
      sim::Card::SecretBox,
      sim::Card::Grant,
      sim::Card::WishfulBaton,
      sim::Card::EarthenVessel,
  };
  state.deck = {
      sim::Card::QuickBall,
      sim::Card::MysteriousTreasure,
      sim::Card::Dawn,
      sim::Card::Dragapult,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode locks = sim::LockMode::None)
      : scenario{"issue-1389/exact", sim::DciProfile::StrictJit, locks,
                 false, 5},
        recipe(sim::pineco_recipe()),
        rng(1389),
        engine(scenario, recipe, rng) {}
};

void test_dead_vessel_precedes_live_treasure() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, seed_35_pre_box_state());

  // Secret Box must preserve the already-held Treasure that supplies the
  // current-turn Dragon discard, because held Fire plus Exploding Energy make
  // Earthen Vessel redundant in this public route:
  // https://api.pokemontcg.io/v2/cards/sv6-163
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1389
  const auto plan =
      sim::EngineTestAccess::secret_box_cost_plan(fixture.engine);
  if (!plan || !plan_contains(*plan, sim::Card::EarthenVessel) ||
      plan_contains(*plan, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error(
        "Secret Box did not prefer the proven-dead Earthen Vessel.");
  }

  if (!sim::EngineTestAccess::play_secret_box(fixture.engine)) {
    throw std::runtime_error("The confirmed Secret Box route did not resolve.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.discard, sim::Card::EarthenVessel) ||
      contains(after.discard, sim::Card::MysteriousTreasure) ||
      !contains(after.hand, sim::Card::MysteriousTreasure) ||
      !contains(after.hand, sim::Card::QuickBall) ||
      !contains(after.hand, sim::Card::Dawn)) {
    throw std::runtime_error(
        "The corrected Secret Box route failed to preserve Treasure and bank "
        "the independent Item channel.");
  }
}

void test_vessel_stays_protected_when_fire_is_missing() {
  Fixture fixture;
  sim::State state = seed_35_pre_box_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::Fire));
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Without held Fire or another proven Fire connector, Earthen Vessel remains
  // a live Energy axis and cannot become the preferred cost:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/1389
  const auto plan =
      sim::EngineTestAccess::secret_box_cost_plan(fixture.engine);
  if (!plan || plan_contains(*plan, sim::Card::EarthenVessel)) {
    throw std::runtime_error(
        "Secret Box spent Earthen Vessel while Fire remained missing.");
  }
}

void test_ability_lock_blocks_the_forretress_replacement_proof() {
  Fixture fixture(sim::LockMode::FullRuleBoxAbility);
  sim::EngineTestAccess::set_state(fixture.engine, seed_35_pre_box_state());

  // Rule Box Ability lock prevents Exploding Energy, so the route must not use
  // the no-lock replacement proof to spend Earthen Vessel:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://api.pokemontcg.io/v2/cards/swsh6-148
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#rule-box-ability-lock
  // https://github.com/FlareZ123/pokemon-sims/issues/1389
  if (sim::EngineTestAccess::play_secret_box(fixture.engine)) {
    throw std::runtime_error(
        "Ability lock admitted the no-lock Secret Box replacement proof.");
  }
}

void test_seed_35_trace_preserves_the_original_treasure() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-pineco");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("The issue-1389 seed fixture is unavailable.");
  }

  std::mt19937_64 rng(35);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();
  if (outcome.first_ready_turn != 2) {
    throw std::runtime_error("Seed 35 lost its legal T2 ready state.");
  }

  const auto has_line = [&trace](const std::string& fragment) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&fragment](const std::string& line) {
                         return line.find(fragment) != std::string::npos;
                       });
  };

  // Issue #1420 found a same-deadline route that preserves the entire Secret
  // Box package. The original #1389 exact-state tests above still constrain Box
  // cost ordering whenever Box is used; this full trace now requires the stronger
  // Steven -> Crispin -> Treasure route and the same strict-JIT T2 result:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://github.com/FlareZ123/pokemon-sims/issues/1389
  // https://github.com/FlareZ123/pokemon-sims/issues/1420
  if (!has_line("banked Crispin and Dragapult ex") ||
      has_line("Earthen Vessel (Secret Box cost)") ||
      has_line("T2 | EXPLODING ENERGY") ||
      !has_line("Dragapult ex (Mysterious Treasure cost)") ||
      !has_line("T2 | READY")) {
    throw std::runtime_error(
        "Seed 35 lost the stronger resource-preserving trace.");
  }
}

}  // namespace

int main() {
  test_dead_vessel_precedes_live_treasure();
  test_vessel_stays_protected_when_fire_is_missing();
  test_ability_lock_blocks_the_forretress_replacement_proof();
  test_seed_35_trace_preserves_the_original_treasure();
}
