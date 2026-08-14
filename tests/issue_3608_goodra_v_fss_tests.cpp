#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static State& state(Engine& engine) { return engine.state_; }
  static bool attach_fss(Engine& engine) { return engine.attach_fss(); }
};
}  // namespace sim

void goodra_vstar_is_a_legal_forest_seal_stone_holder() {
  const sim::Scenario scenario{"issue-3608-goodra-vstar-holder", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 2};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(360801);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::GoodraVstar, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::ForestSealStone};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Hisuian Goodra VSTAR is a Pokémon VSTAR, and effects that refer to Pokémon V
  // include Pokémon VSTAR. Forest Seal Stone may therefore attach to it when its
  // Tool slot is open:
  // https://api.pokemontcg.io/v2/cards/swsh11-136
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://github.com/FlareZ123/pokemon-sims/issues/3608
  if (!sim::is_pokemon_v(sim::Card::GoodraVstar) ||
      !sim::EngineTestAccess::attach_fss(engine) ||
      !sim::EngineTestAccess::state(engine).active ||
      sim::EngineTestAccess::state(engine).active->tool != sim::Tool::ForestSealStone) {
    throw std::runtime_error(
        "Hisuian Goodra VSTAR with an open Tool slot must accept Forest Seal Stone.");
  }
}

void occupied_goodra_tool_slot_is_rejected() {
  const sim::Scenario scenario{"issue-3608-goodra-occupied-tool", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 2};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(360802);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::GoodraVstar, 1, 0, 0, sim::Tool::ForestSealStone};
  state.hand = {sim::Card::ForestSealStone};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // A Pokémon can have only one Pokémon Tool attached at a time:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // https://github.com/FlareZ123/pokemon-sims/issues/3608
  if (sim::EngineTestAccess::attach_fss(engine)) {
    throw std::runtime_error("An occupied Hisuian Goodra VSTAR Tool slot must reject another Tool.");
  }
}

void non_pokemon_v_holder_is_rejected() {
  const sim::Scenario scenario{"issue-3608-non-v-holder", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 2};
  const sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng(360803);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.hand = {sim::Card::ForestSealStone};
  sim::EngineTestAccess::set_state(engine, std::move(state));

  // Forest Seal Stone grants its VSTAR Power specifically to the Pokémon V it is
  // attached to. Tapu Lele-GX is a Pokémon-GX, so this simulator route must not
  // treat it as a Pokémon V holder:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://github.com/FlareZ123/pokemon-sims/issues/3608
  if (sim::is_pokemon_v(sim::Card::TapuLeleGX) || sim::EngineTestAccess::attach_fss(engine)) {
    throw std::runtime_error("Forest Seal Stone holder admission must remain limited to Pokémon V.");
  }
}

int main() {
  goodra_vstar_is_a_legal_forest_seal_stone_holder();
  occupied_goodra_tool_slot_is_rejected();
  non_pokemon_v_holder_is_rejected();
}
