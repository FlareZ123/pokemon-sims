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

  // Pineco and Forretress ex stay outside the canonical sample deck. The temporary
  // 2-2 recipe exists only in this regression and cannot enter aggregate output:
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

void test_exploding_energy_attaches_distributes_and_self_knocks_out() {
  Fixture fixture("forretress-distribution");
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 1};
  state.bench = {
      sim::Pokemon{sim::Card::ForretressEx, 0, 1, 1, sim::Tool::Powerglass},
      sim::Pokemon{sim::Card::RegidragoV, 0},
  };
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, state);

  // Exploding Energy may attach up to five Basic Grass Energy in any distribution,
  // shuffles the deck, then Knocks Out Forretress ex and its full Evolution stack:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (!sim::EngineTestAccess::resolve_exploding_energy(
          fixture.engine, {0U, 0U, 2U, 2U, 2U})) {
    throw std::runtime_error("Exploding Energy rejected a legal five-Energy distribution.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->grass != 2 || after.active->fire != 1 ||
      after.bench.size() != 1U || after.bench.front().grass != 3 ||
      count(after.deck, sim::Card::Grass) != 0 ||
      count(after.discard, sim::Card::Grass) != 1 ||
      count(after.discard, sim::Card::Fire) != 1 ||
      count(after.discard, sim::Card::Powerglass) != 1 ||
      count(after.discard, sim::Card::ForretressEx) != 1 ||
      count(after.discard, sim::Card::Pineco) != 1 ||
      !sim::EngineTestAccess::deck_seen(fixture.engine)) {
    throw std::runtime_error("Exploding Energy produced incorrect attachments, zones, or K1 knowledge.");
  }
}

void test_exploding_energy_zero_selection_and_lock_boundary() {
  Fixture fixture("forretress-zero-selection");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::ForretressEx, 0}};
  state.deck = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(fixture.engine, state);

  // "Up to 5" permits selecting zero cards. The deck was still searched, so the
  // printed self-Knock-Out clause resolves:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (!sim::EngineTestAccess::resolve_exploding_energy(fixture.engine, {})) {
    throw std::runtime_error("Exploding Energy rejected its legal zero-card search.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.active || after.active->card != sim::Card::RegidragoV ||
      !after.bench.empty() || count(after.discard, sim::Card::ForretressEx) != 1 ||
      count(after.discard, sim::Card::Pineco) != 1) {
    throw std::runtime_error("A zero-card Exploding Energy search did not self-KO correctly.");
  }

  Fixture locked("forretress-rulebox-lock", sim::LockMode::FullRuleBoxAbility);
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(locked.engine, state);
  if (sim::EngineTestAccess::resolve_exploding_energy(locked.engine, {0U})) {
    throw std::runtime_error("Path-style Rule Box Ability lock allowed Exploding Energy.");
  }
}

void test_basic_and_evolution_connectors() {
  Fixture quick("forretress-quick-ball");
  sim::State quick_state;
  quick_state.turn = 1;
  quick_state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  quick_state.hand = {sim::Card::QuickBall, sim::Card::Dipplin};
  quick_state.deck = {sim::Card::Pineco, sim::Card::Grass};
  sim::EngineTestAccess::set_state(quick.engine, quick_state);
  // Quick Ball searches a Basic Pokémon after its mandatory discard:
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (!sim::EngineTestAccess::play_quick_ball_for_pineco(quick.engine) ||
      !contains(sim::EngineTestAccess::state(quick.engine).hand, sim::Card::Pineco)) {
    throw std::runtime_error("Quick Ball did not expose Pineco as a Basic connector.");
  }

  Fixture incense("forretress-evolution-incense");
  sim::State incense_state;
  incense_state.turn = 2;
  incense_state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  incense_state.bench = {sim::Pokemon{sim::Card::Pineco, 0}};
  incense_state.hand = {sim::Card::EvolutionIncense};
  incense_state.deck = {sim::Card::ForretressEx, sim::Card::Grass};
  sim::EngineTestAccess::set_state(incense.engine, incense_state);
  // Evolution Incense searches an Evolution Pokémon, which includes Forretress ex:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (!sim::EngineTestAccess::play_evolution_incense_for_forretress(incense.engine) ||
      !contains(sim::EngineTestAccess::state(incense.engine).hand,
                sim::Card::ForretressEx)) {
    throw std::runtime_error("Evolution Incense did not expose Forretress ex.");
  }

  Fixture ultra("forretress-ultra-ball", sim::LockMode::None, 972,
                sim::DciProfile::MatchupFlexJit);
  sim::State ultra_state;
  ultra_state.turn = 2;
  ultra_state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  ultra_state.bench = {sim::Pokemon{sim::Card::Pineco, 0}};
  ultra_state.hand = {sim::Card::UltraBall, sim::Card::Dipplin,
                      sim::Card::ErikasInvitation};
  ultra_state.deck = {sim::Card::ForretressEx, sim::Card::Grass};
  sim::EngineTestAccess::set_state(ultra.engine, ultra_state);
  // Ultra Ball searches any Pokémon after two legal discards:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (!sim::EngineTestAccess::play_ultra_ball_for_forretress_combo(ultra.engine) ||
      !contains(sim::EngineTestAccess::state(ultra.engine).hand,
                sim::Card::ForretressEx)) {
    throw std::runtime_error("Ultra Ball did not expose Forretress ex.");
  }
}

void test_seeded_variant_executes_combo() {
  const sim::DeckRecipe recipe = forretress_variant_recipe();
  const sim::Scenario scenario{"forretress-seeded-variant",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const std::vector<std::uint64_t> seeds{12, 2, 117};
  int complete_routes = 0;
  for (const std::uint64_t seed : seeds) {
    std::mt19937_64 rng(seed);
    sim::TraceLog trace{true, {}};
    sim::Engine engine(scenario, recipe, rng, &trace);
    (void)engine.run();
    const bool evolved = std::any_of(trace.lines.begin(), trace.lines.end(),
        [](const std::string& line) {
          return line.find("Pineco evolved into Forretress ex") != std::string::npos;
        });
    const bool exploded = std::any_of(trace.lines.begin(), trace.lines.end(),
        [](const std::string& line) {
          return line.find("Exploding Energy attached") != std::string::npos;
        });
    if (evolved && exploded) ++complete_routes;
  }

  // Three deterministic 2-2 simulations exercise setup and search variance. At
  // least one must execute the complete legal evolution and Ability route:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/972
  if (complete_routes == 0) {
    throw std::runtime_error("No deterministic 2-2 variant seed executed the combo.");
  }
}

}  // namespace

int main() {
  test_card_identity_and_variant_isolation();
  test_stage_one_evolution_timing();
  test_exploding_energy_attaches_distributes_and_self_knocks_out();
  test_exploding_energy_zero_selection_and_lock_boundary();
  test_basic_and_evolution_connectors();
  test_seeded_variant_executes_combo();
}
