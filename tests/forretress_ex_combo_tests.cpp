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
  static bool advance_forretress_combo(Engine& engine) {
    return engine.advance_forretress_combo();
  }
  static Card choose_supporter_after_search_started(const Engine& engine) {
    return engine.choose_supporter_after_search_started();
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

sim::DeckRecipe dawn_forest_variant_recipe() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  const auto remove_singleton = [&recipe](const sim::Card card) {
    const auto it = std::find_if(recipe.begin(), recipe.end(), [card](const auto& entry) {
      return entry.first == card;
    });
    if (it == recipe.end() || it->second != 1) {
      throw std::runtime_error("Dawn and Forest variant could not remove its test-only singleton.");
    }
    recipe.erase(it);
  };
  for (const sim::Card card : {sim::Card::LatiasEx, sim::Card::Oricorio,
                               sim::Card::Dipplin, sim::Card::Serena,
                               sim::Card::TateLiza, sim::Card::Channeler,
                               sim::Card::Lusamine, sim::Card::TeamYellsCheer}) {
    remove_singleton(card);
  }
  recipe.emplace_back(sim::Card::Pineco, 2);
  recipe.emplace_back(sim::Card::ForretressEx, 2);
  recipe.emplace_back(sim::Card::Dawn, 2);
  recipe.emplace_back(sim::Card::ForestOfVitality, 2);
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

void test_dawn_forest_identity_and_variant_isolation() {
  const sim::DeckRecipe baseline = sim::baseline_recipe();
  const sim::DeckRecipe variant = dawn_forest_variant_recipe();
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

  // Dawn and Forest of Vitality remain in the same intentional test-only support
  // category as Pineco and Forretress ex. They do not enter baseline_recipe(), CLI
  // matrix rows, or committed aggregate results:
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Pineco and Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/issues/1096
  if (baseline_total != 60 || variant_total != 60 ||
      recipe_count(baseline, sim::Card::Dawn) != 0 ||
      recipe_count(baseline, sim::Card::ForestOfVitality) != 0 ||
      recipe_count(variant, sim::Card::Pineco) != 2 ||
      recipe_count(variant, sim::Card::ForretressEx) != 2 ||
      recipe_count(variant, sim::Card::Dawn) != 2 ||
      recipe_count(variant, sim::Card::ForestOfVitality) != 2 ||
      sim::name(sim::Card::Dawn) != "Dawn" ||
      sim::name(sim::Card::ForestOfVitality) != "Forest of Vitality" ||
      !sim::is_supporter(sim::Card::Dawn)) {
    throw std::runtime_error("Dawn or Forest of Vitality leaked into or malformed the baseline recipe.");
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

void test_forest_evolution_and_stadium_procedure() {
  Fixture fixture("forest-evolution-procedure");
  sim::State state;
  state.turn = 1;
  state.stadium = sim::Stadium::ForestOfVitality;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 1}};
  state.hand = {sim::Card::ForretressEx};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::evolve_forretress_ex(fixture.engine)) {
    throw std::runtime_error("Forest of Vitality bypassed the first-turn evolution prohibition.");
  }

  state.turn = 2;
  state.stadium = sim::Stadium::None;
  state.bench.front().entered_turn = 2;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::evolve_forretress_ex(fixture.engine)) {
    throw std::runtime_error("Pineco evolved on its entry turn without Forest of Vitality.");
  }

  state.stadium = sim::Stadium::ForestOfVitality;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (!sim::EngineTestAccess::evolve_forretress_ex(fixture.engine) ||
      sim::EngineTestAccess::state(fixture.engine).bench.front().card !=
          sim::Card::ForretressEx) {
    throw std::runtime_error("Forest of Vitality did not enable the legal turn-two same-turn evolution.");
  }

  // A player may play only one Stadium during a turn, and a Stadium with the same
  // name as the Stadium already in play cannot be played:
  // https://api.pokemontcg.io/v2/cards/me1-117
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1096
  state = {};
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::ForestOfVitality};
  state.stadium = sim::Stadium::ForestOfVitality;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::advance_forretress_combo(fixture.engine) ||
      count(sim::EngineTestAccess::state(fixture.engine).hand,
            sim::Card::ForestOfVitality) != 1) {
    throw std::runtime_error("A same-name Forest of Vitality was illegally replayed.");
  }

  state.stadium = sim::Stadium::None;
  state.stadium_used = true;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (sim::EngineTestAccess::advance_forretress_combo(fixture.engine) ||
      count(sim::EngineTestAccess::state(fixture.engine).hand,
            sim::Card::ForestOfVitality) != 1) {
    throw std::runtime_error("A second Stadium was illegally played during the same turn.");
  }

  state.stadium_used = false;
  state.stadium = sim::Stadium::ChaoticSwell;
  sim::EngineTestAccess::set_state(fixture.engine, state);
  if (!sim::EngineTestAccess::advance_forretress_combo(fixture.engine)) {
    throw std::runtime_error("Forest of Vitality did not attempt to enter play into Chaotic Swell.");
  }
  const sim::State& swell_after = sim::EngineTestAccess::state(fixture.engine);
  if (swell_after.stadium != sim::Stadium::None || !swell_after.stadium_used ||
      count(swell_after.discard, sim::Card::ChaoticSwell) != 1 ||
      count(swell_after.discard, sim::Card::ForestOfVitality) != 1) {
    throw std::runtime_error("Chaotic Swell did not discard itself and the incoming Forest.");
  }

  Fixture locked("forest-removes-rulebox-lock", sim::LockMode::FullRuleBoxAbility);
  state = {};
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 1};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 2}};
  state.hand = {sim::Card::ForestOfVitality, sim::Card::ForretressEx};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass};
  sim::EngineTestAccess::set_state(locked.engine, state);
  if (!sim::EngineTestAccess::advance_forretress_combo(locked.engine)) {
    throw std::runtime_error("Forest did not replace the modeled Path and restore Exploding Energy.");
  }
  const sim::State& unlocked = sim::EngineTestAccess::state(locked.engine);
  // Forest removes the modeled lock, then Exploding Energy selects only the two
  // Grass needed beside held Fire and leaves the other three in deck:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  if (!unlocked.path_lock_removed || unlocked.stadium != sim::Stadium::ForestOfVitality ||
      !unlocked.active || unlocked.active->grass != 2 || unlocked.active->fire != 1 ||
      count(unlocked.deck, sim::Card::Grass) != 3 ||
      count(unlocked.discard, sim::Card::ForretressEx) != 1) {
    throw std::runtime_error("Forest replacement did not unlock the minimal Exploding Energy route.");
  }
}

