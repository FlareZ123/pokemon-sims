#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static State& state(Engine& engine) { return engine.state_; }
  static bool play_mysterious_treasure(Engine& engine, const bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static bool play_quick_ball(Engine& engine, const bool permit_payload) {
    return engine.play_quick_ball(permit_payload);
  }
  static bool play_ultra_ball(Engine& engine, const bool permit_payload) {
    return engine.play_ultra_ball(permit_payload);
  }
  static bool play_earthen_vessel(Engine& engine, const bool permit_payload) {
    return engine.play_earthen_vessel(permit_payload);
  }
  static bool bench_from_hand(Engine& engine, const Card card, const bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
  static void play_basics_from_hand(Engine& engine) { engine.play_basics_from_hand(); }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool benched(const sim::State& state, const sim::Card card) {
  return std::any_of(state.bench.begin(), state.bench.end(), [card](const sim::Pokemon& pokemon) {
    return pokemon.card == card;
  });
}

void test_ultra_ball_prefers_tapu_over_irrelevant_fallback() {
  const sim::Scenario scenario{"ultra-ball-tapu", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{2718};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
  state.hand = {sim::Card::UltraBall, sim::Card::Dipplin, sim::Card::RegidragoV,
                sim::Card::RegidragoVstar};
  state.deck = {sim::Card::LatiasEx, sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball can search for any Pokémon after discarding two other hand cards:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  if (!sim::EngineTestAccess::play_ultra_ball(engine, false)) {
    throw std::runtime_error("Ultra Ball should take the live Tapu Lele-GX connector.");
  }

  const sim::State& after_ball = sim::EngineTestAccess::state(engine);
  if (!contains(after_ball.hand, sim::Card::TapuLeleGX) || contains(after_ball.hand, sim::Card::LatiasEx)) {
    throw std::runtime_error("Ultra Ball must select Tapu Lele-GX before an irrelevant Latias ex fallback.");
  }

  // Wonder Tag resolves when Tapu Lele-GX is played from hand onto the Bench and
  // can search the live Crispin Supporter: https://api.pokemontcg.io/v2/cards/sm2-60
  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::TapuLeleGX, true)) {
    throw std::runtime_error("Tapu Lele-GX should have Bench space for Wonder Tag.");
  }
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Crispin)) {
    throw std::runtime_error("Wonder Tag should complete the Ultra Ball to Crispin connector.");
  }
}

void test_ultra_ball_hands_a_payload_to_earthen_vessel() {
  const sim::Scenario scenario{"ultra-ball-payload-hand-off", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{6801};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::UltraBall, sim::Card::EarthenVessel, sim::Card::Dipplin,
                sim::Card::Grass, sim::Card::Grass};
  state.deck = {sim::Card::MegaDragonite, sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball can discard two other hand cards and search the deck for any Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  if (!sim::EngineTestAccess::play_ultra_ball(engine, true)) {
    throw std::runtime_error("Ultra Ball should fetch a modeled payload for the live Earthen Vessel discard.");
  }
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Ultra Ball should put Mega Dragonite ex into hand for the second Item.");
  }

  // Earthen Vessel may discard another hand card before searching Basic Energy. The
  // remaining Grass target keeps this K1 continuation legal after Ultra Ball resolves:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  if (!sim::EngineTestAccess::play_earthen_vessel(engine, true)) {
    throw std::runtime_error("Earthen Vessel should consume the fetched payload while a Basic Energy target remains.");
  }

  const sim::State& after_vessel = sim::EngineTestAccess::state(engine);
  // Regidrago VSTAR Apex Dragon can use an attack from a Dragon Pokémon in discard:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  if (!contains(after_vessel.discard, sim::Card::MegaDragonite) ||
      !contains(after_vessel.discarded_this_turn, sim::Card::MegaDragonite)) {
    throw std::runtime_error("The fetched Dragon must remain a same-turn strict-JIT payload.");
  }
}

