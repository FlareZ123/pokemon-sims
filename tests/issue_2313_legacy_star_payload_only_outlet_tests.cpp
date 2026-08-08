#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

// Current-main reclaim validation: https://github.com/FlareZ123/pokemon-sims/issues/2313
namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }

  static bool use_legacy_star(Engine& engine) {
    return engine.use_legacy_star();
  }

  static const State& state(const Engine& engine) {
    return engine.state_;
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::State combined_axis_state(const sim::Card suppressor) {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {suppressor, sim::Card::Dragapult, sim::Card::DialgaGX};
  state.discard = {sim::Card::EarthenVessel, sim::Card::MysteriousTreasure};
  state.deck = {
      sim::Card::Fire, sim::Card::Grass, sim::Card::Grass,
      sim::Card::MegaDragonite, sim::Card::GoodraVstar,
      sim::Card::RegidragoV, sim::Card::RegidragoVstar,
      sim::Card::QuickBall, sim::Card::Arven};
  return state;
}

bool run_case(const sim::DciProfile dci, sim::State state, const std::uint64_t seed) {
  const sim::Scenario scenario{"issue-2313", dci, sim::LockMode::None, false, 5};
  std::mt19937_64 rng(seed);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, sim::baseline_recipe(), rng, &trace);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::use_legacy_star(engine);
}

void test_burnet_does_not_suppress_combined_energy_route() {
  // Professor Burnet supplies only the current-turn payload axis. Legacy Star may
  // recover any two cards, and the already discarded Earthen Vessel can bridge the
  // missing Basic Fire Energy with a held Dragon as its printed discard cost:
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Official Supporter, Ability, Item, discard, search, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 / earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed seed-4 regression: https://github.com/FlareZ123/pokemon-sims/issues/2313
  expect(run_case(sim::DciProfile::StrictJit,
                  combined_axis_state(sim::Card::ProfessorBurnet), 231301),
         "Held Professor Burnet still suppressed the combined Legacy Star Energy route.");
}

void test_treasure_does_not_suppress_combined_energy_route() {
  // Mysterious Treasure can discard the held Dragon for the payload axis, but its
  // printed search effect does not supply Basic Fire Energy. Legacy Star therefore
  // remains admissible while Fire is the separate unresolved axis:
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago VSTAR / Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Official Item, Ability, discard, search, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed seed-4 regression: https://github.com/FlareZ123/pokemon-sims/issues/2313
  expect(run_case(sim::DciProfile::MatchupFlexJit,
                  combined_axis_state(sim::Card::MysteriousTreasure), 231302),
         "Held Mysterious Treasure still suppressed the combined Legacy Star Energy route.");
}

void test_spent_attachment_preserves_legacy_star_gate() {
  sim::State state = combined_axis_state(sim::Card::ProfessorBurnet);
  state.manual_energy_used = true;
  expect(!run_case(sim::DciProfile::StrictJit, std::move(state), 231303),
         "Legacy Star bypassed the combined-axis guard after the manual attachment was spent.");
}

void test_held_fire_preserves_direct_route() {
  sim::State state = combined_axis_state(sim::Card::ProfessorBurnet);
  state.hand.push_back(sim::Card::Fire);
  expect(!run_case(sim::DciProfile::StrictJit, std::move(state), 231304),
         "Legacy Star spent the VSTAR Power despite a held Fire plus Burnet direct route.");
}
}  // namespace

int main() {
  try {
    test_burnet_does_not_suppress_combined_energy_route();
    test_treasure_does_not_suppress_combined_energy_route();
    test_spent_attachment_preserves_legacy_star_gate();
    test_held_fire_preserves_direct_route();
    std::cout << "Issue 2313 Legacy Star payload-only outlet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