void test_dawn_search_and_immediate_forest_combo() {
  const sim::Scenario scenario{"dawn-forest-exact-state",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = dawn_forest_variant_recipe();
  std::mt19937_64 rng(1096);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 1};
  state.hand = {sim::Card::Dawn, sim::Card::ForestOfVitality};
  state.deck = {sim::Card::Pineco, sim::Card::ForretressEx,
                sim::Card::MegaDragonite, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Grass};
  sim::EngineTestAccess::set_state(engine, state);

  // Dawn independently searches one Basic, one Stage 1, and one Stage 2. Forest
  // then permits the searched Grass line to evolve on Pineco's entry turn after
  // the first turn, and Exploding Energy resolves under its printed self-KO text:
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Pineco and Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1096
  if (!sim::EngineTestAccess::advance_forretress_combo(engine)) {
    throw std::runtime_error("The exact Dawn plus Forest route made no progress.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  // Dawn and Forest still complete the route while Exploding Energy selects the
  // two missing Grass and preserves the other three for later attackers:
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1329
  if (!after.supporter_used || after.stadium != sim::Stadium::ForestOfVitality ||
      !after.active || after.active->grass != 2 || after.active->fire != 1 ||
      count(after.deck, sim::Card::Grass) != 3 ||
      !contains(after.hand, sim::Card::MegaDragonite) ||
      count(after.discard, sim::Card::Dawn) != 1 ||
      count(after.discard, sim::Card::Pineco) != 1 ||
      count(after.discard, sim::Card::ForretressEx) != 1) {
    throw std::runtime_error("Dawn, Forest, or minimal Exploding Energy produced the wrong exact state.");
  }
  const bool dawn_trace = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("Dawn searched and revealed") != std::string::npos;
      });
  const bool forest_trace = std::any_of(trace.lines.begin(), trace.lines.end(),
      [](const std::string& line) {
        return line.find("newly played Pineco") != std::string::npos;
      });
  if (!dawn_trace || !forest_trace) {
    throw std::runtime_error("The exact Dawn and Forest route lacked readable proof events.");
  }

  // Hidden-information search categories may independently fail. Holding the Stage
  // 1 and having no Stage 2 in deck does not invalidate Dawn's legal Basic search:
  // https://api.pokemontcg.io/v2/cards/me2-87
  sim::Engine partial_engine(scenario, recipe, rng, &trace);
  state = {};
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 0, 1};
  state.hand = {sim::Card::Dawn, sim::Card::ForestOfVitality,
                sim::Card::ForretressEx};
  state.deck = {sim::Card::Pineco, sim::Card::Grass};
  sim::EngineTestAccess::set_state(partial_engine, state);
  if (!sim::EngineTestAccess::advance_forretress_combo(partial_engine) ||
      count(sim::EngineTestAccess::state(partial_engine).discard,
            sim::Card::Dawn) != 1) {
    throw std::runtime_error("Dawn incorrectly required all three hidden search categories.");
  }
}