void test_duplicate_search_items_form_current_turn_payload_chain() {
  const sim::Scenario scenario{"duplicate-search-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};

  {
    std::mt19937_64 rng{531};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
    state.hand = {sim::Card::MysteriousTreasure, sim::Card::MysteriousTreasure, sim::Card::Dipplin};
    state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
    state.discard = {sim::Card::ProfessorBurnet};
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // Mysterious Treasure discards one hand card before searching a Psychic or Dragon Pokémon:
    // https://api.pokemontcg.io/v2/cards/sm6-113
    if (!sim::EngineTestAccess::play_mysterious_treasure(engine, true) ||
        !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::MegaDragonite)) {
      throw std::runtime_error("The first Mysterious Treasure should find Mega Dragonite ex after paying Dipplin.");
    }
    if (!sim::EngineTestAccess::play_mysterious_treasure(engine, true)) {
      throw std::runtime_error("The second Mysterious Treasure should discard the fetched current-turn payload.");
    }
    const sim::State& after = sim::EngineTestAccess::state(engine);
    if (!contains(after.discard, sim::Card::MegaDragonite) ||
        !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
      throw std::runtime_error("Mega Dragonite ex must remain in this turn's discard after the second Mysterious Treasure.");
    }
  }

  {
    std::mt19937_64 rng{532};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
    state.hand = {sim::Card::QuickBall, sim::Card::QuickBall, sim::Card::Dipplin};
    state.deck = {sim::Card::TapuLeleGX, sim::Card::DialgaGX};
    state.discard = {sim::Card::ProfessorBurnet};
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // Quick Ball discards another hand card before searching a Basic Pokémon:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    if (!sim::EngineTestAccess::play_quick_ball(engine, true) ||
        !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::DialgaGX)) {
      throw std::runtime_error("The first Quick Ball should find Dialga-GX after paying Dipplin.");
    }
    if (!sim::EngineTestAccess::play_quick_ball(engine, true)) {
      throw std::runtime_error("The second Quick Ball should discard the fetched current-turn payload.");
    }
    const sim::State& after = sim::EngineTestAccess::state(engine);
    if (!contains(after.discard, sim::Card::DialgaGX) ||
        !contains(after.discarded_this_turn, sim::Card::DialgaGX)) {
      throw std::runtime_error("Dialga-GX must remain in this turn's discard after the second Quick Ball.");
    }
  }

  {
    std::mt19937_64 rng{533};
    sim::Engine engine(scenario, recipe, rng);
    sim::State state;
    state.turn = 2;
    state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
    state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
    state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None});
    state.hand = {sim::Card::UltraBall, sim::Card::UltraBall, sim::Card::RegidragoV,
                  sim::Card::Grass, sim::Card::Grass, sim::Card::Grass};
    state.deck = {sim::Card::TapuLeleGX, sim::Card::MegaDragonite};
    state.discard = {sim::Card::ProfessorBurnet};
    sim::EngineTestAccess::set_state(engine, std::move(state));

    // Ultra Ball discards two other hand cards before searching any Pokémon:
    // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    if (!sim::EngineTestAccess::play_ultra_ball(engine, true) ||
        !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::MegaDragonite)) {
      throw std::runtime_error("The first Ultra Ball should find Mega Dragonite ex after paying two legal costs.");
    }
    if (!sim::EngineTestAccess::play_ultra_ball(engine, true)) {
      throw std::runtime_error("The second Ultra Ball should discard the fetched current-turn payload.");
    }
    const sim::State& after = sim::EngineTestAccess::state(engine);
    if (!contains(after.discard, sim::Card::MegaDragonite) ||
        !contains(after.discarded_this_turn, sim::Card::MegaDragonite)) {
      throw std::runtime_error("Mega Dragonite ex must remain in this turn's discard after the second Ultra Ball.");
    }
  }
}

void test_tapu_is_preserved_when_ultra_ball_directly_finds_vstar() {
  const sim::Scenario scenario{"tapulelegx-redundant-arven", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{220};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1}};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::UltraBall,
                sim::Card::Dipplin, sim::Card::RegidragoV};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Arven};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball directly finds Regidrago VSTAR after two legal costs. Wonder Tag
  // into Arven would spend a Bench slot and the Supporter route for the same axis:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv1-166
  sim::EngineTestAccess::play_basics_from_hand(engine);
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::TapuLeleGX) ||
      benched(sim::EngineTestAccess::state(engine), sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Tapu Lele-GX should be preserved while payable Ultra Ball directly covers VSTAR.");
  }
  if (!sim::EngineTestAccess::play_ultra_ball(engine, false) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("The preserved direct Ultra Ball route should fetch Regidrago VSTAR.");
  }
}

