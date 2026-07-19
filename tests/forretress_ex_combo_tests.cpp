#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static State& state(Engine& engine) { return engine.state_; }
  static const State& state(const Engine& engine) { return engine.state_; }
  static void set_deck_seen(Engine& engine) { engine.deck_seen_ = true; }
  static bool deck_seen(const Engine& engine) { return engine.deck_seen_; }
  static bool evolve_forretress_ex(Engine& engine) {
    return engine.evolve_forretress_ex();
  }
  static bool resolve_exploding_energy(
      Engine& engine, const std::vector<std::size_t>& destinations) {
    return engine.resolve_exploding_energy(destinations);
  }
  static bool play_quick_ball_for_pineco(Engine& engine) {
    return engine.play_quick_ball_for_pineco();
  }
  static bool play_ultra_ball_for_forretress_combo(Engine& engine) {
    return engine.play_ultra_ball_for_forretress_combo();
  }
  static bool play_evolution_incense_for_forretress(Engine& engine) {
    return engine.play_evolution_incense_for_forretress();
  }
  static bool play_pokemon_communication(Engine& engine) {
    return engine.play_pokemon_communication(false);
  }
  static bool play_heavy_ball(Engine& engine) {
    return engine.play_heavy_ball();
  }
  static bool use_fss(Engine& engine) { return engine.use_fss(); }
  static bool play_steven(Engine& engine) { return engine.play_steven(); }
  static bool play_arven(Engine& engine) { return engine.play_arven(); }
  static bool play_gladion(Engine& engine) { return engine.play_gladion(); }
};

}  // namespace sim

namespace {

int count(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

int recipe_count(const sim::DeckRecipe& recipe, const sim::Card card) {
  for (const auto& [candidate, copies] : recipe) {
    if (candidate == card) return copies;
  }
  return 0;
}

sim::DeckRecipe forretress_variant_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  const auto remove_singleton = [&recipe](const sim::Card card) {
    const auto it = std::find_if(recipe.begin(), recipe.end(), [card](const auto& entry) {
      return entry.first == card;
    });
    if (it == recipe.end() || it->second != 1) {
      throw std::runtime_error("Forretress variant could not remove its test-only singleton.");
    }
    recipe.erase(it);
  };
  remove_singleton(sim::Card::ErikasInvitation);
  remove_singleton(sim::Card::Channeler);
  remove_singleton(sim::Card::Lusamine);
  remove_singleton(sim::Card::TeamYellsCheer);
  recipe.emplace_back(sim::Card::Pineco, 2);
  recipe.emplace_back(sim::Card::ForretressEx, 2);
  return recipe;
}

struct Fixture {
  sim::Scenario scenario;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  explicit Fixture(const std::string& label,
                   const sim::LockMode locks = sim::LockMode::None,
                   const std::uint64_t seed = 972,
                   const sim::DciProfile dci = sim::DciProfile::StrictJit)
      : scenario{label, dci, locks, false, 4},
        recipe(forretress_variant_recipe()),
        rng(seed),
        trace{true, {}},
        engine(scenario, recipe, rng, &trace) {}
};

void test_card_identity_and_variant_isolation() {
  const sim::DeckRecipe baseline = sim::baseline_recipe();
  const sim::DeckRecipe variant = forretress_variant_recipe();
  int baseline_total = 0;
  int variant_total = 0;
  for (const auto& [card, copies] : baseline) {
    (void)card;
    baseline_total += copies;
  }
  for (const auto& [card, copies] : variant) {
    (void)card;
    variant_total += copies;
  }

  // Pineco and Forretress ex are intentionally implemented outside the canonical
  // sample deck. The temporary 2-2 recipe exists only inside this regression:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (baseline_total != 60 || variant_total != 60 ||
      recipe_count(baseline, sim::Card::Pineco) != 0 ||
      recipe_count(baseline, sim::Card::ForretressEx) != 0 ||
      recipe_count(variant, sim::Card::Pineco) != 2 ||
      recipe_count(variant, sim::Card::ForretressEx) != 2) {
    throw std::runtime_error("The test-only 2-2 line leaked into or malformed the baseline recipe.");
  }
  if (sim::name(sim::Card::Pineco) != "Pineco" ||
      sim::name(sim::Card::ForretressEx) != "Forretress ex" ||
      !sim::is_basic(sim::Card::Pineco) ||
      sim::is_basic(sim::Card::ForretressEx) ||
      !sim::is_pokemon(sim::Card::Pineco) ||
      !sim::is_pokemon(sim::Card::ForretressEx) ||
      sim::is_rule_box_pokemon(sim::Card::Pineco) ||
      !sim::is_rule_box_pokemon(sim::Card::ForretressEx)) {
    throw std::runtime_error("Pineco or Forretress ex card classification is incorrect.");
  }
}

