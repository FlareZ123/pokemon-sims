#include "issue_962_quick_ball_vessel_common.inc"

void test_quick_ball_connector_preserves_vessel_for_next_turn() {
  const sim::Scenario scenario{"issue-962-positive", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{962};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, base_state());

  // Quick Ball can discard the first Dragon, search Tapu Lele-GX, and enable
  // Wonder Tag for Crispin. Once that public K1 route is established, Crispin plus
  // the manual attachment creates GF on T2. Preserving Earthen Vessel and the
  // second Dragon leaves a legal T3 strict-JIT discard and final Grass search:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Resource priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Original connector bug: https://github.com/FlareZ123/pokemon-sims/issues/962
  // Confirmed timing refinement: https://github.com/FlareZ123/pokemon-sims/issues/1447
  if (!sim::EngineTestAccess::run_search_step(engine, true)) {
    throw std::runtime_error("The first Energy-search step should play Quick Ball.");
  }
  const sim::State& after_quick_ball = sim::EngineTestAccess::state(engine);
  if (!contains(after_quick_ball.discard, sim::Card::QuickBall) ||
      !contains(after_quick_ball.discard, sim::Card::GoodraVstar) ||
      !contains(after_quick_ball.hand, sim::Card::TapuLeleGX) ||
      !contains(after_quick_ball.hand, sim::Card::EarthenVessel)) {
    throw std::runtime_error("Quick Ball must pay Goodra, fetch Tapu Lele-GX, and preserve Vessel.");
  }

  if (sim::EngineTestAccess::run_search_step(engine, true)) {
    throw std::runtime_error("The second search step should hold all Items for the proven Crispin route.");
  }
  const sim::State& after_hold = sim::EngineTestAccess::state(engine);
  if (!contains(after_hold.hand, sim::Card::Crispin) ||
      !contains(after_hold.hand, sim::Card::EarthenVessel) ||
      !contains(after_hold.hand, sim::Card::DialgaGX) ||
      contains(after_hold.discard, sim::Card::EarthenVessel) ||
      contains(after_hold.discard, sim::Card::DialgaGX)) {
    throw std::runtime_error("Wonder Tag must find Crispin while Vessel and its T3 Dragon cost remain held.");
  }

  if (!sim::EngineTestAccess::play_crispin(engine) ||
      !sim::EngineTestAccess::attach_manual(engine)) {
    throw std::runtime_error("Crispin plus the manual attachment must establish the projected T2 GF line.");
  }
  const sim::Pokemon& active = *sim::EngineTestAccess::state(engine).active;
  if (active.grass != 1 || active.fire != 1) {
    throw std::runtime_error("The refined selector route must leave the Active Regidrago VSTAR at GF.");
  }
}

}  // namespace

int main() {
  try {
    test_quick_ball_connector_preserves_vessel_for_next_turn();
    std::cout << "Issue 962 refined connector test passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
