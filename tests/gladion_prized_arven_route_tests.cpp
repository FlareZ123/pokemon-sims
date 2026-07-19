#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static void set_known_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};
}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

const sim::DeckRecipe& recipe() {
  static const sim::DeckRecipe value = sim::baseline_recipe();
  return value;
}

sim::State base_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 2, 1, sim::Tool::None};
  state.hand = {sim::Card::Gladion};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::ForestSealStone,
                sim::Card::BrilliantBlender, sim::Card::Dragapult};
  state.prizes = {sim::Card::Arven, sim::Card::RegidragoV, sim::Card::Crispin,
                  sim::Card::Channeler, sim::Card::Grass, sim::Card::Fire};
  return state;
}

void known_prized_arven_beats_redundant_basic() {
  const sim::Scenario scenario{"issue-969-positive", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{96901};
  sim::Engine engine(scenario, recipe(), rng);
  sim::EngineTestAccess::set_known_state(engine, base_state());

  // Gladion may take Arven from the known Prize set. On the next turn Arven finds
  // Brilliant Blender and Forest Seal Stone, Star Alchemy finds Regidrago VSTAR,
  // and Blender supplies the strict-JIT payload while the Active already has GGF:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/969
  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion did not resolve for the deterministic prized-Arven route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (!contains(after.hand, sim::Card::Arven) ||
      contains(after.hand, sim::Card::RegidragoV) ||
      !contains(after.prizes, sim::Card::RegidragoV)) {
    throw std::runtime_error("Gladion failed to prefer the deterministic prized-Arven route.");
  }
}

void direct_prized_vstar_remains_first_priority() {
  const sim::Scenario scenario{"issue-969-vstar-control", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{96902};
  sim::Engine engine(scenario, recipe(), rng);
  sim::State state = base_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::RegidragoVstar));
  state.prizes[2] = sim::Card::RegidragoVstar;
  sim::EngineTestAccess::set_known_state(engine, std::move(state));

  // An immediately missing known-prized VSTAR outranks the slower Arven connector:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/969
  if (!sim::EngineTestAccess::play_gladion(engine) ||
      !contains(sim::EngineTestAccess::state(engine).hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error("The prized-Arven route displaced direct VSTAR recovery.");
  }
}

void incomplete_connector_route_does_not_take_arven() {
  const sim::Scenario scenario{"issue-969-missing-blender", sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 5};
  std::mt19937_64 rng{96903};
  sim::Engine engine(scenario, recipe(), rng);
  sim::State state = base_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(), sim::Card::BrilliantBlender));
  sim::EngineTestAccess::set_known_state(engine, std::move(state));

  // Arven cannot guarantee the documented route when the inspected deck has no
  // Brilliant Blender, so the narrow selector must not claim that connector:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/969
  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion unexpectedly declined its mandatory critical exchange.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (contains(after.hand, sim::Card::Arven) ||
      !contains(after.hand, sim::Card::RegidragoV)) {
    throw std::runtime_error("Gladion selected Arven without the complete inspected connector route.");
  }
}

void item_lock_blocks_the_future_item_route() {
  const sim::Scenario scenario{"issue-969-item-lock", sim::DciProfile::StrictJit,
                               sim::LockMode::FullItem, false, 5};
  std::mt19937_64 rng{96904};
  sim::Engine engine(scenario, recipe(), rng);
  sim::EngineTestAccess::set_known_state(engine, base_state());

  // Brilliant Blender is an Item, so a full Item lock makes the future Arven route
  // incomplete even though Forest Seal Stone is now a Pokémon Tool:
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  // https://github.com/FlareZ123/pokemon-sims/issues/969
  if (!sim::EngineTestAccess::play_gladion(engine)) {
    throw std::runtime_error("Gladion unexpectedly declined the locked-state exchange.");
  }
  if (contains(sim::EngineTestAccess::state(engine).hand, sim::Card::Arven)) {
    throw std::runtime_error("Gladion selected the prized Arven route through full Item lock.");
  }
}

void seed_299_full_trace_selects_arven_and_reaches_turn_three() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  if (!scenario) throw std::runtime_error("Missing strict-jit/go-second scenario.");
  std::mt19937_64 rng{299};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, recipe(), rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  const bool selected_arven = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("Exchanged Gladion for Arven to guarantee the next-turn Forest Seal Stone and Brilliant Blender route.") !=
               std::string::npos;
      });
  // Seed 299 preserves #969's full-trace boundary. Arven is the only known Prize
  // connector that produces a current legal future VSTAR plus Blender route because
  // no held Dragon can pay Mysterious Treasure for same-turn strict-JIT completion:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://github.com/FlareZ123/pokemon-sims/issues/969
  // Seed 19 is now the distinct same-turn Treasure route: https://github.com/FlareZ123/pokemon-sims/issues/1059
  if (!selected_arven || outcome.first_ready_turn != 3) {
    throw std::runtime_error("Seed 299 did not preserve the deterministic prized-Arven T3 route.");
  }
}

}  // namespace

int main() {
  known_prized_arven_beats_redundant_basic();
  direct_prized_vstar_remains_first_priority();
  incomplete_connector_route_does_not_take_arven();
  item_lock_blocks_the_future_item_route();
  seed_299_full_trace_selects_arven_and_reaches_turn_three();
  return 0;
}
