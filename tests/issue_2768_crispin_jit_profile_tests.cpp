#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static bool route_available(const Engine& engine) {
    return engine.issue_1393_held_crispin_completion_available();
  }
  static void choose_supporter(Engine& engine) {
    engine.choose_supporter_issue_1152();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::Scenario scenario(const sim::DciProfile profile) {
  return sim::Scenario{"issue-2768-crispin-jit-profile", profile,
                       sim::LockMode::None, false, 4};
}

sim::State exact_blender_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0,
                              sim::Tool::ForestSealStone};
  state.bench = {sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None},
                 sim::Pokemon{sim::Card::RegidragoV, 2, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::QuickBall, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::Gladion,
                sim::Card::MegaDragonite, sim::Card::BrilliantBlender};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::Oricorio,
                sim::Card::Dragapult, sim::Card::DialgaGX};
  state.prizes = {sim::Card::GoodraVstar};
  return state;
}

void both_jit_profiles_prefer_held_crispin() {
  // Crispin plus the unused manual attachment completes GGF, while the held
  // Brilliant Blender gives a deterministic current-turn Dragon payload outlet.
  // Gladion consumes the same Supporter slot and does not advance the ready state.
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Supporter, attachment, evolution, Item, Ability, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, earliest route, and shared same-turn JIT timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Original selector route: https://github.com/FlareZ123/pokemon-sims/issues/1393
  // Confirmed cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2768
  for (const sim::DciProfile profile : {sim::DciProfile::StrictJit,
                                        sim::DciProfile::MatchupFlexJit}) {
    std::mt19937_64 rng{2768};
    const sim::DeckRecipe recipe = sim::baseline_recipe();
    sim::Engine engine{scenario(profile), recipe, rng};
    sim::EngineTestAccess::set_state(engine, exact_blender_state());

    expect(sim::EngineTestAccess::route_available(engine),
           "A same-turn JIT profile rejected the held-Crispin completion route");
    sim::EngineTestAccess::choose_supporter(engine);
    expect(contains(engine.state().discard, sim::Card::Crispin),
           "The selector did not play Crispin on the complete route");
    expect(contains(engine.state().hand, sim::Card::Gladion),
           "The selector spent Gladion despite the faster held-Crispin route");
  }
}

void no_discard_control_does_not_enter_the_jit_override() {
  // NoDiscardControl uses different earlier-payload semantics and remains outside
  // this same-turn-JIT selector override.
  // DCI/JIT profile contract: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed cross-profile regression: https://github.com/FlareZ123/pokemon-sims/issues/2768
  std::mt19937_64 rng{2768};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine{scenario(sim::DciProfile::NoDiscardControl), recipe, rng};
  sim::EngineTestAccess::set_state(engine, exact_blender_state());
  expect(!sim::EngineTestAccess::route_available(engine),
         "NoDiscardControl entered the same-turn-JIT Crispin override");
}
}  // namespace

int main() {
  try {
    both_jit_profiles_prefer_held_crispin();
    no_discard_control_does_not_enter_the_jit_override();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
