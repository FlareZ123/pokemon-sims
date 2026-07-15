#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <string>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void record_ready(Engine& engine) { engine.record_ready(); }
  static bool attach_fss(Engine& engine) { return engine.attach_fss(); }
  static bool attach_powerglass(Engine& engine) { return engine.attach_powerglass(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
};

}  // namespace sim

namespace {

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool benched(const sim::State& state, const sim::Card card) {
  return std::any_of(state.bench.begin(), state.bench.end(), [card](const sim::Pokemon& pokemon) {
    return pokemon.card == card;
  });
}

void test_star_alchemy_uses_crispin_for_same_turn_ggf() {
  using namespace sim;
  const Scenario scenario{"fss-crispin-completion", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(156);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 1, 0, Tool::ForestSealStone};
  state.hand = {Card::RegidragoVstar};
  state.deck = {Card::Crispin, Card::EarthenVessel, Card::Grass, Card::Fire};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};

  // Crispin attaches one selected Energy and puts the other into hand, while a player
  // may manually attach one Energy during the turn: https://api.pokemontcg.io/v2/cards/sv7-133
  // https://www.pokemon.com/us/pokemon-tcg/rules
  EngineTestAccess::run_turn(engine);
  EngineTestAccess::record_ready(engine);

  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(state.active->grass == 2 && state.active->fire == 1);
  assert(count(state.discard, Card::Crispin) == 1);
  assert(count(state.discard, Card::EarthenVessel) == 0);
  assert(count(state.deck, Card::EarthenVessel) == 1);
  assert(EngineTestAccess::outcome(engine).first_ready_turn == 2);
}

void test_star_alchemy_keeps_vessel_priority_without_same_turn_crispin_completion() {
  using namespace sim;
  const Scenario scenario{"fss-vessel-future-energy", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(157);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 0, 0, Tool::ForestSealStone};
  state.hand = {Card::RegidragoVstar, Card::Dipplin};
  state.deck = {Card::Crispin, Card::EarthenVessel, Card::Grass, Card::Fire};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};

  // Earthen Vessel puts Basic Energy into hand rather than attaching it, so the
  // three-missing-Energy state cannot be completed by either route this turn: https://api.pokemontcg.io/v2/cards/sv4-163
  EngineTestAccess::run_turn(engine);
  EngineTestAccess::record_ready(engine);

  assert(count(state.discard, Card::EarthenVessel) == 1);
  assert(count(state.deck, Card::Crispin) == 1);
  assert(EngineTestAccess::outcome(engine).first_ready_turn == 0);
}

