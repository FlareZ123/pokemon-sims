#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static const std::vector<Card>& resolving_sources(const Engine& engine) {
    return engine.resolving_trainer_sources_;
  }
  static bool begin_supporter_resolution(Engine& engine, const Card card) {
    return engine.begin_supporter_resolution(card);
  }
  static bool finish_supporter_resolution(Engine& engine, const Card card) {
    return engine.finish_supporter_resolution(card);
  }
  static bool play_tate_draw(Engine& engine) {
    return engine.play_tate_draw();
  }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void require(const bool condition, const std::string_view message) {
  if (!condition) throw std::runtime_error(std::string(message));
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  Fixture()
      : scenario{"issue-4348/exact", sim::DciProfile::StrictJit,
                 sim::LockMode::None, false, 5},
        recipe(sim::pineco_recipe()),
        rng(4348),
        trace{true, {}},
        engine(scenario, recipe, rng, &trace) {}
};

void shared_supporter_source_lifecycle_matches_b03() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.hand = {sim::Card::Crispin, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);

  // B-03 step 2 plays the Supporter from hand, step 3 resolves its text, and step
  // 4 discards it only after use. The shared resolving zone represents that source.
  // Supporter procedure B-03: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Confirmed lifecycle defect: https://github.com/FlareZ123/pokemon-sims/issues/4348
  require(sim::EngineTestAccess::begin_supporter_resolution(
              fixture.engine, sim::Card::Crispin),
          "Supporter did not enter the resolving-source zone.");
  const sim::State& resolving = sim::EngineTestAccess::state(fixture.engine);
  require(!contains(resolving.hand, sim::Card::Crispin),
          "Resolving Supporter remained in ordinary hand.");
  require(!contains(resolving.discard, sim::Card::Crispin),
          "Resolving Supporter became discard-visible before its effect finished.");
  require(resolving.supporter_used,
          "Playing the Supporter did not consume the turn's Supporter action.");
  require(contains(sim::EngineTestAccess::resolving_sources(fixture.engine),
                   sim::Card::Crispin),
          "Supporter source was not retained while resolving.");

  require(sim::EngineTestAccess::finish_supporter_resolution(
              fixture.engine, sim::Card::Crispin),
          "Resolved Supporter did not move to discard.");
  const sim::State& finished = sim::EngineTestAccess::state(fixture.engine);
  require(contains(finished.discard, sim::Card::Crispin),
          "Resolved Supporter source did not end in discard.");
  require(sim::EngineTestAccess::resolving_sources(fixture.engine).empty(),
          "Resolved Supporter leaked from the resolving-source zone.");
}

void tate_draw_excludes_its_source_from_the_shuffled_hand() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.hand = {sim::Card::TateLiza, sim::Card::Grass};
  state.deck = {sim::Card::Fire, sim::Card::RegidragoV,
                sim::Card::RegidragoVstar, sim::Card::Dipplin,
                sim::Card::Crispin, sim::Card::QuickBall};
  sim::EngineTestAccess::set_state(fixture.engine, state);

  // Tate & Liza's draw mode shuffles the remaining hand and draws 5. Its own
  // Supporter source is already being played at B-03 step 2, so it cannot be one
  // of the cards shuffled into the deck by its step-3 effect.
  // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
  // Supporter procedure B-03: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed lifecycle defect: https://github.com/FlareZ123/pokemon-sims/issues/4348
  require(sim::EngineTestAccess::play_tate_draw(fixture.engine),
          "Legal Tate & Liza draw mode did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(contains(after.discard, sim::Card::TateLiza),
          "Tate & Liza did not reach discard after its effect.");
  require(!contains(after.deck, sim::Card::TateLiza),
          "Tate & Liza was incorrectly shuffled into the deck by its own effect.");
  require(sim::EngineTestAccess::resolving_sources(fixture.engine).empty(),
          "Tate & Liza remained in the resolving-source zone.");
}

}  // namespace

int main() {
  shared_supporter_source_lifecycle_matches_b03();
  tate_draw_excludes_its_source_from_the_shuffled_hand();
  return 0;
}
