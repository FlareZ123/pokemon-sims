#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <stdexcept>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static Card choose_supporter(Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
};

}  // namespace sim

namespace {

sim::State exact_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::Crispin, sim::Card::Grass, sim::Card::Fire,
                sim::Card::Gladion};
  state.prizes = {sim::Card::Grass, sim::Card::Oricorio};
  state.discard = {sim::Card::QuickBall, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::QuickBall,
                               sim::Card::MegaDragonite};
  return state;
}

sim::Card selected_supporter(sim::State state,
                             const sim::DciProfile dci =
                                 sim::DciProfile::StrictJit) {
  const sim::Scenario scenario{"issue-1870", dci, sim::LockMode::None,
                               true, 5};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{1870};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::choose_supporter(engine);
}

void test_crispin_preempts_final_prized_energy_gladion() {
  // Quick Ball has already discarded Mega Dragonite ex this turn. Wonder Tag
  // must choose Crispin to complete GGF now instead of Gladion for a Prize card:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://github.com/FlareZ123/pokemon-sims/issues/1870
  if (selected_supporter(exact_state()) != sim::Card::Crispin) {
    throw std::runtime_error(
        "Crispin must preempt Gladion for the current-turn GGF finish.");
  }
}

void test_absent_crispin_preserves_gladion_fallback() {
  sim::State state = exact_state();
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::Gladion};
  if (selected_supporter(std::move(state)) != sim::Card::Gladion) {
    throw std::runtime_error(
        "Gladion must remain the fallback when Crispin is absent.");
  }
}

void test_non_strict_profile_preserves_gladion_priority() {
  if (selected_supporter(exact_state(), sim::DciProfile::NoDiscardControl) !=
      sim::Card::Gladion) {
    throw std::runtime_error(
        "The issue-1870 override must remain strict-JIT specific.");
  }
}

void test_non_vstar_state_preserves_gladion_priority() {
  sim::State state = exact_state();
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::ForestSealStone};
  state.hand.push_back(sim::Card::RegidragoVstar);
  if (selected_supporter(std::move(state)) != sim::Card::Gladion) {
    throw std::runtime_error(
        "The narrow override must require an already Active VSTAR.");
  }
}

void test_spent_supporter_preserves_gladion_priority() {
  sim::State state = exact_state();
  state.supporter_used = true;
  // Only one Supporter may be played each turn, so Crispin cannot complete the
  // current-turn route once the Supporter action is spent:
  // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://github.com/FlareZ123/pokemon-sims/issues/1870
  if (selected_supporter(std::move(state)) != sim::Card::Gladion) {
    throw std::runtime_error(
        "The override must require an unused Supporter action.");
  }
}

}  // namespace

int main() {
  // This regression is validated together with the merged final-Energy Vessel
  // and Professor Burnet routes from issue 1866:
  // https://github.com/FlareZ123/pokemon-sims/issues/1866
  // https://github.com/FlareZ123/pokemon-sims/pull/1881
  // Both fixed-seed 100,000-trial matrices are regenerated from the combined
  // source before the complete Release and sanitizer suites run:
  // https://github.com/FlareZ123/pokemon-sims/blob/main/results/simulation_results.csv
  // https://github.com/FlareZ123/pokemon-sims/blob/main/results/multi_deck_comparison.csv
  test_crispin_preempts_final_prized_energy_gladion();
  test_absent_crispin_preserves_gladion_fallback();
  test_non_strict_profile_preserves_gladion_priority();
  test_non_vstar_state_preserves_gladion_priority();
  test_spent_supporter_preserves_gladion_priority();
  return 0;
}
