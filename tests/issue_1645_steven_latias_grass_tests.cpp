#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

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
  static bool route(const Engine& engine) {
    return engine.issue_1645_steven_latias_grass_route_available();
  }
};

}  // namespace sim

namespace {

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

sim::State issue_1927_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0}};
  state.hand = {sim::Card::StevensResolve, sim::Card::RegidragoVstar,
                sim::Card::EarthenVessel, sim::Card::BrilliantBlender,
                sim::Card::Dragapult};
  state.deck = {sim::Card::LatiasEx, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Fire, sim::Card::MegaDragonite};
  state.prizes = {sim::Card::Crispin, sim::Card::Crispin};
  return state;
}

void verify_k1_provenance_equivalence() {
  const auto route_is_live = [](const bool deck_seen,
                                const bool prizes_revealed,
                                const std::uint64_t seed) {
    const sim::Scenario scenario{
        "issue-1927", sim::DciProfile::MatchupFlexJit,
        sim::LockMode::None, false, 5};
    const sim::DeckRecipe recipe = sim::baseline_recipe();
    std::mt19937_64 rng{seed};
    sim::Engine engine{scenario, recipe, rng};
    sim::EngineTestAccess::set_state(
        engine, issue_1927_state(), deck_seen, prizes_revealed);
    return sim::EngineTestAccess::route(engine);
  };

  // Either legal inspection supplies the same K1 knowledge for the Steven,
  // Latias ex, and Grass target package. K0 remains rejected:
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Correction precedent: https://github.com/FlareZ123/pokemon-sims/commit/690808e65feb4c17034cd3d76157ff5929a65754
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Official Supporter, Item, search, attachment, evolution, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1927
  if (!route_is_live(true, false, 192701) ||
      !route_is_live(false, true, 192702) ||
      route_is_live(false, false, 192703)) {
    throw std::runtime_error("Issue 1927 K1 provenance boundary failed");
  }
}

}  // namespace

int main() {
  using namespace sim;

  verify_k1_provenance_equivalence();

  const auto scenario = scenario_by_label("matchup-flex-jit/go-second");
  const CrobatModelingDeck* deck =
      crobat_modeling_deck_by_id("crobat1-heavy-ball");
  if (!scenario || deck == nullptr) {
    throw std::runtime_error("Issue 1645 registered setup is unavailable.");
  }

  std::mt19937_64 rng(218);
  TraceLog trace{true, {}};
  Engine engine(*scenario, deck->recipe, rng, &trace);
  const TrialOutcome outcome = engine.run();

  // Earthen Vessel resolves and attaches before Steven ends turn one. Steven then
  // searches Latias ex and one Grass, preserving held VSTAR and Blender. T2 evolves
  // and retreats the prepared Regidrago, while T3 Fire plus Blender reaches readiness:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core Item, attachment, evolution, Supporter, retreat, and turn procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Original refined route: https://github.com/FlareZ123/pokemon-sims/issues/1645
  // Confirmed pre-Steven ordering bug: https://github.com/FlareZ123/pokemon-sims/issues/1700
  if (outcome.first_ready_turn != 3 ||
      !trace_contains(trace,
                      "Earthen Vessel searched Grass and Fire before Steven's Resolve") ||
      !trace_contains(trace,
                      "Searched the complete post-Vessel T3 route: Latias ex, Grass Energy") ||
      !trace_contains(trace, "T2 | EVOLVE") ||
      !trace_contains(trace, "T2 | RETREAT") ||
      !trace_contains(trace, "T3 | READY")) {
    throw std::runtime_error(
        "Seed 218 did not complete the corrected pre-Steven Vessel route on T3.");
  }

  if (trace_contains(
          trace,
          "Searched up to 3 cards: Regidrago V, Regidrago VSTAR, Gladion") ||
      trace_contains(trace,
                     "Searched the complete Latias-Grass T4 route")) {
    throw std::runtime_error(
        "Seed 218 still selected a slower Steven target package.");
  }
}
