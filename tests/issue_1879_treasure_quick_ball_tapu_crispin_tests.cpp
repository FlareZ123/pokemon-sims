#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = true,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static std::optional<std::array<Card, 2>> plan(const Engine& engine) {
    return engine.issue_1879_treasure_quick_ball_plan();
  }
  static bool play(Engine& engine) {
    return engine.complete_issue_1879_treasure_quick_ball_tapu_crispin_route();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool payload_ready(const Engine& engine) { return engine.payload_ready(); }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool contains(const std::vector<sim::Card>& cards, const sim::Card card) {
  return std::find(cards.begin(), cards.end(), card) != cards.end();
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1879-treasure-quick-ball-tapu-crispin",
                       sim::DciProfile::StrictJit, lock, true, 4};
}

sim::State base_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::QuickBall,
                sim::Card::EarthenVessel};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::RegidragoV};
  state.prizes = {sim::Card::BrilliantBlender, sim::Card::RegidragoVstar,
                  sim::Card::Gladion, sim::Card::Serena,
                  sim::Card::PathToPeak, sim::Card::FieldBlower};
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  explicit Fixture(sim::Scenario selected = scenario(),
                   const std::uint64_t seed = 1879)
      : scenario_value(std::move(selected)),
        recipe(sim::baseline_recipe()),
        rng(seed),
        trace{true, {}},
        engine(scenario_value, recipe, rng, &trace) {}
};

void complete_two_connector_costs_and_energy_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, base_state());

  const auto plan = sim::EngineTestAccess::plan(fixture.engine);
  expect(plan.has_value(), "The complete issue-1879 route was not recognized");
  expect((*plan)[0] == sim::Card::EarthenVessel,
         "The route did not preserve Quick Ball while choosing the first cost");
  expect(sim::is_payload((*plan)[1]),
         "Mysterious Treasure did not select a permitted Dragon payload");

  // Each search Item pays its own discard cost. Treasure spends route-replaced
  // Vessel and searches a Dragon; Quick Ball spends that Dragon and searches Tapu.
  // Wonder Tag finds Crispin, whose attachment plus the unused manual attachment
  // completes GGF.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, DCI/JIT, supporter contention, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1879
  expect(sim::EngineTestAccess::play(fixture.engine),
         "The complete issue-1879 route did not resolve");

  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(state.active.has_value() && state.active->grass >= 2 &&
             state.active->fire >= 1,
         "Crispin plus the manual attachment did not complete GGF");
  expect(sim::EngineTestAccess::payload_ready(fixture.engine),
         "Quick Ball did not establish the current-turn payload");
  expect(state.supporter_used && state.manual_energy_used,
         "The route did not consume the exact Supporter and attachment actions");
  expect(contains(state.discard, sim::Card::MysteriousTreasure) &&
             contains(state.discard, sim::Card::QuickBall) &&
             contains(state.discard, sim::Card::EarthenVessel) &&
             contains(state.discard, sim::Card::Crispin),
         "The route did not preserve all printed costs and played cards");
  expect(std::any_of(state.bench.begin(), state.bench.end(),
                     [](const sim::Pokemon& pokemon) {
                       return pokemon.card == sim::Card::TapuLeleGX;
                     }),
         "Tapu Lele-GX was not Benched for Wonder Tag");
}

void knowledge_provenance_boundaries() {
  // A deck search and a full Prize inspection both establish K1. True K0 does not.
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  Fixture deck_k1;
  sim::EngineTestAccess::set_state(deck_k1.engine, base_state(), true, false);
  expect(sim::EngineTestAccess::plan(deck_k1.engine).has_value(),
         "Deck-search K1 did not admit the public route");

  Fixture prize_k1;
  sim::EngineTestAccess::set_state(prize_k1.engine, base_state(), false, true);
  expect(sim::EngineTestAccess::plan(prize_k1.engine).has_value(),
         "Prize-inspection K1 did not admit the public route");
  expect(sim::EngineTestAccess::play(prize_k1.engine),
         "Prize-inspection K1 did not complete the public route");

  Fixture k0;
  sim::EngineTestAccess::set_state(k0.engine, base_state(), false, false);
  expect(!sim::EngineTestAccess::plan(k0.engine).has_value(),
         "True K0 read known search targets");
}

void payload_choice_ignores_hidden_deck_order() {
  Fixture fixture;
  sim::State state = base_state();
  state.deck = {sim::Card::Dragapult, sim::Card::Fire,
                sim::Card::TapuLeleGX, sim::Card::Grass,
                sim::Card::Crispin, sim::Card::MegaDragonite,
                sim::Card::RegidragoV};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  const auto plan = sim::EngineTestAccess::plan(fixture.engine);
  expect(plan.has_value() && (*plan)[1] == sim::Card::MegaDragonite,
         "The route selected a payload from hidden deck order");
}

