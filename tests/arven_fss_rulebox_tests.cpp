#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& e, State s) { e.state_ = std::move(s); }
  static State& state(Engine& e) { return e.state_; }
  static void set_deck_seen(Engine& e) { e.deck_seen_ = true; }
  static bool arven(Engine& e) { return e.play_arven(); }
  static bool mysterious_treasure(Engine& e) { return e.play_mysterious_treasure(true); }
  static bool quick_ball(Engine& e) { return e.play_quick_ball(true); }
  static bool earthen_vessel(Engine& e) { return e.play_earthen_vessel(true); }
  static bool payload_ready(const Engine& e) { return e.payload_ready(); }
  static bool attach(Engine& e) { return e.attach_fss(); }
  static bool attach_powerglass(Engine& e) { return e.attach_powerglass(); }
  static bool resolve_powerglass(Engine& e) { return e.resolve_powerglass_end_turn(); }
  static bool star(Engine& e) { return e.use_fss(); }
  static bool gladion(Engine& e) { return e.play_gladion(); }
};
}  // namespace sim

bool has(const std::vector<sim::Card>& cards, sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

sim::State arven_payload_only_state(std::vector<sim::Card> deck) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Arven, sim::Card::MegaDragonite};
  state.deck = std::move(deck);
  return state;
}

