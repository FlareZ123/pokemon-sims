#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static State& state(Engine& engine) { return engine.state_; }
  static void choose_opening_active(Engine& engine) { engine.choose_opening_active(); }
  static bool bench_from_hand(Engine& engine, Card card, bool resolve_entry) {
    return engine.bench_from_hand(card, resolve_entry);
  }
  static bool play_mysterious_treasure(Engine& engine, bool permit_payload) {
    return engine.play_mysterious_treasure(permit_payload);
  }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static void record_ready(Engine& engine) { engine.record_ready(); }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static const TrialOutcome& outcome(const Engine& engine) { return engine.outcome_; }
};

}  // namespace sim

namespace {

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool benched(const sim::State& state, const sim::Card card) {
  return std::any_of(state.bench.begin(), state.bench.end(),
                     [card](const sim::Pokemon& pokemon) {
                       return pokemon.card == card;
                     });
}

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void test_mawile_starts_active_so_oricorio_keeps_vital_dance() {
  // Vital Dance triggers only when Oricorio is played from hand onto the Bench
  // during the turn. Mawile-GX is a Basic Pokémon and can legally take the opening
  // Active Spot while Oricorio remains in hand:
  // https://api.pokemontcg.io/v2/cards/sm2-55
  // https://api.pokemontcg.io/v2/cards/sm11-141
  // https://www.pokemon.com/us/pokemon-tcg/rules
  sim::Scenario scenario{"oricorio-opening-preservation", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{306};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.hand = {sim::Card::Oricorio, sim::Card::MawileGX};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::choose_opening_active(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.active && after.active->card == sim::Card::MawileGX,
         "Mawile-GX should take the opening Active Spot when it is paired with Oricorio");
  expect(contains(after.hand, sim::Card::Oricorio),
         "Oricorio should remain in hand so Vital Dance can trigger when it is Benched during the turn");
}

void test_oricorio_selects_a_second_legal_energy_for_follow_up_search() {
  // Vital Dance may search up to 2 Basic Energy cards: https://api.pokemontcg.io/v2/cards/sm2-55
  // Mysterious Treasure requires a hand-discard cost: https://api.pokemontcg.io/v2/cards/sm6-113
  sim::Scenario scenario{"oricorio-full-energy-search", sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1, sim::Tool::None};
  state.hand = {sim::Card::Oricorio, sim::Card::Fire, sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Fire, sim::Card::Grass};

  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{97};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));

  expect(sim::EngineTestAccess::bench_from_hand(engine, sim::Card::Oricorio, true),
         "Vital Dance should resolve when Oricorio is legally Benched from hand.");
  const sim::State& after_vital_dance = sim::EngineTestAccess::state(engine);
  expect(count(after_vital_dance.hand, sim::Card::Fire) == 2,
         "Vital Dance should keep the second legal Fire Energy after finding the needed Grass Energy.");
  expect(contains(after_vital_dance.hand, sim::Card::Grass),
         "Vital Dance should still find the Grass Energy that advances GGF.");

  expect(sim::EngineTestAccess::play_mysterious_treasure(engine, false),
         "The second Fire Energy should make Mysterious Treasure's discard cost payable.");
  const sim::State& after_treasure = sim::EngineTestAccess::state(engine);
  expect(contains(after_treasure.hand, sim::Card::RegidragoVstar),
         "Mysterious Treasure should search Regidrago VSTAR after the legal Fire discard.");
}