void test_star_alchemy_fetches_oricorio_when_crispin_blocks_burnet() {
  using namespace sim;
  const Scenario scenario{"fss-oricorio-preserves-burnet", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(187);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.bench = {Pokemon{Card::RegidragoV, 1, 0, 0, Tool::ForestSealStone}};
  state.hand = {Card::ProfessorBurnet};
  state.deck = {Card::Crispin, Card::Oricorio, Card::Fire, Card::MegaDragonite};

  // Star Alchemy may search any card and should take Oricorio when Crispin would
  // consume the Supporter play needed by Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Oricorio's Vital Dance can search up to 2 Basic Energy, so one final Fire is
  // enough for the manual attachment: https://api.pokemontcg.io/v2/cards/sm2-55
  // Burnet then searches and discards the Dragon payload in the same turn:
  // https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Only one Supporter and one manual attachment are available each turn:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  EngineTestAccess::run_turn(engine);
  EngineTestAccess::record_ready(engine);

  assert(state.vstar_power_used);
  assert(benched(state, Card::Oricorio));
  assert(count(state.discard, Card::Crispin) == 0);
  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(state.active->grass == 2 && state.active->fire == 1);
  assert(count(state.discard, Card::ProfessorBurnet) == 1);
  assert(count(state.discard, Card::MegaDragonite) == 1);
  assert(EngineTestAccess::outcome(engine).first_ready_turn == 2);
}

void test_k0_star_alchemy_may_take_an_unknown_any_card_fallback() {
  using namespace sim;
  const Scenario scenario{"fss-any-card-fallback", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(225);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::ForestSealStone};
  state.deck = {Card::TeamYellsCheer};

  // Before legal deck inspection, fixed-list setup targets remain plausible under K0.
  // Star Alchemy may therefore resolve and discover only the fallback card:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  assert(EngineTestAccess::use_fss(engine));
  assert(state.vstar_power_used);
  assert(count(state.hand, Card::TeamYellsCheer) == 1);
  assert(state.deck.empty());
}

void test_k1_star_alchemy_holds_when_only_irrelevant_serena_remains() {
  using namespace sim;
  const Scenario scenario{"fss-k1-irrelevant-serena", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(563);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::ForestSealStone};
  state.deck = {Card::Serena};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // Forest Seal Stone supplies one game-wide VSTAR Power. Serena cannot search or
  // attach the missing Fire Energy, so K1 must preserve Star Alchemy:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  assert(!EngineTestAccess::use_fss(engine));
  assert(!state.vstar_power_used);
  assert(state.hand.empty());
  assert(state.deck == std::vector<Card>{Card::Serena});
}

void test_k1_star_alchemy_uses_a_live_energy_target() {
  using namespace sim;
  const Scenario scenario{"fss-k1-live-energy", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(564);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::ForestSealStone};
  state.deck = {Card::Fire};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // Star Alchemy may search the missing Fire Energy, which can be manually attached
  // to satisfy Apex Dragon's GGF cost: https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(EngineTestAccess::use_fss(engine));
  assert(state.vstar_power_used);
  assert(count(state.hand, Card::Fire) == 1);
}

void test_k1_star_alchemy_uses_a_live_payload_outlet() {
  using namespace sim;
  const Scenario scenario{"fss-k1-live-payload", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(565);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::ForestSealStone};
  state.deck = {Card::BrilliantBlender, Card::MegaDragonite};
  EngineTestAccess::set_deck_seen(engine);

  // Brilliant Blender searches and discards a Dragon payload from deck, so it is a
  // live strict-JIT Star Alchemy target: https://api.pokemontcg.io/v2/cards/sv8-164
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  assert(EngineTestAccess::use_fss(engine));
  assert(state.vstar_power_used);
  assert(count(state.hand, Card::BrilliantBlender) == 1);
}

void test_k1_star_alchemy_uses_a_live_vstar_target() {
  using namespace sim;
  const Scenario scenario{"fss-k1-live-vstar", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(566);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoV, 1, 2, 1, Tool::ForestSealStone};
  state.deck = {Card::RegidragoVstar};
  EngineTestAccess::set_deck_seen(engine);

  // Regidrago VSTAR is the direct missing Evolution target for the established Basic:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  assert(EngineTestAccess::use_fss(engine));
  assert(state.vstar_power_used);
  assert(count(state.hand, Card::RegidragoVstar) == 1);
}

void test_arven_fss_fetches_raw_energy_after_supporter_use() {
  using namespace sim;
  const Scenario scenario{"arven-fss-raw-energy", DciProfile::StrictJit, LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(206);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.hand = {Card::Arven};
  state.deck = {Card::ForestSealStone, Card::Crispin, Card::Fire};
  state.discard = {Card::MegaDragonite};
  state.discarded_this_turn = {Card::MegaDragonite};

  // Arven uses the turn's Supporter play before Forest Seal Stone resolves:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // Star Alchemy may search the missing Fire Energy directly, and the unused manual
  // attachment completes GGF while Crispin remains unplayable this turn:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv7-133
  EngineTestAccess::run_turn(engine);
  EngineTestAccess::record_ready(engine);

  assert(state.supporter_used);
  assert(state.vstar_power_used);
  assert(state.active && state.active->card == Card::RegidragoVstar);
  assert(state.active->grass == 2 && state.active->fire == 1);
  assert(count(state.discard, Card::Arven) == 1);
  assert(count(state.hand, Card::Crispin) == 0);
  assert(count(state.deck, Card::Crispin) == 1);
  assert(EngineTestAccess::outcome(engine).first_ready_turn == 2);
}

void test_fss_trace_names_vstar_holder() {
  using namespace sim;
  const Scenario scenario{"fss-vstar-holder-trace", DciProfile::StrictJit,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(350);
  TraceLog trace;
  trace.enabled = true;
  Engine engine(scenario, recipe, rng, &trace);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::ForestSealStone};

  // Pokémon V includes Pokémon VSTAR, and Forest Seal Stone may attach to a
  // Pokémon V. The deterministic trace must name the actual selected holder:
  // https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  assert(EngineTestAccess::attach_fss(engine));
  assert(state.active->tool == Tool::ForestSealStone);
  assert(std::any_of(trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
    return line.find("Attached Forest Seal Stone to Regidrago VSTAR.") !=
           std::string::npos;
  }));
}

void test_fss_holds_without_alternate_v_when_powerglass_needs_active_slot() {
  using namespace sim;
  const Scenario scenario{"fss-hold-for-powerglass", DciProfile::NoDiscardControl,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(381);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 0, Tool::None};
  state.hand = {Card::ForestSealStone, Card::Powerglass};
  state.discard = {Card::Fire};

  // Forest Seal Stone can use another Pokémon V holder, while Powerglass must be on
  // its Active holder and each Pokémon may hold only one Tool. With no alternate V,
  // the legal stronger route holds the Stone and reserves the Active slot:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // https://www.pokemon.com/us/pokemon-tcg/rules
  assert(!EngineTestAccess::attach_fss(engine));
  assert(state.active->tool == Tool::None);
  assert(count(state.hand, Card::ForestSealStone) == 1);
  assert(EngineTestAccess::attach_powerglass(engine));
  assert(state.active->tool == Tool::Powerglass);
}

void test_fss_uses_active_when_powerglass_route_is_not_live() {
  using namespace sim;
  const Scenario scenario{"fss-active-without-live-powerglass", DciProfile::NoDiscardControl,
                          LockMode::None, false, 4};
  const DeckRecipe recipe = baseline_recipe();
  std::mt19937_64 rng(382);
  Engine engine(scenario, recipe, rng);
  State& state = EngineTestAccess::state(engine);
  state.turn = 2;
  state.active = Pokemon{Card::RegidragoVstar, 1, 2, 1, Tool::None};
  state.hand = {Card::ForestSealStone, Card::Powerglass};
  state.discard = {Card::Fire};

  // Powerglass cannot advance a holder that already has GGF, so the normal legal
  // Active-first Forest Seal Stone attachment remains available:
  // https://api.pokemontcg.io/v2/cards/sv6pt5-63
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  assert(EngineTestAccess::attach_fss(engine));
  assert(state.active->tool == Tool::ForestSealStone);
  assert(count(state.hand, Card::Powerglass) == 1);
}

}  // namespace

int main() {
  test_star_alchemy_uses_crispin_for_same_turn_ggf();
  test_star_alchemy_keeps_vessel_priority_without_same_turn_crispin_completion();
  test_star_alchemy_fetches_oricorio_when_crispin_blocks_burnet();
  test_k0_star_alchemy_may_take_an_unknown_any_card_fallback();
  test_k1_star_alchemy_holds_when_only_irrelevant_serena_remains();
  test_k1_star_alchemy_uses_a_live_energy_target();
  test_k1_star_alchemy_uses_a_live_payload_outlet();
  test_k1_star_alchemy_uses_a_live_vstar_target();
  test_arven_fss_fetches_raw_energy_after_supporter_use();
  test_fss_trace_names_vstar_holder();
  test_fss_holds_without_alternate_v_when_powerglass_needs_active_slot();
  test_fss_uses_active_when_powerglass_route_is_not_live();
  return 0;
}
