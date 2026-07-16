#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool play_brilliant_blender(Engine& engine) {
    return engine.play_brilliant_blender();
  }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

void test_blender_holds_for_live_fss_tate_connector_with_incomplete_target() {
  using namespace sim;
  const Scenario scenario{"blender-fss-tate-incomplete", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(521);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::ForestSealStone};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 1, 1, Tool::None}};
  state.hand = {Card::BrilliantBlender};
  state.deck = {Card::TateLiza, Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // Star Alchemy preserves its one-per-game search when Tate would promote an
  // Energy-incomplete VSTAR. Brilliant Blender must preserve the matching one-shot
  // payload resource until that selected attacker can pay Apex Dragon's GGF cost:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/part_011_fss_latias_override.inc#L4-L24
  assert(!EngineTestAccess::play_brilliant_blender(engine));
  assert(contains(state.hand, Card::BrilliantBlender));
  assert(contains(state.deck, Card::TateLiza));
  assert(contains(state.deck, Card::MegaDragonite));
  assert(state.discard.empty());
}

void test_blender_resolves_when_fss_tate_target_is_energy_complete() {
  using namespace sim;
  const Scenario scenario{"blender-fss-tate-complete", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(522);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::ForestSealStone};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None}};
  state.hand = {Card::BrilliantBlender};
  state.deck = {Card::TateLiza, Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // A GGF-complete promotion target makes the Star Alchemy to Tate route complete,
  // so Blender may supply the strict-JIT Dragon payload this turn:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sm7-148
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(EngineTestAccess::play_brilliant_blender(engine));
  assert(contains(state.discard, Card::BrilliantBlender));
  assert(contains(state.discard, Card::MegaDragonite));
  assert(contains(state.discarded_this_turn, Card::MegaDragonite));
}

void test_blender_resolves_when_no_fss_tate_connector_remains() {
  using namespace sim;
  const Scenario scenario{"blender-fss-no-tate", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(523);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::ForestSealStone};
  state.bench = {Pokemon{Card::RegidragoVstar, 1, 1, 1, Tool::None}};
  state.hand = {Card::BrilliantBlender};
  state.deck = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // After a legal inspection proves Tate & Liza absent from the deck, Forest Seal
  // Stone no longer supplies the incomplete promotion connector. Blender remains the
  // live strict-JIT payload outlet and may resolve:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  assert(EngineTestAccess::play_brilliant_blender(engine));
  assert(contains(state.discard, Card::BrilliantBlender));
  assert(contains(state.discard, Card::MegaDragonite));
}

void test_blender_holds_after_manual_energy_window_is_lost() {
  using namespace sim;
  const Scenario scenario{"blender-energy-window-lost", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(737);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 1, 1, Tool::None};
  state.hand = {Card::BrilliantBlender, Card::Grass};
  state.deck = {Card::MegaDragonite};
  state.manual_energy_used = true;
  EngineTestAccess::set_deck_seen(engine);

  // The one manual attachment is already spent, and no effect can attach the final
  // Grass this turn. Strict JIT must preserve Blender because its payload would
  // expire before the Active VSTAR can pay Apex Dragon's GGF cost:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#strict-jit-definition
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  assert(!EngineTestAccess::play_brilliant_blender(engine));
  assert(contains(state.hand, Card::BrilliantBlender));
  assert(contains(state.hand, Card::Grass));
  assert(contains(state.deck, Card::MegaDragonite));
  assert(state.discard.empty());
}

void test_blender_resolves_before_unused_manual_attachment() {
  using namespace sim;
  const Scenario scenario{"blender-manual-energy-live", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(738);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 1, 1, Tool::None};
  state.hand = {Card::BrilliantBlender, Card::Grass};
  state.deck = {Card::MegaDragonite};
  state.manual_energy_used = false;
  EngineTestAccess::set_deck_seen(engine);

  // The unused manual attachment can place the held final Grass after Blender, so
  // the current-turn payload route remains complete:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  assert(EngineTestAccess::play_brilliant_blender(engine));
  assert(contains(state.discard, Card::BrilliantBlender));
  assert(contains(state.discard, Card::MegaDragonite));
}

void test_blender_resolves_when_held_crispin_can_attach_final_energy() {
  using namespace sim;
  const Scenario scenario{"blender-held-crispin-live", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(739);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 1, 1, Tool::None};
  state.hand = {Card::BrilliantBlender, Card::Crispin};
  state.deck = {Card::MegaDragonite, Card::Grass, Card::Fire};
  state.manual_energy_used = true;
  EngineTestAccess::set_deck_seen(engine);

  // With two different Basic Energy types searchable, Crispin can attach the final
  // Grass even after the manual attachment has been used:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  assert(EngineTestAccess::play_brilliant_blender(engine));
  assert(contains(state.discard, Card::BrilliantBlender));
  assert(contains(state.discard, Card::MegaDragonite));
}

void test_blender_holds_when_crispin_has_only_one_energy_type() {
  using namespace sim;
  const Scenario scenario{"blender-one-type-crispin-dead", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(740);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 1, 1, Tool::None};
  state.hand = {Card::BrilliantBlender, Card::Crispin};
  state.deck = {Card::MegaDragonite, Card::Grass};
  state.manual_energy_used = true;
  EngineTestAccess::set_deck_seen(engine);

  // Crispin puts a lone searched Energy into hand and attaches none. After the
  // manual attachment is spent, that one-type resolution cannot complete GGF:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/737
  assert(!EngineTestAccess::play_brilliant_blender(engine));
  assert(contains(state.hand, Card::BrilliantBlender));
  assert(contains(state.hand, Card::Crispin));
  assert(contains(state.deck, Card::MegaDragonite));
  assert(state.discard.empty());
}

}  // namespace

int main() {
  test_blender_holds_for_live_fss_tate_connector_with_incomplete_target();
  test_blender_resolves_when_fss_tate_target_is_energy_complete();
  test_blender_resolves_when_no_fss_tate_connector_remains();
  test_blender_holds_after_manual_energy_window_is_lost();
  test_blender_resolves_before_unused_manual_attachment();
  test_blender_resolves_when_held_crispin_can_attach_final_energy();
  test_blender_holds_when_crispin_has_only_one_energy_type();
  return 0;
}
