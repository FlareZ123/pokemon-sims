#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine, const bool seen) { engine.deck_seen_ = seen; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool bench_from_hand(Engine& engine, const Card card, const bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
  static bool needs_tapu_connector(Engine& engine) { return engine.needs_tapu_connector(); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_wonder_tag_yields_known_dead_burnet_to_arven() {
  const sim::Scenario scenario{"wonder-tag-k1", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{31415};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::TapuLeleGX};
  state.deck = {sim::Card::ProfessorBurnet, sim::Card::Arven,
                sim::Card::EvolutionIncense, sim::Card::ForestSealStone};
  state.prizes = {sim::Card::MegaDragonite, sim::Card::Dragapult,
                  sim::Card::GoodraVstar, sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Wonder Tag searches the deck for a Supporter after Tapu Lele-GX is played
  // from hand to the Bench: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX, true)) {
    throw std::runtime_error("Tapu Lele-GX should resolve Wonder Tag from the Bench.");
  }

  const sim::State& after = sim::EngineTestAccess::state(engine);
  // Professor Burnet searches the deck before discarding selected cards, so the
  // inspected payload-empty deck makes it dead: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Arven remains the live VSTAR connector: https://api.pokemontcg.io/v2/cards/sv1-166
  if (!contains(after.hand, sim::Card::Arven) || contains(after.hand, sim::Card::ProfessorBurnet)) {
    throw std::runtime_error("Wonder Tag should select Arven instead of a known-dead Professor Burnet.");
  }
}

void test_wonder_tag_uses_arven_vessel_for_final_energy() {
  const sim::Scenario scenario{"wonder-tag-arven-vessel-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{197};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Dipplin};
  state.deck = {sim::Card::Arven, sim::Card::EarthenVessel, sim::Card::Fire, sim::Card::Grass};
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Wonder Tag may find Arven, Arven may find Earthen Vessel, and Vessel may
  // search the final Basic Fire Energy for the turn's manual attachment:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://www.pokemon.com/us/pokemon-tcg/rules
  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX, true)) {
    throw std::runtime_error("Tapu Lele-GX should resolve Wonder Tag for the Energy connector.");
  }
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Arven)) {
    throw std::runtime_error("Wonder Tag should select Arven for the live Earthen Vessel route.");
  }

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->grass != 2 || after.active->fire != 1) {
    throw std::runtime_error("Arven into Earthen Vessel should complete GGF with the manual attachment.");
  }
  if (!after.supporter_used || !contains(after.discard, sim::Card::Arven) ||
      !contains(after.discard, sim::Card::EarthenVessel) || !contains(after.discard, sim::Card::Dipplin)) {
    throw std::runtime_error("The full Wonder Tag, Arven, and Earthen Vessel chain should resolve its costs.");
  }
}

void test_wonder_tag_uses_an_available_supporter_fallback() {
  const sim::Scenario scenario{"wonder-tag-legal-fallback", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{219};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::TapuLeleGX};
  state.deck = {sim::Card::Guzma};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Wonder Tag may search for any Supporter, so an available Guzma is a legal
  // fallback when no preferred setup Supporter remains:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sm3-115
  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX, true)) {
    throw std::runtime_error("Tapu Lele-GX should resolve Wonder Tag for the fallback test.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Guzma) || !after.deck.empty()) {
    throw std::runtime_error("Wonder Tag should take the actually available legal Supporter fallback.");
  }
}

void test_wonder_tag_tate_connector_accepts_held_payload_route() {
  const sim::Scenario scenario{"wonder-tag-tate-held-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{450};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::TateLiza, sim::Card::Dipplin};
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // Wonder Tag may search Tate & Liza. Tate may promote the GGF VSTAR, then
  // Mysterious Treasure may discard the held Dragon and search the remaining Dipplin:
  // https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!sim::EngineTestAccess::needs_tapu_connector(engine)) {
    throw std::runtime_error("Wonder Tag should recognize Tate plus the payable held-payload Item.");
  }
  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX, true) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::TateLiza)) {
    throw std::runtime_error("Wonder Tag should search Tate & Liza for the live promotion route.");
  }

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!after.active || after.active->card != sim::Card::RegidragoVstar ||
      !contains(after.discarded_this_turn, sim::Card::MegaDragonite) ||
      !contains(after.discard, sim::Card::TateLiza) ||
      !contains(after.discard, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("Wonder Tag, Tate, and Mysterious Treasure should complete the exact route.");
  }
}

void test_wonder_tag_tate_connector_rejects_empty_post_search_deck() {
  const sim::Scenario scenario{"wonder-tag-tate-empty-outlet", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{451};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite};
  state.deck = {sim::Card::TateLiza};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // After Wonder Tag removes Tate, Mysterious Treasure cannot begin its required
  // deck search from zero cards: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  if (sim::EngineTestAccess::needs_tapu_connector(engine)) {
    throw std::runtime_error("The connector must reject a held Item whose later search would see an empty deck.");
  }
}

void test_wonder_tag_tate_connector_rejects_incomplete_target() {
  const sim::Scenario scenario{"wonder-tag-tate-incomplete-target", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{452};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None}};
  // Holding a truly live Crispin suppresses the independent Wonder Tag Energy
  // connector, so this control isolates the new Tate path:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  state.hand = {sim::Card::TapuLeleGX, sim::Card::MysteriousTreasure,
                sim::Card::MegaDragonite, sim::Card::Crispin};
  state.deck = {sim::Card::TateLiza, sim::Card::Dipplin, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // Tate cannot make an Energy-incomplete VSTAR pay Apex Dragon's GGF cost:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (sim::EngineTestAccess::needs_tapu_connector(engine)) {
    throw std::runtime_error("Wonder Tag must preserve Tapu when the isolated Tate promotion target is Energy-incomplete.");
  }
}

void test_wonder_tag_tate_connector_rejects_missing_payload_cost() {
  const sim::Scenario scenario{"wonder-tag-tate-unpayable-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{453};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::TateLiza, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine, true);

  // Mysterious Treasure requires another card as its discard cost, and this route
  // specifically requires that cost to be the held Apex Dragon payload:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (sim::EngineTestAccess::needs_tapu_connector(engine)) {
    throw std::runtime_error("Wonder Tag must reject Tate when no held Dragon can pay the payload cost.");
  }
}

}  // namespace

int main() {
  try {
    test_wonder_tag_yields_known_dead_burnet_to_arven();
    test_wonder_tag_uses_arven_vessel_for_final_energy();
    test_wonder_tag_uses_an_available_supporter_fallback();
    test_wonder_tag_tate_connector_accepts_held_payload_route();
    test_wonder_tag_tate_connector_rejects_empty_post_search_deck();
    test_wonder_tag_tate_connector_rejects_incomplete_target();
    test_wonder_tag_tate_connector_rejects_missing_payload_cost();
    std::cout << "Wonder Tag connector tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