void test_stage_one_evolution_timing() {
  Fixture fixture("forretress-evolution-timing");
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 0}};
  state.hand = {sim::Card::ForretressEx};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::evolve_forretress_ex(fixture.engine)) {
    throw std::runtime_error("Forretress ex evolved on the first turn.");
  }

  state.turn = 2;
  state.bench.front().entered_turn = 2;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::evolve_forretress_ex(fixture.engine)) {
    throw std::runtime_error("Forretress ex evolved during Pineco's entry turn.");
  }

  state.bench.front().entered_turn = 1;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (!sim::EngineTestAccess::evolve_forretress_ex(fixture.engine) ||
      sim::EngineTestAccess::state(fixture.engine).bench.front().card !=
          sim::Card::ForretressEx) {
    throw std::runtime_error("A prior-turn Pineco did not evolve into Forretress ex.");
  }
}

void test_exploding_energy_attaches_five_and_self_knocks_out() {
  Fixture fixture("forretress-five-grass");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 1};
  state.bench = {sim::Pokemon{sim::Card::ForretressEx, 0}};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, state);

  // Exploding Energy searches up to five Basic Grass Energy, attaches them in any
  // distribution, shuffles, and then Knocks Out Forretress ex:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (!sim::EngineTestAccess::resolve_exploding_energy(
          fixture.engine, {0U, 0U, 0U, 0U, 0U})) {
    throw std::runtime_error("Exploding Energy did not resolve its five-Energy route.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->grass != 5 || after.active->fire != 1 ||
      !after.bench.empty() || count(after.deck, sim::Card::Grass) != 0 ||
      count(after.discard, sim::Card::ForretressEx) != 1 ||
      count(after.discard, sim::Card::Pineco) != 1 ||
      !sim::EngineTestAccess::deck_seen(fixture.engine)) {
    throw std::runtime_error("Exploding Energy produced incorrect attachments, zones, or K1 knowledge.");
  }
}

void test_exploding_energy_zero_selection_still_self_knocks_out() {
  Fixture fixture("forretress-zero-selection");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::ForretressEx, 0}};
  state.deck = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, state);

  // "Up to 5" permits selecting zero cards after choosing to use the Ability.
  // The deck was still searched, so the printed self-Knock-Out clause resolves:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (!sim::EngineTestAccess::resolve_exploding_energy(fixture.engine, {})) {
    throw std::runtime_error("Exploding Energy rejected its legal zero-card search.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      !after.bench.empty() || count(after.discard, sim::Card::ForretressEx) != 1 ||
      count(after.discard, sim::Card::Pineco) != 1 ||
      !sim::EngineTestAccess::deck_seen(fixture.engine)) {
    throw std::runtime_error(
        "A zero-card Exploding Energy search did not self-KO or preserve zones.");
  }
}

void test_exploding_energy_distribution_and_stack_cleanup() {
  Fixture fixture("forretress-distribution");
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {
      sim::Pokemon{sim::Card::ForretressEx, 0, 1, 1, sim::Tool::Powerglass},
      sim::Pokemon{sim::Card::RegidragoV, 0},
  };
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (!sim::EngineTestAccess::resolve_exploding_energy(
          fixture.engine, {0U, 0U, 2U, 2U, 2U})) {
    throw std::runtime_error("Exploding Energy rejected a legal split distribution.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->grass != 2 || after.bench.size() != 1U ||
      after.bench.front().grass != 3 ||
      count(after.discard, sim::Card::Grass) != 1 ||
      count(after.discard, sim::Card::Fire) != 1 ||
      count(after.discard, sim::Card::Powerglass) != 1 ||
      count(after.discard, sim::Card::ForretressEx) != 1 ||
      count(after.discard, sim::Card::Pineco) != 1) {
    throw std::runtime_error("Exploding Energy did not preserve distribution or KO stack cleanup.");
  }
}

void test_active_forretress_self_knockout_promotes_bench() {
  Fixture fixture("forretress-active-ko");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::ForretressEx, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0}};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (!sim::EngineTestAccess::resolve_exploding_energy(
          fixture.engine, {1U, 1U, 1U, 1U, 1U})) {
    throw std::runtime_error("Active Forretress ex could not resolve Exploding Energy.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      after.active->grass != 5 || !after.bench.empty()) {
    throw std::runtime_error("Active Forretress ex self-KO did not promote the powered Bench target.");
  }
}

