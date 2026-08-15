#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static bool oricorio_ability_available(const Engine& engine) {
    return engine.ability_available_for_pokemon_garbodor(Card::Oricorio);
  }
  static bool needs_oricorio(const Engine& engine) {
    return engine.needs_oricorio_connector();
  }
  static bool bench_oricorio(Engine& engine) {
    return engine.bench_oricorio_if_useful();
  }
  static int hand_oricorio(const Engine& engine) {
    return engine.hand_count(Card::Oricorio);
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_garbotoxin_rejects_oricorio_before_bench_cost() {
  const sim::Scenario scenario{"garbodor-shake-ability-lock/issue-3899",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(3899);
  sim::Engine engine(scenario, recipe, rng, nullptr);

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::Oricorio};
  state.deck = {sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Garbotoxin removes the other Pokemon's Ability while the modeled Tool lock is
  // active, so Vital Dance cannot justify paying Oricorio's Bench-space cost.
  // Garbodor / Garbotoxin: https://api.pokemontcg.io/v2/cards/xy9-57
  // Oricorio / Vital Dance: https://api.pokemontcg.io/v2/cards/sm2-55
  // Ability procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  expect(!sim::EngineTestAccess::oricorio_ability_available(engine),
         "Garbodor scenario must suppress Oricorio's Ability.");
  expect(!sim::EngineTestAccess::needs_oricorio(engine),
         "A lock-dead Oricorio must not be admitted as an Energy connector.");
  expect(!sim::EngineTestAccess::bench_oricorio(engine),
         "A lock-dead Oricorio must not consume a Bench slot.");
  expect(sim::EngineTestAccess::hand_oricorio(engine) == 1,
         "Rejected Oricorio must remain in hand.");
}
}  // namespace

int main() {
  try {
    test_garbotoxin_rejects_oricorio_before_bench_cost();
    std::cout << "Issue 3899 Oricorio Garbotoxin admission tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