void test_wonder_tag_prefers_live_dawn_forest_route() {
  const sim::Scenario scenario{"wonder-tag-dawn-forest",
                               sim::DciProfile::StrictJit,
                               sim::LockMode::None, false, 4};
  const sim::DeckRecipe recipe = dawn_forest_variant_recipe();
  std::mt19937_64 rng(1096);
  sim::Engine engine(scenario, recipe, rng);
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.hand = {sim::Card::ForestOfVitality};
  state.deck = {sim::Card::Dawn, sim::Card::Pineco,
                sim::Card::ForretressEx, sim::Card::MegaDragonite,
                sim::Card::Crispin, sim::Card::Grass, sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, state);
  if (sim::EngineTestAccess::choose_supporter_after_search_started(engine) !=
      sim::Card::Dawn) {
    throw std::runtime_error("Wonder Tag did not select the live Dawn plus Forest compression route.");
  }

  state.hand.clear();
  sim::Engine control(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(control, state);
  if (sim::EngineTestAccess::choose_supporter_after_search_started(control) ==
      sim::Card::Dawn) {
    throw std::runtime_error("Wonder Tag selected Dawn without a live evolution-timing route.");
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

void test_seeded_dawn_forest_routes() {
  const sim::DeckRecipe recipe = dawn_forest_variant_recipe();
  struct SeedCase {
    bool going_first;
    std::uint64_t seed;
  };
  for (const SeedCase seed_case : {SeedCase{true, 110}, SeedCase{false, 29}}) {
    const sim::Scenario scenario{
        seed_case.going_first ? "dawn-forest/go-first" : "dawn-forest/go-second",
        sim::DciProfile::StrictJit, sim::LockMode::None,
        seed_case.going_first, 5};
    std::mt19937_64 rng(seed_case.seed);
    sim::TraceLog trace{true, {}};
    sim::Engine engine(scenario, recipe, rng, &trace);
    const sim::TrialOutcome outcome = engine.run();
    const auto has = [&trace](const std::string& needle) {
      return std::any_of(trace.lines.begin(), trace.lines.end(),
                         [&needle](const std::string& line) {
                           return line.find(needle) != std::string::npos;
                         });
    };

    // These fixed seeds exercise the complete Wonder Tag or held-Dawn search,
    // Forest-enabled entry-turn evolution, Exploding Energy, and a T2 ready state:
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Pineco and Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1
    // https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://github.com/FlareZ123/pokemon-sims/issues/1096
    if (!has("Dawn searched and revealed") ||
        !has("Forest of Vitality allowed the newly played Pineco") ||
        !has("Exploding Energy attached") || outcome.first_ready_turn != 2) {
      throw std::runtime_error("A fixed Dawn and Forest seed lost its complete T2 proof route.");
    }
  }
}

}  // namespace

int main() {
  test_card_identity_and_variant_isolation();
  test_dawn_forest_identity_and_variant_isolation();
  test_stage_one_evolution_timing();
  test_forest_evolution_and_stadium_procedure();
  test_dawn_search_and_immediate_forest_combo();
  test_wonder_tag_prefers_live_dawn_forest_route();
  test_exploding_energy_attaches_distributes_and_self_knocks_out();
  test_exploding_energy_zero_selection_and_lock_boundary();
  test_basic_and_evolution_connectors();
  test_seeded_variant_executes_combo();
  test_seeded_dawn_forest_routes();
}