void test_rule_box_ability_lock_blocks_exploding_energy() {
  Fixture fixture("forretress-rulebox-lock", sim::LockMode::FullRuleBoxAbility);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::ForretressEx, 0}};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::resolve_exploding_energy(fixture.engine, {0U}) ||
      sim::EngineTestAccess::state(fixture.engine).deck != state.deck ||
      sim::EngineTestAccess::state(fixture.engine).bench.size() != 1U) {
    throw std::runtime_error("Path-style Rule Box Ability lock allowed Exploding Energy.");
  }
}

void test_quick_ball_connector_finds_pineco() {
  Fixture fixture("forretress-quick-ball");
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::QuickBall, sim::Card::Dipplin};
  state.deck = {sim::Card::Pineco, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (!sim::EngineTestAccess::play_quick_ball_for_pineco(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand, sim::Card::Pineco)) {
    throw std::runtime_error("Quick Ball did not expose Pineco as a Basic connector.");
  }
}

void test_ultra_ball_connector_finds_forretress() {
  Fixture fixture("forretress-ultra-ball", sim::LockMode::None, 972,
                  sim::DciProfile::MatchupFlexJit);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 0}};
  state.hand = {sim::Card::UltraBall, sim::Card::Dipplin,
                sim::Card::ErikasInvitation};
  state.deck = {sim::Card::ForretressEx, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (!sim::EngineTestAccess::play_ultra_ball_for_forretress_combo(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::ForretressEx)) {
    throw std::runtime_error("Ultra Ball did not expose Forretress ex as a Pokémon connector.");
  }
}

void test_evolution_incense_connector_finds_forretress() {
  Fixture fixture("forretress-evolution-incense");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 0}};
  state.hand = {sim::Card::EvolutionIncense};
  state.deck = {sim::Card::ForretressEx, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (!sim::EngineTestAccess::play_evolution_incense_for_forretress(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::ForretressEx)) {
    throw std::runtime_error("Evolution Incense did not expose the Stage 1 Forretress ex connector.");
  }
}

void test_pokemon_communication_connector_finds_forretress() {
  Fixture fixture("forretress-communication");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 0}};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin};
  state.deck = {sim::Card::ForretressEx, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (!sim::EngineTestAccess::play_pokemon_communication(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::ForretressEx)) {
    throw std::runtime_error("Pokémon Communication did not expose the Forretress ex connector.");
  }
}

void test_heavy_ball_connector_recovers_prized_pineco() {
  Fixture fixture("forretress-heavy-ball");
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::HisuianHeavyBall};
  state.deck = {sim::Card::ForretressEx, sim::Card::Grass};
  state.prizes = {sim::Card::Pineco, sim::Card::Fire, sim::Card::Arven,
                  sim::Card::Crispin, sim::Card::Channeler,
                  sim::Card::Lusamine};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (!sim::EngineTestAccess::play_heavy_ball(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::Pineco)) {
    throw std::runtime_error("Hisuian Heavy Ball did not recover prized Pineco.");
  }
}

void test_forest_seal_stone_connector_finds_forretress() {
  Fixture fixture("forretress-fss");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1,
                              sim::Tool::ForestSealStone};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 0}};
  state.deck = {sim::Card::ForretressEx, sim::Card::Grass};
  state.discard = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, state);

  // Star Alchemy searches any card, so the missing Stage 1 is a legal direct
  // connector when Energy is the unresolved Regidrago axis:
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (!sim::EngineTestAccess::use_fss(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::ForretressEx)) {
    throw std::runtime_error(
        "Forest Seal Stone did not expose Forretress ex as an any-card connector.");
  }
}

void test_steven_connector_reserves_forretress() {
  Fixture fixture("forretress-steven");
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 0}};
  state.hand = {sim::Card::StevensResolve};
  state.deck = {sim::Card::ForretressEx, sim::Card::RegidragoVstar,
                sim::Card::BrilliantBlender, sim::Card::Grass,
                sim::Card::Fire, sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, state);

  // Steven's Resolve may search any three cards. A setup Pineco makes the Stage 1
  // a legal reserved Energy connector for the following turn:
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (!sim::EngineTestAccess::play_steven(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::ForretressEx) ||
      !sim::EngineTestAccess::state(fixture.engine).turn_ended) {
    throw std::runtime_error(
        "Steven's Resolve did not reserve Forretress ex through its unrestricted search.");
  }
}

void test_arven_connector_finds_evolution_incense_for_forretress() {
  Fixture fixture("forretress-arven");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 0}};
  state.hand = {sim::Card::Arven};
  state.deck = {sim::Card::EvolutionIncense, sim::Card::ForestSealStone,
                sim::Card::ForretressEx, sim::Card::Grass};
  sim::EngineTestAccess::set_state(fixture.engine, state);

  // Arven can fetch the Item connector and a Tool. Evolution Incense then exposes
  // the missing Stage 1 while Forest Seal Stone preserves a separate any-card line:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (!sim::EngineTestAccess::play_arven(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::EvolutionIncense) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::ForestSealStone)) {
    throw std::runtime_error(
        "Arven did not find the Forretress ex Item connector and live Tool route.");
  }
}

