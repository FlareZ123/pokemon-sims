#include "issue_962_quick_ball_vessel_common.inc"

void test_quick_ball_connector_precedes_vessel_when_it_compresses_energy() {
  const sim::Scenario scenario{"issue-962-positive", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{962};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, base_state());

  // Quick Ball can discard the first Dragon, search Tapu Lele-GX, and enable
  // Wonder Tag for Crispin. A separately payable Earthen Vessel then searches the
  // Basic Energy that Crispin plus the manual attachment compress into GG on T2:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/962
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

  if (!sim::EngineTestAccess::run_search_step(engine, true)) {
    throw std::runtime_error("The second Energy-search step should Bench Tapu and play Vessel.");
  }
  const sim::State& after_vessel = sim::EngineTestAccess::state(engine);
  if (!contains(after_vessel.discard, sim::Card::EarthenVessel) ||
      !contains(after_vessel.discard, sim::Card::DialgaGX) ||
      !contains(after_vessel.hand, sim::Card::Crispin) ||
      !contains(after_vessel.hand, sim::Card::Grass) ||
      !contains(after_vessel.hand, sim::Card::Fire)) {
    throw std::runtime_error("Wonder Tag must find Crispin before Vessel pays the second Dragon cost.");
  }

  if (!sim::EngineTestAccess::play_crispin(engine) ||
      !sim::EngineTestAccess::attach_manual(engine)) {
    throw std::runtime_error("Crispin plus the manual attachment must complete the projected T2 GG line.");
  }
  const sim::Pokemon& active = *sim::EngineTestAccess::state(engine).active;
  if (active.grass != 2 || active.fire != 0) {
    throw std::runtime_error("The positive selector route must leave the Active Regidrago VSTAR at GG.");
  }
}

}  // namespace

int main() {
  try {
    test_quick_ball_connector_precedes_vessel_when_it_compresses_energy();
    std::cout << "Issue 962 positive connector test passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
