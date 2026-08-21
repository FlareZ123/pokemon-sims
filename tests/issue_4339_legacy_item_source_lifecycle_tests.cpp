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
  static void set_deck_seen(Engine& engine, const bool value) {
    engine.deck_seen_ = value;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static const std::vector<Card>& resolving_sources(const Engine& engine) {
    return engine.resolving_trainer_sources_;
  }
  static bool begin_legacy_item_resolution(Engine& engine, const Card card) {
    return engine.begin_legacy_item_resolution(card);
  }
  static bool finish_legacy_item_resolution(Engine& engine, const Card card) {
    return engine.finish_legacy_item_resolution(card);
  }
  static bool discard_from_hand(Engine& engine, const Card card) {
    return engine.discard_from_hand(card, "issue 4339 test cost", "R-GAME-ITEM");
  }
  static bool play_mysterious_treasure(Engine& engine) {
    return engine.play_mysterious_treasure(false);
  }
  static bool play_earthen_vessel(Engine& engine) {
    return engine.play_earthen_vessel(false);
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
      : scenario{"issue-4339/exact", sim::DciProfile::StrictJit,
                 sim::LockMode::None, false, 5},
        recipe(sim::pineco_recipe()),
        rng(4339),
        trace{true, {}},
        engine(scenario, recipe, rng, &trace) {}
};

void test_shared_lifecycle_keeps_played_item_out_of_hand_and_discard() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);

  // B-01 step 2 plays the Item from hand, step 3 resolves its printed text, and
  // step 4 discards the Item only after it has been used. The mandatory cost is a
  // separate card and is already in discard while the Item source is resolving.
  // Item procedure B-01: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Confirmed lifecycle defect: https://github.com/FlareZ123/pokemon-sims/issues/4339
  require(sim::EngineTestAccess::begin_legacy_item_resolution(
              fixture.engine, sim::Card::MysteriousTreasure),
          "Played Item did not enter the resolving-source zone.");
  const sim::State& resolving_state = sim::EngineTestAccess::state(fixture.engine);
  require(!contains(resolving_state.hand, sim::Card::MysteriousTreasure),
          "Resolving Item remained visible in ordinary hand.");
  require(!contains(resolving_state.discard, sim::Card::MysteriousTreasure),
          "Resolving Item became discard-visible before its effect finished.");
  require(contains(sim::EngineTestAccess::resolving_sources(fixture.engine),
                   sim::Card::MysteriousTreasure),
          "Played Item was not retained in the resolving-source zone.");

  require(sim::EngineTestAccess::discard_from_hand(fixture.engine, sim::Card::Grass),
          "Mandatory other-card cost was not payable.");
  const sim::State& paid_state = sim::EngineTestAccess::state(fixture.engine);
  require(contains(paid_state.discard, sim::Card::Grass),
          "Mandatory cost must already be in discard during Item resolution.");
  require(!contains(paid_state.discard, sim::Card::MysteriousTreasure),
          "Item source became discard-visible while its cost was resolving.");

  require(sim::EngineTestAccess::finish_legacy_item_resolution(
              fixture.engine, sim::Card::MysteriousTreasure),
          "Resolved Item did not move to discard at B-01 step 4.");
  const sim::State& finished_state = sim::EngineTestAccess::state(fixture.engine);
  require(contains(finished_state.discard, sim::Card::MysteriousTreasure),
          "Resolved Item source did not end in discard.");
  require(sim::EngineTestAccess::resolving_sources(fixture.engine).empty(),
          "Resolved Item source leaked from the resolving zone.");
}

void test_mysterious_treasure_finishes_in_discard_after_legal_search() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::Channeler};
  state.deck = {sim::Card::RegidragoV, sim::Card::Grass,
                sim::Card::ErikasInvitation};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine, true);

  // Mysterious Treasure discards one hand card, searches a Psychic or Dragon
  // Pokémon, then shuffles. The played source is discarded only after that Item
  // effect finishes under B-01.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Item procedure B-01: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed lifecycle defect: https://github.com/FlareZ123/pokemon-sims/issues/4339
  require(sim::EngineTestAccess::play_mysterious_treasure(fixture.engine),
          "Legal Mysterious Treasure search did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(contains(after.hand, sim::Card::RegidragoV),
          "Mysterious Treasure did not search the legal Dragon target.");
  require(contains(after.discard, sim::Card::MysteriousTreasure),
          "Mysterious Treasure source did not enter discard after resolution.");
  require(contains(after.discard, sim::Card::Channeler),
          "Mysterious Treasure mandatory cost did not enter discard.");
  require(sim::EngineTestAccess::resolving_sources(fixture.engine).empty(),
          "Mysterious Treasure remained resolving after its search finished.");
}

void test_earthen_vessel_uses_same_source_lifecycle() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::EarthenVessel, sim::Card::Channeler};
  state.deck = {sim::Card::Grass, sim::Card::Fire,
                sim::Card::ErikasInvitation};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine, true);

  // Earthen Vessel requires discarding another card, searches up to two Basic
  // Energy, and shuffles. Its played Item source follows the same B-01 lifecycle.
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Item procedure B-01: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed lifecycle defect: https://github.com/FlareZ123/pokemon-sims/issues/4339
  require(sim::EngineTestAccess::play_earthen_vessel(fixture.engine),
          "Legal Earthen Vessel search did not resolve.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  require(contains(after.hand, sim::Card::Grass) &&
              contains(after.hand, sim::Card::Fire),
          "Earthen Vessel did not move its legal Basic Energy targets to hand.");
  require(contains(after.discard, sim::Card::EarthenVessel),
          "Earthen Vessel source did not enter discard after resolution.");
  require(contains(after.discard, sim::Card::Channeler),
          "Earthen Vessel mandatory cost did not enter discard.");
  require(sim::EngineTestAccess::resolving_sources(fixture.engine).empty(),
          "Earthen Vessel remained resolving after its effect finished.");
}

}  // namespace

int main() {
  test_shared_lifecycle_keeps_played_item_out_of_hand_and_discard();
  test_mysterious_treasure_finishes_in_discard_after_legal_search();
  test_earthen_vessel_uses_same_source_lifecycle();
  return 0;
}