void test_arven_finds_mysterious_treasure_for_held_payload() {
  const sim::Scenario scenario{"arven-held-payload-treasure", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(64701);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, arven_payload_only_state({sim::Card::MysteriousTreasure, sim::Card::Dipplin}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Arven searches Mysterious Treasure, whose one-card discard places the held
  // Dragon payload in discard before its legal Dipplin search:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/647
  if (!sim::EngineTestAccess::arven(engine) ||
      !has(sim::EngineTestAccess::state(engine).hand, sim::Card::MysteriousTreasure) ||
      !sim::EngineTestAccess::mysterious_treasure(engine) ||
      !has(sim::EngineTestAccess::state(engine).discarded_this_turn, sim::Card::MegaDragonite) ||
      !sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("Arven must complete the target-legal Mysterious Treasure held-payload route.");
  }
}

void test_arven_finds_quick_ball_for_held_payload() {
  const sim::Scenario scenario{"arven-held-payload-quick-ball", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(64702);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, arven_payload_only_state({sim::Card::QuickBall, sim::Card::RegidragoV}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Quick Ball may discard the held Dragon and legally search the known Basic:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/647
  if (!sim::EngineTestAccess::arven(engine) ||
      !has(sim::EngineTestAccess::state(engine).hand, sim::Card::QuickBall) ||
      !sim::EngineTestAccess::quick_ball(engine) ||
      !has(sim::EngineTestAccess::state(engine).discarded_this_turn, sim::Card::MegaDragonite) ||
      !sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("Arven must complete the target-legal Quick Ball held-payload route.");
  }
}

void test_arven_finds_earthen_vessel_for_held_payload() {
  const sim::Scenario scenario{"arven-held-payload-vessel", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(64703);
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, arven_payload_only_state({sim::Card::EarthenVessel, sim::Card::Grass}));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Earthen Vessel may discard the held Dragon and legally search Basic Energy:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/647
  if (!sim::EngineTestAccess::arven(engine) ||
      !has(sim::EngineTestAccess::state(engine).hand, sim::Card::EarthenVessel) ||
      !sim::EngineTestAccess::earthen_vessel(engine) ||
      !has(sim::EngineTestAccess::state(engine).discarded_this_turn, sim::Card::MegaDragonite) ||
      !sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("Arven must complete the target-legal Earthen Vessel held-payload route.");
  }
}

void test_arven_rejects_dead_held_payload_routes() {
  const sim::DeckRecipe recipe = sim::baseline_recipe();

  {
    const sim::Scenario scenario{"arven-no-held-payload", sim::DciProfile::StrictJit,
                                 sim::LockMode::None, false, 4};
    std::mt19937_64 rng(64704);
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = arven_payload_only_state({sim::Card::MysteriousTreasure, sim::Card::Dipplin});
    state.hand = {sim::Card::Arven};
    sim::EngineTestAccess::set_state(engine, std::move(state));
    sim::EngineTestAccess::set_deck_seen(engine);
    if (sim::EngineTestAccess::arven(engine)) {
      throw std::runtime_error("Arven must hold when no Dragon payload is held.");
    }
  }

  {
    const sim::Scenario scenario{"arven-held-payload-item-lock", sim::DciProfile::StrictJit,
                                 sim::LockMode::FullItem, false, 4};
    std::mt19937_64 rng(64705);
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_state(
        engine, arven_payload_only_state({sim::Card::MysteriousTreasure, sim::Card::Dipplin}));
    sim::EngineTestAccess::set_deck_seen(engine);
    if (sim::EngineTestAccess::arven(engine)) {
      throw std::runtime_error("Arven must hold when Item lock blocks the promised outlet.");
    }
  }

  {
    const sim::Scenario scenario{"arven-held-payload-off-class", sim::DciProfile::StrictJit,
                                 sim::LockMode::None, false, 4};
    std::mt19937_64 rng(64706);
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_state(
        engine, arven_payload_only_state({sim::Card::MysteriousTreasure, sim::Card::Grass}));
    sim::EngineTestAccess::set_deck_seen(engine);
    if (sim::EngineTestAccess::arven(engine)) {
      throw std::runtime_error("Arven must hold when Mysterious Treasure has no class-legal target.");
    }
  }

  {
    const sim::Scenario scenario{"arven-held-payload-empty-after-search", sim::DciProfile::StrictJit,
                                 sim::LockMode::None, false, 4};
    std::mt19937_64 rng(64707);
    sim::Engine engine(scenario, recipe, rng);
    sim::EngineTestAccess::set_state(
        engine, arven_payload_only_state({sim::Card::MysteriousTreasure}));
    sim::EngineTestAccess::set_deck_seen(engine);
    if (sim::EngineTestAccess::arven(engine)) {
      throw std::runtime_error("Arven must hold when taking the Item would leave no search target.");
    }
  }

  {
    const sim::Scenario scenario{"arven-held-payload-supporter-spent", sim::DciProfile::StrictJit,
                                 sim::LockMode::None, false, 4};
    std::mt19937_64 rng(64708);
    sim::Engine engine(scenario, recipe, rng);
    sim::State state = arven_payload_only_state({sim::Card::MysteriousTreasure, sim::Card::Dipplin});
    state.supporter_used = true;
    sim::EngineTestAccess::set_state(engine, std::move(state));
    sim::EngineTestAccess::set_deck_seen(engine);
    if (sim::EngineTestAccess::arven(engine)) {
      throw std::runtime_error("Arven must remain unavailable after the turn's Supporter play.");
    }
  }

  // Known-empty and off-class searches cannot be played solely for their cost:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
  // Supporter and Item timing: https://www.pokemon.com/us/pokemon-tcg/rules
}

void test_arven_preserves_blender_priority_over_held_payload_item() {
  const sim::Scenario scenario{"arven-held-payload-blender-priority", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(64709);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = arven_payload_only_state(
      {sim::Card::BrilliantBlender, sim::Card::Dragapult,
       sim::Card::MysteriousTreasure, sim::Card::Dipplin});
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Brilliant Blender directly searches and discards a deck payload, so it remains
  // earlier than a held-payload discard Item in Arven's candidate order:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  if (!sim::EngineTestAccess::arven(engine) ||
      !has(sim::EngineTestAccess::state(engine).hand, sim::Card::BrilliantBlender) ||
      has(sim::EngineTestAccess::state(engine).hand, sim::Card::MysteriousTreasure)) {
    throw std::runtime_error("Arven must preserve direct Brilliant Blender priority.");
  }
}

void test_arven_fss_known_prize_route() {
  const sim::Scenario scenario{"arven-fss-k1", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(211);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Arven};
  state.deck = {sim::Card::ForestSealStone, sim::Card::Gladion};
  state.prizes = {sim::Card::RegidragoVstar, sim::Card::MegaDragonite, sim::Card::Dragapult,
                  sim::Card::GoodraVstar, sim::Card::DialgaGX, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Arven is a Supporter and Forest Seal Stone is a Tool after the erratum:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  if (!sim::EngineTestAccess::arven(engine) ||
      !has(sim::EngineTestAccess::state(engine).hand, sim::Card::ForestSealStone)) {
    throw std::runtime_error("Arven must retain the K1 Forest Seal Stone route through Item lock.");
  }
  // Star Alchemy finds Gladion for the known Prize route:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm4-95
  if (!sim::EngineTestAccess::attach(engine) || !sim::EngineTestAccess::star(engine) ||
      !has(sim::EngineTestAccess::state(engine).hand, sim::Card::Gladion)) {
    throw std::runtime_error("Forest Seal Stone must find Gladion after the K1 search.");
  }

  sim::State& next = sim::EngineTestAccess::state(engine);
  next.turn = 3;
  next.supporter_used = false;
  if (!sim::EngineTestAccess::gladion(engine) ||
      !has(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("Gladion must recover the known prized VSTAR on the following turn.");
  }
}

void test_arven_finds_powerglass_after_vstar_power_is_spent() {
  const sim::Scenario scenario{"arven-powerglass-item-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(212);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Arven};
  state.deck = {sim::Card::Powerglass};
  state.discard = {sim::Card::Fire, sim::Card::MegaDragonite};
  state.vstar_power_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Arven may search Powerglass as its Pokémon Tool target. Pokémon Tools remain
  // playable through Item lock, and Powerglass can recover the missing Fire Energy
  // from discard when its holder is Active at end of turn:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  // https://api.pokemontcg.io/v2/cards/sv6pt5-63
  if (!sim::EngineTestAccess::arven(engine) ||
      !has(sim::EngineTestAccess::state(engine).hand, sim::Card::Powerglass)) {
    throw std::runtime_error(
        "Arven must find the live Powerglass Tool route after the VSTAR Power is spent.");
  }
}

void test_held_fss_preserves_active_slot_for_arven_powerglass() {
  const sim::Scenario scenario{"held-fss-arven-powerglass", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(4011);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::ForestSealStone, sim::Card::Arven};
  state.deck = {sim::Card::Powerglass};
  state.discard = {sim::Card::Fire, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.manual_energy_used = true;
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Arven can search Powerglass, whose end-of-turn effect needs the Active holder's
  // only Tool slot. Forest Seal Stone must remain in hand when no alternate Pokémon V
  // can hold it and the manual Energy attachment has already been spent:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://www.pokemon.com/us/pokemon-tcg/rules
  if (sim::EngineTestAccess::attach(engine)) {
    throw std::runtime_error("Forest Seal Stone must not consume the live Powerglass Tool slot.");
  }
  if (sim::EngineTestAccess::state(engine).active->tool != sim::Tool::None ||
      !has(sim::EngineTestAccess::state(engine).hand, sim::Card::ForestSealStone)) {
    throw std::runtime_error("The Active Tool slot and held Forest Seal Stone must be preserved.");
  }
  if (!sim::EngineTestAccess::arven(engine) ||
      !has(sim::EngineTestAccess::state(engine).hand, sim::Card::Powerglass)) {
    throw std::runtime_error("Arven must search the preserved Powerglass route.");
  }
  if (!sim::EngineTestAccess::attach_powerglass(engine) ||
      !sim::EngineTestAccess::resolve_powerglass(engine) ||
      sim::EngineTestAccess::state(engine).active->fire != 1) {
    throw std::runtime_error("Powerglass must use the reserved slot and recover Fire.");
  }
}

void test_fss_attaches_when_manual_energy_route_remains_live() {
  const sim::Scenario scenario{"fss-live-manual-energy", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(4012);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::ForestSealStone, sim::Card::Arven};
  state.deck = {sim::Card::Powerglass, sim::Card::Fire};
  state.discard = {sim::Card::Fire, sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // With the manual attachment unused, Star Alchemy can still search the missing
  // Basic Energy for that attachment, so Forest Seal Stone remains a live Tool:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://www.pokemon.com/us/pokemon-tcg/rules
  if (!sim::EngineTestAccess::attach(engine) ||
      sim::EngineTestAccess::state(engine).active->tool != sim::Tool::ForestSealStone) {
    throw std::runtime_error("Forest Seal Stone should attach while its manual-Energy route is live.");
  }
}

int main() {
  test_arven_finds_mysterious_treasure_for_held_payload();
  test_arven_finds_quick_ball_for_held_payload();
  test_arven_finds_earthen_vessel_for_held_payload();
  test_arven_rejects_dead_held_payload_routes();
  test_arven_preserves_blender_priority_over_held_payload_item();
  test_arven_fss_known_prize_route();
  test_arven_finds_powerglass_after_vstar_power_is_spent();
  test_held_fss_preserves_active_slot_for_arven_powerglass();
  test_fss_attaches_when_manual_energy_route_remains_live();
}
