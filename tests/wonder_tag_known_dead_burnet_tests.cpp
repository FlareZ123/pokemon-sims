#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool bench_from_hand(Engine& engine, const Card card, const bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
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

}  // namespace

int main() {
  try {
    test_wonder_tag_yields_known_dead_burnet_to_arven();
    std::cout << "Wonder Tag known-dead Burnet tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