void test_star_alchemy_preserves_burnet_with_unusable_held_payload() {
  sim::Scenario scenario{"fss-oricorio-stranded-held-payload",
                         sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{621};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::MegaDragonite};
  state.deck = {sim::Card::Crispin, sim::Card::Oricorio, sim::Card::Fire,
                sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // A Dragon in hand cannot supply Apex Dragon without a legal discard effect.
  // Star Alchemy must search Oricorio so Vital Dance and the manual attachment solve
  // Fire while Professor Burnet keeps the Supporter slot and discards Dragapult:
  // Forest Seal Stone https://api.pokemontcg.io/v2/cards/swsh12-156
  // Oricorio https://api.pokemontcg.io/v2/cards/sm2-55
  // Crispin https://api.pokemontcg.io/v2/cards/sv7-133
  // Professor Burnet https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Apex Dragon https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core turn limits https://www.pokemon.com/us/pokemon-tcg/rules
  // Parent route https://github.com/FlareZ123/pokemon-sims/issues/187
  sim::EngineTestAccess::run_turn(engine);
  sim::EngineTestAccess::record_ready(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(after.vstar_power_used,
         "The stranded-payload route should use Star Alchemy.");
  expect(benched(after, sim::Card::Oricorio),
         "Star Alchemy should fetch and Bench Oricorio.");
  expect(count(after.discard, sim::Card::Crispin) == 0,
         "Crispin must remain unused so Burnet keeps the Supporter play.");
  expect(count(after.discard, sim::Card::ProfessorBurnet) == 1,
         "Professor Burnet should resolve after Vital Dance.");
  expect(count(after.discard, sim::Card::Dragapult) == 1,
         "Professor Burnet should discard the deck payload.");
  expect(count(after.hand, sim::Card::MegaDragonite) == 1,
         "The unusable held payload should remain in hand.");
  expect(after.active && after.active->fire == 1,
         "Vital Dance and the manual attachment should complete Fire.");
  expect(sim::EngineTestAccess::outcome(engine).first_ready_turn == 2,
         "The corrected route should reach strict-JIT readiness on turn 2.");
}

void test_star_alchemy_holds_with_live_held_vessel_payload_item() {
  sim::Scenario scenario{"fss-hold-live-held-vessel-payload-item",
                         sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{622};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::MegaDragonite,
                sim::Card::EarthenVessel};
  state.deck = {sim::Card::Crispin, sim::Card::Oricorio, sim::Card::Fire,
                sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Earthen Vessel can discard the held Dragon, search the known Fire target, and
  // leave the manual attachment unused until Fire reaches the Active Regidrago VSTAR.
  // The searched Crispin would remain unused, so the complete K1 route preserves the
  // once-per-game Star Alchemy resource:
  // Earthen Vessel https://api.pokemontcg.io/v2/cards/sv4-163
  // Crispin https://api.pokemontcg.io/v2/cards/sv7-133
  // Forest Seal Stone https://api.pokemontcg.io/v2/cards/swsh12-156
  // Apex Dragon https://api.pokemontcg.io/v2/cards/swsh12-136
  // Turn procedure https://www.pokemon.com/us/pokemon-tcg/rules
  // Route policy https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug https://github.com/FlareZ123/pokemon-sims/issues/990
  expect(!sim::EngineTestAccess::use_fss(engine),
         "Star Alchemy should remain unused with a complete held Vessel route.");
  sim::EngineTestAccess::run_turn(engine);
  sim::EngineTestAccess::record_ready(engine);

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(!after.vstar_power_used,
         "The complete Vessel route should preserve Star Alchemy.");
  expect(count(after.deck, sim::Card::Crispin) == 1,
         "Unused Crispin should remain in deck.");
  expect(count(after.deck, sim::Card::Oricorio) == 1,
         "Oricorio should remain in deck when Vessel completes Energy directly.");
  expect(count(after.discard, sim::Card::MegaDragonite) == 1,
         "Earthen Vessel should discard the held Dragon payload.");
  expect(after.active && after.active->fire == 1,
         "The searched Fire Energy should be manually attached.");
  expect(sim::EngineTestAccess::outcome(engine).first_ready_turn == 2,
         "The held Vessel route should reach strict-JIT readiness on turn two.");
}

void test_star_alchemy_keeps_crispin_with_payable_ultra_ball_payload_item() {
  sim::Scenario scenario{"fss-crispin-live-ultra-payload-item",
                         sim::DciProfile::StrictJit, sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  std::mt19937_64 rng{623};
  sim::Engine engine(scenario, recipe, rng);

  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::ForestSealStone}};
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::MegaDragonite,
                sim::Card::UltraBall, sim::Card::Dipplin};
  state.deck = {sim::Card::Crispin, sim::Card::Oricorio, sim::Card::Fire,
                sim::Card::Dragapult};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(engine);

  // Ultra Ball can discard the held Dragon plus Dipplin and has a known Pokémon
  // target. That live Item route means Star Alchemy may select Crispin:
  // Ultra Ball https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // Crispin https://api.pokemontcg.io/v2/cards/sv7-133
  // Forest Seal Stone https://api.pokemontcg.io/v2/cards/swsh12-156
  // Apex Dragon https://api.pokemontcg.io/v2/cards/swsh12-136
  expect(sim::EngineTestAccess::use_fss(engine),
         "Star Alchemy should resolve with a payable Ultra Ball payload outlet.");
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(count(after.hand, sim::Card::Crispin) == 1,
         "A payable Ultra Ball should preserve the Crispin target.");
  expect(count(after.deck, sim::Card::Oricorio) == 1,
         "Oricorio should remain in deck when Ultra Ball handles the payload.");
}

}  // namespace

int main() {
  try {
    test_mawile_starts_active_so_oricorio_keeps_vital_dance();
    test_oricorio_selects_a_second_legal_energy_for_follow_up_search();
    test_star_alchemy_preserves_burnet_with_unusable_held_payload();
    test_star_alchemy_holds_with_live_held_vessel_payload_item();
    test_star_alchemy_keeps_crispin_with_payable_ultra_ball_payload_item();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
