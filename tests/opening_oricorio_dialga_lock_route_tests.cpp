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
      : scenario{"issue-3433-lock-opening", sim::DciProfile::StrictJit,
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

sim::State replace_card(sim::State state, const sim::Card old_card,
                        const sim::Card new_card) {
  const auto it = std::find(state.hand.begin(), state.hand.end(), old_card);
  if (it == state.hand.end()) {
    throw std::runtime_error("replacement source card is absent");
  }
  *it = new_card;
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

void require_dialga_with_oricorio_held(const sim::LockMode lock,
                                       sim::State state,
                                       const std::uint64_t seed,
                                       const char* message) {
  Fixture fixture{lock, seed};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(fixture.engine);
  const auto& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      std::find(after.hand.begin(), after.hand.end(), sim::Card::Oricorio) ==
          after.hand.end()) {
    throw std::runtime_error(message);
  }
}

void route_invariants_preserve_vital_dance() {
  const std::array locks{sim::LockMode::TurnTwoItem,
                         sim::LockMode::FullRuleBoxAbility,
                         sim::LockMode::FullItem,
                         sim::LockMode::FullCombined};
  for (std::size_t index = 0; index < locks.size(); ++index) {
    sim::State without_cheer =
        replace_card(exact_hand(), sim::Card::TeamYellsCheer,
                     sim::Card::RoseannesBackup);
    sim::State without_powerglass =
        replace_card(exact_hand(), sim::Card::Powerglass,
                     sim::Card::WishfulBaton);
    sim::State without_both =
        replace_card(without_cheer, sim::Card::Powerglass,
                     sim::Card::WishfulBaton);
    sim::State treasure_connector =
        replace_card(exact_hand(), sim::Card::QuickBall,
                     sim::Card::MysteriousTreasure);

    // Setup can start the redundant Dialga-GX and keep Oricorio in hand for Vital
    // Dance. Team Yell's Cheer and Powerglass are not prerequisites for setup,
    // Vital Dance, or the held one-discard Regidrago V connector. Quick Ball and
    // Mysterious Treasure share the relevant one-discard Basic-search route:
    // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
    // https://api.pokemontcg.io/v2/cards/sm2-55
    // https://api.pokemontcg.io/v2/cards/sm5-100
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://api.pokemontcg.io/v2/cards/swsh12-135
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#lock-interpretation
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/674
    // https://github.com/FlareZ123/pokemon-sims/issues/3433
    require_dialga_with_oricorio_held(
        locks[index], exact_hand(), 343300U + index,
        "historical lock route lost Vital Dance");
    require_dialga_with_oricorio_held(
        locks[index], std::move(without_cheer), 343310U + index,
        "Team Yell's Cheer identity changed the opening choice");
    require_dialga_with_oricorio_held(
        locks[index], std::move(without_powerglass), 343320U + index,
        "Powerglass identity changed the opening choice");
    require_dialga_with_oricorio_held(
        locks[index], std::move(without_both), 343330U + index,
        "incidental fixture identities changed the opening choice");
    require_dialga_with_oricorio_held(
        locks[index], std::move(treasure_connector), 343340U + index,
        "equivalent one-discard connector changed the opening choice");
  }
}

void controls_reject_overbroad_routes() {
  sim::State unique = exact_hand();
  unique.hand.erase(std::find(unique.hand.begin(), unique.hand.end(),
                              sim::Card::MegaDragonite));
  if (choose(sim::LockMode::FullRuleBoxAbility, std::move(unique), 343350) !=
      sim::Card::Oricorio) {
    throw std::runtime_error("unique Dialga payload was exposed");
  }

  sim::State unpayable;
  unpayable.hand = {sim::Card::QuickBall, sim::Card::MegaDragonite,
                    sim::Card::DialgaGX, sim::Card::Oricorio,
                    sim::Card::RegidragoVstar, sim::Card::Crispin,
                    sim::Card::ForestSealStone};
  // Immediate Quick Ball access still requires a legal discard. A held Crispin
  // supplies the competing Supporter/Energy axis, so the deferred connector does
  // not override the strict-DCI route-preservation decision:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/788
  // https://github.com/FlareZ123/pokemon-sims/issues/3433
  if (choose(sim::LockMode::FullRuleBoxAbility, std::move(unpayable), 343351) !=
      sim::Card::Oricorio) {
    throw std::runtime_error("protected Quick Ball route overrode held Crispin");
  }

  sim::State ultra;
  ultra.hand = {sim::Card::UltraBall, sim::Card::TeamYellsCheer,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::RegidragoVstar,
                sim::Card::ForestSealStone};
  // Ultra Ball has a two-card discard cost and is outside the one-discard opening
  // connector class. Its identity alone cannot admit this route:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/3433
  if (choose(sim::LockMode::FullRuleBoxAbility, std::move(ultra), 343352) !=
      sim::Card::Oricorio) {
    throw std::runtime_error("Ultra Ball identity over-expanded the route");
  }
}

}  // namespace

int main() {
  route_invariants_preserve_vital_dance();
  controls_reject_overbroad_routes();
  return 0;
}
