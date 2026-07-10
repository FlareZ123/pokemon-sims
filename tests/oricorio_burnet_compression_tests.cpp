#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
  static void run_turn(Engine& engine) { engine.run_turn(); }
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

void test_oricorio_preserves_burnet_for_same_turn_readiness() {
  const sim::Scenario scenario{"oricorio-burnet-compression", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{4242};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.hand = {sim::Card::Oricorio, sim::Card::Crispin, sim::Card::ProfessorBurnet,
                sim::Card::RegidragoVstar};
  state.deck = {sim::Card::Grass, sim::Card::Fire, sim::Card::MegaDragonite};
  // Vital Dance searches the final Basic Fire Energy when Oricorio is played from
  // hand to the Bench: https://api.pokemontcg.io/v2/cards/sm2-55
  // Professor Burnet then searches the deck and discards the Dragon payload:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago V entered play on turn 1, so it may evolve on turn 2 under the core
  // evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  sim::EngineTestAccess::set_state(engine, std::move(state));

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!benched(after, sim::Card::Oricorio)) {
    throw std::runtime_error("Oricorio should be benched to preserve Professor Burnet's Supporter turn.");
  }
  if (!after.active || after.active->card != sim::Card::RegidragoVstar || after.active->grass != 2 || after.active->fire != 1) {
    throw std::runtime_error("The Oricorio/manual Energy line should evolve and complete GGF this turn.");
  }
  if (!contains(after.discard, sim::Card::ProfessorBurnet) || !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Professor Burnet should use the preserved Supporter play to discard Mega Dragonite ex.");
  }
  if (!contains(after.hand, sim::Card::Crispin)) {
    throw std::runtime_error("Crispin should remain available after Oricorio supplies the Energy axis.");
  }
  if (!sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("The Burnet discard must satisfy strict current-turn payload timing.");
  }
}

void test_oricorio_takes_last_bench_slot_over_tapu_for_burnet() {
  const sim::Scenario scenario{"oricorio-last-bench-slot", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{180};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0, sim::Tool::None},
                 sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None},
                 sim::Pokemon{sim::Card::MawileGX, 1, 0, 0, sim::Tool::None},
                 sim::Pokemon{sim::Card::DialgaGX, 1, 0, 0, sim::Tool::None}};
  state.hand = {sim::Card::TapuLeleGX, sim::Card::Oricorio, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::Crispin, sim::Card::MegaDragonite, sim::Card::Fire, sim::Card::Grass};

  // With one Bench slot left, Wonder Tag into Crispin would consume both that slot
  // and the single Supporter play: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv7-133 https://www.pokemon.com/us/pokemon-tcg/rules
  // Oricorio's Vital Dance can find the final Fire, leaving Professor Burnet to
  // discard the Dragon payload in the same turn: https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  sim::EngineTestAccess::set_state(engine, std::move(state));

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!benched(after, sim::Card::Oricorio)) {
    throw std::runtime_error("Oricorio should take the last Bench slot for the Burnet-preserving line.");
  }
  if (benched(after, sim::Card::TapuLeleGX)) {
    throw std::runtime_error("Tapu Lele-GX should not consume the last Bench slot when Oricorio preserves Burnet.");
  }
  if (!after.active || after.active->card != sim::Card::RegidragoVstar || after.active->grass != 2 || after.active->fire != 1) {
    throw std::runtime_error("Oricorio plus manual attachment should complete GGF.");
  }
  if (!contains(after.discard, sim::Card::ProfessorBurnet) || !contains(after.discard, sim::Card::MegaDragonite)) {
    throw std::runtime_error("Professor Burnet should remain playable and discard Mega Dragonite ex.");
  }
  if (!sim::EngineTestAccess::payload_ready(engine)) {
    throw std::runtime_error("The preserved Burnet line must satisfy strict current-turn payload timing.");
  }
}

sim::State four_card_bench_with_active_regi(const sim::Card fourth_filler) {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::LatiasEx, 1},
                 sim::Pokemon{sim::Card::MawileGX, 1},
                 sim::Pokemon{sim::Card::DialgaGX, 1},
                 sim::Pokemon{fourth_filler, 1}};
  return state;
}

void test_oricorio_takes_final_slot_over_redundant_regidrago() {
  const sim::Scenario scenario{"oricorio-before-redundant-regi", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{228};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state = four_card_bench_with_active_regi(sim::Card::TapuLeleGX);
  state.hand = {sim::Card::RegidragoV, sim::Card::Oricorio,
                sim::Card::RegidragoVstar, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::Fire, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // The Bench limit is five, and Vital Dance requires Oricorio to be played from
  // hand to the Bench: https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sm2-55
  sim::EngineTestAccess::play_basics_from_hand(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!benched(after, sim::Card::Oricorio) || !contains(after.hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("Oricorio should take the final slot before a redundant Regidrago V.");
  }
}

void test_tapu_takes_final_slot_over_redundant_regidrago() {
  const sim::Scenario scenario{"tapu-before-redundant-regi", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{229};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state = four_card_bench_with_active_regi(sim::Card::Oricorio);
  state.hand = {sim::Card::RegidragoV, sim::Card::TapuLeleGX};
  state.deck = {sim::Card::Arven, sim::Card::EvolutionIncense, sim::Card::RegidragoVstar};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Wonder Tag requires Tapu Lele-GX to be played from hand to Bench and may find
  // Arven for the missing VSTAR axis: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // https://api.pokemontcg.io/v2/cards/sv1-166
  sim::EngineTestAccess::play_basics_from_hand(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!benched(after, sim::Card::TapuLeleGX) || !contains(after.hand, sim::Card::Arven) ||
      !contains(after.hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("Tapu Lele-GX should take the final slot before a redundant Regidrago V.");
  }
}

void test_redundant_regidrago_uses_final_slot_without_live_connector() {
  const sim::Scenario scenario{"redundant-regi-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{230};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state = four_card_bench_with_active_regi(sim::Card::TapuLeleGX);
  state.hand = {sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  sim::EngineTestAccess::play_basics_from_hand(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (after.bench.size() != 5U || !benched(after, sim::Card::RegidragoV)) {
    throw std::runtime_error("The redundant Regidrago V should use the final slot when no live connector exists.");
  }
}

}  // namespace

int main() {
  test_oricorio_preserves_burnet_for_same_turn_readiness();
  test_oricorio_takes_last_bench_slot_over_tapu_for_burnet();
  test_oricorio_takes_final_slot_over_redundant_regidrago();
  test_tapu_takes_final_slot_over_redundant_regidrago();
  test_redundant_regidrago_uses_final_slot_without_live_connector();
  return 0;
}
