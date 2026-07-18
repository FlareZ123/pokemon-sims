#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_opening_active(Engine& engine) {
    engine.choose_opening_active();
  }
};

}  // namespace sim

namespace {

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(const sim::LockMode lock, const std::uint64_t seed)
      : scenario{"issue-674-lock-opening", sim::DciProfile::StrictJit,
                 lock, false, 4},
        rng(seed),
        engine(scenario, recipe, rng) {}
};

sim::State exact_hand() {
  sim::State state;
  state.hand = {sim::Card::QuickBall, sim::Card::TeamYellsCheer,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::Powerglass,
                sim::Card::RegidragoVstar};
  return state;
}

sim::Card choose(const sim::LockMode lock, sim::State state,
                 const std::uint64_t seed) {
  Fixture fixture{lock, seed};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(fixture.engine);
  const auto& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active) throw std::runtime_error("opening selector left no Active");
  return after.active->card;
}

void exact_graph_preserves_vital_dance() {
  const std::array locks{sim::LockMode::TurnTwoItem,
                         sim::LockMode::FullRuleBoxAbility,
                         sim::LockMode::FullItem,
                         sim::LockMode::FullCombined};
  for (std::size_t index = 0; index < locks.size(); ++index) {
    Fixture fixture{locks[index], 67400U + index};
    sim::EngineTestAccess::set_state(fixture.engine, exact_hand());

    // Setup may start Dialga-GX while Oricorio remains in hand. Vital Dance
    // triggers only when Oricorio is played from hand, and Oricorio has no Rule
    // Box, so Path-style Rule Box Ability suppression does not disable it:
    // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/swsh6-148
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://api.pokemontcg.io/v2/cards/me2pt5-152
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/674
    sim::EngineTestAccess::choose_opening_active(fixture.engine);
    const auto& after = sim::EngineTestAccess::state(fixture.engine);
    if (!after.active || after.active->card != sim::Card::DialgaGX ||
        std::find(after.hand.begin(), after.hand.end(), sim::Card::Oricorio) ==
            after.hand.end()) {
      throw std::runtime_error("conditioned lock graph lost Vital Dance");
    }
  }
}

void controls_reject_overbroad_routes() {
  sim::State unique = exact_hand();
  unique.hand.erase(std::find(unique.hand.begin(), unique.hand.end(),
                              sim::Card::MegaDragonite));
  if (choose(sim::LockMode::FullRuleBoxAbility, std::move(unique), 67410) !=
      sim::Card::Oricorio) {
    throw std::runtime_error("unique Dialga payload was exposed");
  }

  sim::State unmatched = exact_hand();
  *std::find(unmatched.hand.begin(), unmatched.hand.end(),
             sim::Card::TeamYellsCheer) = sim::Card::Arven;
  if (choose(sim::LockMode::FullItem, std::move(unmatched), 67411) !=
      sim::Card::Oricorio) {
    throw std::runtime_error("unmatched full-Item graph was admitted");
  }

  sim::State unpayable;
  unpayable.hand = {sim::Card::QuickBall, sim::Card::MegaDragonite,
                    sim::Card::DialgaGX, sim::Card::Oricorio,
                    sim::Card::RegidragoVstar, sim::Card::Crispin,
                    sim::Card::ForestSealStone};
  // Quick Ball requires another card as its discard cost. Protected singleton
  // cards do not make a live strict-DCI route:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/788
  if (choose(sim::LockMode::FullRuleBoxAbility, std::move(unpayable), 67412) !=
      sim::Card::Oricorio) {
    throw std::runtime_error("unpayable Quick Ball was treated as live");
  }

  sim::State ultra;
  ultra.hand = {sim::Card::UltraBall, sim::Card::TeamYellsCheer,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::RegidragoVstar,
                sim::Card::ForestSealStone};
  // Ultra Ball requires two other cards, so identity alone cannot create this
  // one-discard opening exception:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  if (choose(sim::LockMode::FullRuleBoxAbility, std::move(ultra), 67413) !=
      sim::Card::Oricorio) {
    throw std::runtime_error("Ultra Ball identity over-expanded the route");
  }
}

}  // namespace

int main() {
  exact_graph_preserves_vital_dance();
  controls_reject_overbroad_routes();
  return 0;
}