void test_gladion_connector_recovers_prized_forretress() {
  Fixture fixture("forretress-gladion");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 1};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 0}};
  state.hand = {sim::Card::Gladion};
  state.deck = {sim::Card::Grass, sim::Card::Grass};
  state.prizes = {sim::Card::ForretressEx, sim::Card::Fire,
                  sim::Card::Arven, sim::Card::Crispin,
                  sim::Card::Channeler, sim::Card::Lusamine};
  state.discard = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Gladion may exchange itself for any selected Prize card. K1 therefore
  // recovers the prized Stage 1 when it is the missing combo half:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (!sim::EngineTestAccess::play_gladion(fixture.engine) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).hand,
                sim::Card::ForretressEx) ||
      !contains(sim::EngineTestAccess::state(fixture.engine).prizes,
                sim::Card::Gladion)) {
    throw std::runtime_error(
        "Gladion did not recover a known prized Forretress ex connector.");
  }
}

void test_seeded_variant_executes_combo() {
  const sim::DeckRecipe recipe = forretress_variant_recipe();
  const sim::Scenario scenario{"forretress-seeded-variant",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  struct SeedExpectation {
    std::uint64_t seed;
    int ready_turn;
    std::string required_connector;
  };
  const std::vector<SeedExpectation> cases{
      // Seed 12 starts Pineco Active, evolves on T2, attaches four remaining Grass,
      // self-KOs, promotes Regidrago V, and reaches readiness on T2.
      // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1
      // https://api.pokemontcg.io/v2/cards/sv4pt5-2
      // Core procedure: https://www.pokemon.com/us/pokemon-tcg/rules
      // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/972
      {12, 2, "PROMOTE | rules: R-GAME-KO"},
      // Seed 2 proves Hisuian Heavy Ball recovers prized Pineco before the T2
      // Forretress ex evolution and Exploding Energy route.
      // https://api.pokemontcg.io/v2/cards/swsh10-146
      {2, 2, "exchanged Heavy Ball for Pineco"},
      // Seed 117 proves Quick Ball can supply Pineco on T2, after which the
      // prior-turn timing rule delays Forretress ex and Exploding Energy to T3.
      // https://api.pokemontcg.io/v2/cards/swsh1-179
      {117, 3, "Searched a Basic Pokémon: Pineco"},
  };

  for (const SeedExpectation& expected : cases) {
    std::mt19937_64 rng(expected.seed);
    sim::TraceLog trace{true, {}};
    sim::Engine engine(scenario, recipe, rng, &trace);
    const sim::TrialOutcome outcome = engine.run();
    const bool evolved = std::any_of(trace.lines.begin(), trace.lines.end(),
        [](const std::string& line) {
          return line.find("Pineco evolved into Forretress ex") != std::string::npos;
        });
    const bool exploded = std::any_of(trace.lines.begin(), trace.lines.end(),
        [](const std::string& line) {
          return line.find("Exploding Energy attached") != std::string::npos;
        });
    const bool connector = std::any_of(trace.lines.begin(), trace.lines.end(),
        [&expected](const std::string& line) {
          return line.find(expected.required_connector) != std::string::npos;
        });
    if (!evolved || !exploded || !connector ||
        outcome.first_ready_turn != expected.ready_turn) {
      throw std::runtime_error(
          "A deterministic 2-2 variant seed did not execute its source-bound combo route.");
    }
  }
}

}  // namespace

int main() {
  test_card_identity_and_variant_isolation();
  test_stage_one_evolution_timing();
  test_exploding_energy_attaches_five_and_self_knocks_out();
  test_exploding_energy_zero_selection_still_self_knocks_out();
  test_exploding_energy_distribution_and_stack_cleanup();
  test_active_forretress_self_knockout_promotes_bench();
  test_rule_box_ability_lock_blocks_exploding_energy();
  test_quick_ball_connector_finds_pineco();
  test_ultra_ball_connector_finds_forretress();
  test_evolution_incense_connector_finds_forretress();
  test_pokemon_communication_connector_finds_forretress();
  test_heavy_ball_connector_recovers_prized_pineco();
  test_forest_seal_stone_connector_finds_forretress();
  test_steven_connector_reserves_forretress();
  test_arven_connector_finds_evolution_incense_for_forretress();
  test_gladion_connector_recovers_prized_forretress();
  test_seeded_variant_executes_combo();
}
