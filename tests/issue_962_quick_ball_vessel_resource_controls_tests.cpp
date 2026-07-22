#include "issue_962_quick_ball_vessel_common.inc"

void test_vessel_stays_first_when_manual_attachment_already_completes_energy() {
  const sim::Scenario scenario{"issue-962-one-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{965};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = base_state();
  state.active->grass = 2;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Earthen Vessel can search the single missing Fire Energy for the unused manual
  // attachment, so spending Quick Ball, Tapu Lele-GX, and the Supporter slot adds
  // no setup speed: https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/962
  if (!sim::EngineTestAccess::run_search_step(engine, true)) {
    throw std::runtime_error("A one-Energy manual route must preserve ordinary Vessel priority.");
  }
  require_vessel_was_first(sim::EngineTestAccess::state(engine));
}

void test_vessel_stays_first_when_second_discard_is_unpayable() {
  const sim::Scenario scenario{"issue-962-cost-contention", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{965};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = base_state();
  state.hand = {sim::Card::QuickBall, sim::Card::EarthenVessel,
                sim::Card::GoodraVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Each Item needs a separate discarded card. Quick Ball-first is rejected when
  // paying its Dragon cost would leave no policy-legal Earthen Vessel cost:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/962
  if (!sim::EngineTestAccess::run_search_step(engine, true)) {
    throw std::runtime_error("Discard-cost contention must preserve ordinary Vessel priority.");
  }
  require_vessel_was_first(sim::EngineTestAccess::state(engine));
}

}  // namespace

int main() {
  try {
    test_vessel_stays_first_when_manual_attachment_already_completes_energy();
    test_vessel_stays_first_when_second_discard_is_unpayable();
    std::cout << "Issue 962 resource controls passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
