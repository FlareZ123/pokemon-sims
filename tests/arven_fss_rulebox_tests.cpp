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
    throw std::runtime_error("Arven must find the live Powerglass Tool route after the VSTAR Power is spent.");
  }
}

int main() {
  test_arven_fss_known_prize_route();
  test_arven_finds_powerglass_after_vstar_power_is_spent();
}
