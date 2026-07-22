#include "issue_962_quick_ball_vessel_common.inc"

void test_vessel_stays_first_when_tapu_is_unavailable() {
  const sim::Scenario scenario{"issue-962-no-tapu", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{963};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = base_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::TapuLeleGX),
                   state.deck.end());
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Quick Ball cannot prefer a connector that the inspected deck does not contain:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#knowledge-states
  // https://github.com/FlareZ123/pokemon-sims/issues/962
  if (!sim::EngineTestAccess::run_search_step(engine, true)) {
    throw std::runtime_error("A missing Tapu Lele-GX must preserve ordinary Vessel priority.");
  }
  require_vessel_was_first(sim::EngineTestAccess::state(engine));
}

void test_vessel_stays_first_when_crispin_is_unavailable() {
  const sim::Scenario scenario{"issue-962-no-crispin", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{964};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = base_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), sim::Card::Crispin),
                   state.deck.end());
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Wonder Tag cannot justify the connector when the inspected deck contains no
  // Crispin target: https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/962
  if (!sim::EngineTestAccess::run_search_step(engine, true)) {
    throw std::runtime_error("A missing Crispin must preserve ordinary Vessel priority.");
  }
  require_vessel_was_first(sim::EngineTestAccess::state(engine));
}

void test_vessel_stays_first_when_bench_is_full() {
  const sim::Scenario scenario{"issue-962-full-bench", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{964};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = base_state();
  for (int index = 0; index < 5; ++index) {
    state.bench.push_back(sim::Pokemon{sim::Card::MawileGX, 1, 0, 0,
                                      sim::Tool::None});
  }
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Tapu Lele-GX must be played to an open Bench before Wonder Tag can resolve:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/962
  if (!sim::EngineTestAccess::run_search_step(engine, true)) {
    throw std::runtime_error("A full Bench must preserve ordinary Vessel priority.");
  }
  require_vessel_was_first(sim::EngineTestAccess::state(engine));
}

}  // namespace

int main() {
  try {
    test_vessel_stays_first_when_tapu_is_unavailable();
    test_vessel_stays_first_when_crispin_is_unavailable();
    test_vessel_stays_first_when_bench_is_full();
    std::cout << "Issue 962 target controls passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