void test_tapu_remains_live_when_ultra_ball_is_unpayable() {
  const sim::Scenario scenario{"tapulelegx-unpayable-ultra", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{221};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::UltraBall};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Arven};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Ultra Ball needs two other cards, so this copy is not a direct VSTAR route:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Wonder Tag may therefore remain the live Arven connector:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  sim::EngineTestAccess::play_basics_from_hand(engine);
  if (!benched(sim::EngineTestAccess::state(engine), sim::Card::TapuLeleGX) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Arven)) {
    throw std::runtime_error("An unpayable Ultra Ball must not suppress the live Tapu Lele-GX connector.");
  }
}

void test_wonder_tag_preserves_incomplete_tate_promotion() {
  const sim::Scenario scenario{"wonder-tag-incomplete-tate", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{4061};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None}};
  state.hand = {sim::Card::TapuLeleGX};
  state.deck = {sim::Card::TateLiza};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Wonder Tag may search Tate & Liza, but that route spends a Bench slot, the
  // Ability, and the Supporter before promoting a VSTAR that cannot pay GGF:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  sim::EngineTestAccess::play_basics_from_hand(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::TapuLeleGX) ||
      benched(after, sim::Card::TapuLeleGX) ||
      !contains(after.deck, sim::Card::TateLiza)) {
    throw std::runtime_error("Tapu Lele-GX must be preserved when Tate would promote an incomplete VSTAR.");
  }

  std::mt19937_64 forced_rng{4063};
  sim::Engine forced_engine(scenario, recipe, forced_rng);
  sim::State forced_state;
  forced_state.turn = 2;
  forced_state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  forced_state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1, sim::Tool::None}};
  forced_state.hand = {sim::Card::TapuLeleGX};
  forced_state.deck = {sim::Card::TateLiza};
  forced_state.discard = {sim::Card::MegaDragonite};
  forced_state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(forced_engine, std::move(forced_state));

  // Even if another legal route already put Tapu Lele-GX onto the Bench, Wonder Tag
  // is optional and must not fall through to Tate after rejecting the incomplete switch:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-148
  if (!sim::EngineTestAccess::bench_from_hand(forced_engine, sim::Card::TapuLeleGX, true)) {
    throw std::runtime_error("The forced Wonder Tag selector reproduction should Bench Tapu Lele-GX.");
  }
  const sim::State& forced_after = sim::EngineTestAccess::state(forced_engine);
  if (contains(forced_after.hand, sim::Card::TateLiza) ||
      !contains(forced_after.deck, sim::Card::TateLiza)) {
    throw std::runtime_error("Wonder Tag must not use the generic Tate fallback for an incomplete promotion.");
  }
}

void test_wonder_tag_fetches_tate_for_complete_promotion() {
  const sim::Scenario scenario{"wonder-tag-complete-tate", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{4062};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None}};
  state.hand = {sim::Card::TapuLeleGX};
  state.deck = {sim::Card::TateLiza};
  state.discard = {sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A complete GGF VSTAR makes Wonder Tag into Tate & Liza a direct Active-position
  // connector, so the existing legal route remains available:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  sim::EngineTestAccess::play_basics_from_hand(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!benched(after, sim::Card::TapuLeleGX) ||
      !contains(after.hand, sim::Card::TateLiza) ||
      contains(after.deck, sim::Card::TateLiza)) {
    throw std::runtime_error("Wonder Tag should fetch Tate for a complete GGF promotion target.");
  }
}

}  // namespace

int main() {
  try {
    test_ultra_ball_prefers_tapu_over_irrelevant_fallback();
    test_ultra_ball_hands_a_payload_to_earthen_vessel();
    test_duplicate_search_items_form_current_turn_payload_chain();
    test_tapu_is_preserved_when_ultra_ball_directly_finds_vstar();
    test_tapu_remains_live_when_ultra_ball_is_unpayable();
    test_wonder_tag_preserves_incomplete_tate_promotion();
    test_wonder_tag_fetches_tate_for_complete_promotion();
    std::cout << "ultra ball connector tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
