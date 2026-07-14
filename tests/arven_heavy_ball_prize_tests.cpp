#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool play_heavy_ball(Engine& engine) { return engine.play_heavy_ball(); }
  static bool play_earthen_vessel_for_payload(Engine& engine) { return engine.play_earthen_vessel(true); }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
  static bool bench_from_hand(Engine& engine, const Card card, const bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
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

void test_arven_fetches_heavy_ball_for_known_prized_regidrago() {
  const sim::Scenario scenario{"arven-heavy-ball-prize", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{106};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::Arven};
  state.deck = {sim::Card::HisuianHeavyBall};
  state.prizes = {sim::Card::RegidragoV, sim::Card::Grass, sim::Card::Fire,
                  sim::Card::Dipplin, sim::Card::MawileGX, sim::Card::Guzma};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  // A prior legal deck inspection establishes K1. Arven can then find the Item,
  // and Heavy Ball may exchange itself for the known prized Basic Regidrago V:
  // https://api.pokemontcg.io/v2/cards/sv1-166 https://api.pokemontcg.io/v2/cards/swsh10-146 https://api.pokemontcg.io/v2/cards/swsh12-135
  sim::EngineTestAccess::set_deck_seen(engine);

  if (!sim::EngineTestAccess::play_arven(engine)) {
    throw std::runtime_error("Arven should fetch Hisuian Heavy Ball for the known prized Basic route.");
  }
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::HisuianHeavyBall)) {
    throw std::runtime_error("Arven should put Hisuian Heavy Ball into hand.");
  }

  if (!sim::EngineTestAccess::play_heavy_ball(engine)) {
    throw std::runtime_error("Hisuian Heavy Ball should recover the known prized Regidrago V.");
  }
  if (!contains(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("Hisuian Heavy Ball should exchange itself for Regidrago V.");
  }

  if (!sim::EngineTestAccess::bench_from_hand(engine, sim::Card::RegidragoV, false) ||
      !benched(sim::EngineTestAccess::state(engine), sim::Card::RegidragoV)) {
    throw std::runtime_error("The recovered Regidrago V should be a same-turn Bench route.");
  }
}

void test_heavy_ball_prefers_ready_now_dialga_payload() {
  // Hisuian Heavy Ball can exchange itself for a revealed Basic Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // Dialga-GX is a Basic Dragon payload, Earthen Vessel discards another hand card,
  // and Apex Dragon uses a Dragon attack from discard:
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  const sim::Scenario scenario{"heavy-ball-ready-dialga-payload", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{6356};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::HisuianHeavyBall, sim::Card::EarthenVessel};
  state.deck = {sim::Card::Grass};
  state.prizes = {sim::Card::Oricorio, sim::Card::DialgaGX, sim::Card::Dipplin};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  if (!sim::EngineTestAccess::play_heavy_ball(engine)) {
    throw std::runtime_error("Hisuian Heavy Ball should recover a prized Basic.");
  }
  const sim::State& recovered = sim::EngineTestAccess::state(engine);
  if (!contains(recovered.hand, sim::Card::DialgaGX) || contains(recovered.hand, sim::Card::Oricorio)) {
    throw std::runtime_error("Heavy Ball should choose the ready-now Dialga-GX payload before dead Oricorio compression.");
  }

  if (!sim::EngineTestAccess::play_earthen_vessel_for_payload(engine)) {
    throw std::runtime_error("Earthen Vessel should discard the recovered Dialga-GX payload.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.discard, sim::Card::DialgaGX) ||
      !contains(after.discarded_this_turn, sim::Card::DialgaGX) ||
      !sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("The Heavy Ball into Earthen Vessel route should satisfy strict-JIT payload readiness.");
  }
}

}  // namespace

int main() {
  try {
    test_arven_fetches_heavy_ball_for_known_prized_regidrago();
    test_heavy_ball_prefers_ready_now_dialga_payload();
    std::cout << "Arven Heavy Ball prize tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
