#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool duplicate_gladion_route(const Engine& engine) {
    return engine.wonder_tag_duplicate_held_gladion_has_no_marginal_route();
  }
};
}  // namespace sim

namespace {

sim::State issue_1929_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Gladion};
  state.deck = {sim::Card::Gladion, sim::Card::Grass, sim::Card::Fire};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::Arven};
  return state;
}

void test_k1_provenance_equivalence() {
  const auto route_is_live = [](const bool deck_seen,
                                const bool prizes_revealed,
                                const std::uint64_t seed) {
    const sim::Scenario scenario{
        "issue-1929", sim::DciProfile::NoDiscardControl,
        sim::LockMode::None, true, 5};
    const sim::DeckRecipe recipe = sim::baseline_recipe();
    std::mt19937_64 rng{seed};
    sim::Engine engine{scenario, recipe, rng};
    sim::EngineTestAccess::set_state(
        engine, issue_1929_state(), deck_seen, prizes_revealed);
    return sim::EngineTestAccess::duplicate_gladion_route(engine);
  };

  // Either legal inspection supplies the same K1 knowledge for the duplicate
  // Wonder Tag to Gladion hold. K0 cannot know the prized VSTAR axis:
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Correction precedent: https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Ability, search, Prize, and one-Supporter-per-turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1929
  if (!route_is_live(true, false, 192901) ||
      !route_is_live(false, true, 192902) ||
      route_is_live(false, false, 192903)) {
    throw std::runtime_error("Issue 1929 K1 provenance boundary failed");
  }
}

}  // namespace

int main() {
  test_k1_provenance_equivalence();
  const auto scenario = sim::scenario_by_label("no-discard-control/go-first");
  if (!scenario) throw std::runtime_error("Missing no-discard-control/go-first scenario");
  sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{172};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const bool held_on_t3 = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T3 | HOLD TAPU LELE-GX") != std::string::npos &&
               line.find("held Gladion") != std::string::npos;
      });
  const bool duplicate_t3_gladion = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T3 | WONDER TAG") != std::string::npos &&
               line.find("Gladion") != std::string::npos;
      });
  const bool used_held_gladion = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("T3 | PLAY SUPPORTER") != std::string::npos &&
               line.find("R-GLADION-01") != std::string::npos &&
               line.find("Earthen Vessel") != std::string::npos;
      });

  // Wonder Tag would search another Gladion for the same known Prize axis. The held
  // copy already performs that mandatory exchange, and only one Supporter can be
  // played this turn. Holding Tapu preserves the Bench slot without changing T3:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1039
  if (outcome.first_ready_turn != 3 || !held_on_t3 ||
      duplicate_t3_gladion || !used_held_gladion) {
    throw std::runtime_error("Seed 172 still spent Wonder Tag for duplicate held Gladion");
  }
  return 0;
}
