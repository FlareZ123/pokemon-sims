#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void choose_opening_active(Engine& engine) { engine.choose_opening_active(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void require_dialga_active(sim::Engine& engine, const char* message) {
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::DialgaGX ||
      !contains(after.hand, sim::Card::Oricorio)) {
    throw std::runtime_error(message);
  }
}

void require_oricorio_active(sim::Engine& engine, const char* message) {
  sim::EngineTestAccess::choose_opening_active(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::Oricorio ||
      !contains(after.hand, sim::Card::DialgaGX)) {
    throw std::runtime_error(message);
  }
}

void test_exact_issue_663_hand_preserves_vital_dance() {
  const sim::Scenario scenario{"opening-oricorio-dialga-issue-663", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{663};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::QuickBall, sim::Card::TeamYellsCheer,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::Powerglass,
                sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Either Basic may start. Mega Dragonite ex preserves the Dragon-payload axis,
  // Quick Ball exposes Regidrago V, and retaining Oricorio keeps Vital Dance live:
  // https://tcg.pokemon.com/assets/img/learn-to-play/getting-started/quick-start-rules/en-us/quick_start_rulebook.pdf#Set_Up_to_Play
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://github.com/FlareZ123/pokemon-sims/issues/663
  require_dialga_active(engine, "The approved #663 hand should preserve Oricorio for Vital Dance.");
}

void test_steven_arven_public_graph_preserves_vital_dance() {
  const sim::Scenario scenario{"opening-oricorio-dialga-supporter-graph", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66301};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::Oricorio, sim::Card::RegidragoVstar,
                sim::Card::StevensResolve, sim::Card::Lusamine,
                sim::Card::Arven, sim::Card::HisuianGoodraVstar,
                sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Steven's Resolve can search any three cards and Arven can expose a search Item.
  // Hisuian Goodra VSTAR supplies the additional held Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://github.com/FlareZ123/pokemon-sims/issues/663
  require_dialga_active(engine, "A public Steven/Arven Regidrago graph should not waste Vital Dance.");
}

void test_unique_dialga_payload_keeps_oricorio_active() {
  const sim::Scenario scenario{"opening-oricorio-dialga-unique-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66302};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::QuickBall, sim::Card::DialgaGX, sim::Card::Oricorio,
                sim::Card::TeamYellsCheer, sim::Card::Powerglass,
                sim::Card::RegidragoVstar, sim::Card::Lusamine};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Dialga-GX is the only held Dragon attack payload, so its strict-JIT DCI value
  // remains higher than the Energy-compression route:
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // https://github.com/FlareZ123/pokemon-sims/issues/663
  require_oricorio_active(engine, "A unique Dialga-GX payload must remain in hand.");
}

void test_complete_energy_hand_keeps_oricorio_active() {
  const sim::Scenario scenario{"opening-oricorio-dialga-energy-complete", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66303};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::DialgaGX, sim::Card::Oricorio,
                sim::Card::Grass, sim::Card::Fire, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Both GGF Energy types are already public in hand, so Vital Dance no longer
  // supplies the missing setup axis that justifies consuming Dialga-GX as Active:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/663
  require_oricorio_active(engine, "A complete Energy hand should preserve Dialga-GX.");
}

void test_no_public_regidrago_connector_keeps_oricorio_active() {
  const sim::Scenario scenario{"opening-oricorio-dialga-no-connector", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66304};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::TeamYellsCheer,
                sim::Card::Powerglass, sim::Card::RegidragoVstar,
                sim::Card::Lusamine};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  require_oricorio_active(engine, "Without a public Regidrago connector, preserve the Dialga route.");
}

void test_lock_scope_remains_owned_by_issue_674() {
  const sim::Scenario scenario{"opening-oricorio-dialga-lock-scope", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, true, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{66305};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.hand = {sim::Card::QuickBall, sim::Card::TeamYellsCheer,
                sim::Card::MegaDragonite, sim::Card::DialgaGX,
                sim::Card::Oricorio, sim::Card::Powerglass,
                sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // This regression keeps #663 restricted to its twice-approved no-lock scope.
  // The separately approved lock distributions and their controls belong to #674:
  // https://github.com/FlareZ123/pokemon-sims/issues/663
  // https://github.com/FlareZ123/pokemon-sims/issues/674
  require_oricorio_active(engine, "Issue #663 must not silently absorb the #674 lock scope.");
}

}  // namespace

int main() {
  test_exact_issue_663_hand_preserves_vital_dance();
  test_steven_arven_public_graph_preserves_vital_dance();
  test_unique_dialga_payload_keeps_oricorio_active();
  test_complete_energy_hand_keeps_oricorio_active();
  test_no_public_regidrago_connector_keeps_oricorio_active();
  test_lock_scope_remains_owned_by_issue_674();
  return 0;
}
