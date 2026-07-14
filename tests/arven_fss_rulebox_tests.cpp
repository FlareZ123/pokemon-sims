#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& e, State s) { e.state_ = std::move(s); }
  static State& state(Engine& e) { return e.state_; }
  static bool arven(Engine& e) { return e.play_arven(); }
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
  test_arven_fss_known_prize_route();
  test_arven_finds_powerglass_after_vstar_power_is_spent();
  test_held_fss_preserves_active_slot_for_arven_powerglass();
  test_fss_attaches_when_manual_energy_route_remains_live();
}