void crispin_energy_boundaries() {
  {
    Fixture fixture;
    sim::State state = base_state();
    state.active->grass = 2;
    state.manual_energy_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(sim::EngineTestAccess::plan(fixture.engine).has_value(),
           "The route rejected Crispin's direct final-Fire attachment");
    expect(sim::EngineTestAccess::play(fixture.engine),
           "Crispin did not complete the sole missing Energy");
  }
  {
    Fixture available;
    sim::State state = base_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                                 sim::Card::Grass),
                     state.deck.end());
    state.active->grass = 2;
    sim::EngineTestAccess::set_state(available.engine, state);
    expect(sim::EngineTestAccess::plan(available.engine).has_value(),
           "The route rejected a legal one-type Crispin line");
    expect(sim::EngineTestAccess::play(available.engine),
           "The one-type Crispin route did not use the manual attachment");

    Fixture spent;
    state.manual_energy_used = true;
    sim::EngineTestAccess::set_state(spent.engine, std::move(state));
    expect(!sim::EngineTestAccess::plan(spent.engine).has_value(),
           "The route invented an attachment after the manual action was spent");
  }
}

void public_resource_boundaries_are_enforced() {
  {
    Fixture fixture{scenario(sim::LockMode::FullItem)};
    sim::EngineTestAccess::set_state(fixture.engine, base_state());
    expect(!sim::EngineTestAccess::plan(fixture.engine).has_value(),
           "The route played search Items through full Item lock");
  }
  {
    Fixture fixture;
    sim::State state = base_state();
    state.supporter_used = true;
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::plan(fixture.engine).has_value(),
           "The route reused the Supporter action");
  }
  {
    Fixture fixture;
    sim::State state = base_state();
    for (int i = 0; i < 5; ++i) {
      state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                                        sim::Tool::None});
    }
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::plan(fixture.engine).has_value(),
           "The route Benched Tapu Lele-GX onto a full Bench");
  }
  {
    Fixture fixture;
    sim::State state = base_state();
    state.hand = {sim::Card::MysteriousTreasure, sim::Card::QuickBall};
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::plan(fixture.engine).has_value(),
           "The route invented Treasure's first discard cost");
  }
  for (const sim::Card missing : {sim::Card::TapuLeleGX,
                                  sim::Card::Crispin}) {
    Fixture fixture;
    sim::State state = base_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), missing),
                     state.deck.end());
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    expect(!sim::EngineTestAccess::plan(fixture.engine).has_value(),
           "The route invented a missing Tapu or Crispin target");
  }
}

sim::TrialOutcome run_seed(const std::string& label, const std::uint64_t seed,
                           sim::TraceLog& trace) {
  const auto selected = sim::scenario_by_label(label);
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value() && deck != nullptr,
         "The registered issue-1879 fixture is unavailable");
  std::mt19937_64 rng(seed);
  sim::Engine engine(*selected, deck->recipe, rng, &trace);
  return engine.run();
}

void exact_registered_witnesses_reach_the_earliest_turn() {
  {
    sim::TraceLog trace{true, {}};
    const sim::TrialOutcome result =
        run_seed("strict-jit/go-first", 146, trace);
    expect(result.first_ready_turn == 4 && !result.setup_failed,
           "Seed 146 did not complete the public K1 T4 route");
    expect(trace_contains(trace, "Mysterious Treasure issue-1879 first cost") &&
               trace_contains(trace, "Quick Ball issue-1879 payload cost") &&
               trace_contains(trace, "T4 | WONDER TAG") &&
               trace_contains(trace, "T4 | READY"),
           "Seed 146 did not use the composed route");
  }
  {
    sim::TraceLog trace{true, {}};
    const sim::TrialOutcome result =
        run_seed("strict-jit/go-second", 163, trace);
    expect(result.first_ready_turn == 3 && !result.setup_failed,
           "Seed 163 did not improve to the public K1 T3 route");
    expect(trace_contains(trace, "Mysterious Treasure issue-1879 first cost") &&
               trace_contains(trace, "Quick Ball issue-1879 payload cost") &&
               trace_contains(trace, "T3 | PLAY SUPPORTER") &&
               trace_contains(trace, "T3 | READY"),
           "Seed 163 did not use the composed route");
  }
}
}  // namespace

int main() {
  try {
    complete_two_connector_costs_and_energy_route();
    knowledge_provenance_boundaries();
    payload_choice_ignores_hidden_deck_order();
    crispin_energy_boundaries();
    public_resource_boundaries_are_enforced();
    exact_registered_witnesses_reach_the_earliest_turn();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
