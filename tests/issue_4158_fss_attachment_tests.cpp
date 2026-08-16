#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static Pokemon* first_open_fss_holder(Engine& engine, const bool allow_active) {
    return engine.first_open_fss_holder(allow_active);
  }
  static bool attach_fss(Engine& engine) { return engine.attach_fss(); }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

sim::Engine make_engine(std::mt19937_64& rng, sim::State state) {
  const sim::Scenario scenario{"issue-4158", sim::DciProfile::MatchupFlexJit,
                               sim::LockMode::None, true, 5};
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return engine;
}

void test_non_v_holder_is_physically_legal_and_safe_after_vstar_use() {
  sim::State state;
  state.turn = 1;
  state.vstar_power_used = true;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0};
  state.hand = {sim::Card::ForestSealStone};
  std::mt19937_64 rng{415800};
  sim::Engine engine = make_engine(rng, std::move(state));

  // Forest Seal Stone's Tool attachment text applies to a Pokémon with an open
  // Tool slot. Its Pokémon V clause separately controls access to Star Alchemy.
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Advanced Pokémon Tool procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed legality bug: https://github.com/FlareZ123/pokemon-sims/issues/4158
  sim::Pokemon* holder = sim::EngineTestAccess::first_open_fss_holder(engine, true);
  expect(holder != nullptr && holder->card == sim::Card::Oricorio,
         "A non-V Pokémon with an open Tool slot was rejected as an FSS holder.");
  expect(sim::EngineTestAccess::attach_fss(engine),
         "Forest Seal Stone did not attach to the legal non-V holder after VSTAR use.");
  expect(engine.state().active &&
             engine.state().active->tool == sim::Tool::ForestSealStone,
         "The legal non-V holder did not receive Forest Seal Stone.");
}

void test_v_holder_remains_strategy_preferred() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::Oricorio, 0, 0, 0};
  state.bench = {sim::Pokemon{sim::Card::CrobatV, 0, 0, 0}};
  state.hand = {sim::Card::ForestSealStone};
  std::mt19937_64 rng{415801};
  sim::Engine engine = make_engine(rng, std::move(state));

  // Only a Pokémon V holder gains Star Alchemy, so strategy must preserve that
  // value while the VSTAR Power remains unused:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/blob/main/CARD_CLASS_CLEANUP.md
  sim::Pokemon* holder = sim::EngineTestAccess::first_open_fss_holder(engine, true);
  expect(holder != nullptr && holder->card == sim::Card::CrobatV,
         "Forest Seal Stone stopped preferring the Star-Alchemy-capable Pokémon V holder.");
  expect(sim::EngineTestAccess::attach_fss(engine),
         "Forest Seal Stone did not attach to the preferred Pokémon V holder.");
  expect(engine.state().bench.front().tool == sim::Tool::ForestSealStone &&
             engine.state().active->tool == sim::Tool::None,
         "Forest Seal Stone did not preserve Pokémon V holder preference.");
}

void test_bench_non_v_is_physically_legal_when_active_is_disallowed() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 0};
  state.bench = {sim::Pokemon{sim::Card::Oricorio, 0, 0, 0}};
  state.hand = {sim::Card::ForestSealStone, sim::Card::Powerglass};
  state.discard = {sim::Card::Grass};
  state.manual_energy_used = true;
  std::mt19937_64 rng{415802};
  sim::Engine engine = make_engine(rng, std::move(state));

  // Physical holder legality remains broad even when strategy reserves the Active
  // Tool slot for Powerglass:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Powerglass: https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // Advanced Pokémon Tool procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  sim::Pokemon* bench_holder =
      sim::EngineTestAccess::first_open_fss_holder(engine, false);
  expect(bench_holder != nullptr && bench_holder->card == sim::Card::Oricorio,
         "A legal Benched non-V holder was unavailable when Active attachment was disallowed.");
}

}  // namespace

int main() {
  test_non_v_holder_is_physically_legal_and_safe_after_vstar_use();
  test_v_holder_remains_strategy_preferred();
  test_bench_non_v_is_physically_legal_when_active_is_disallowed();
  return 0;
}