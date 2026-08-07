#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void set_knowledge(Engine& engine, const bool deck_seen,
                            const bool prizes_revealed = false) {
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool issue_2323_available(Engine& engine) {
    return engine.issue_2323_redundant_payload_cost().has_value();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void test_seed_871_reaches_t3_without_future_oracle() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2323 exact seed fixture unavailable");
  std::mt19937_64 rng{871};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official Item, discard, search, Bench, Supporter, attachment, evolution, Ability, Prize, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1, strict-JIT, DCI, and earliest-route contracts: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug and source-bound reproduction: https://github.com/FlareZ123/pokemon-sims/issues/2323 https://github.com/FlareZ123/pokemon-sims/pull/2321
  expect(outcome.first_ready_turn == 3 && !outcome.setup_failed,
         "issue-2323 seed 871 did not become ready on T3");
  expect(has(trace, "Mysterious Treasure issue-2323 redundant-payload cost") &&
             has(trace, "T1 | DECK KNOWLEDGE") &&
             has(trace, "T1 | BENCH") && has(trace, "Regidrago V") &&
             has(trace, "T3 | READY"),
         "issue-2323 seed 871 omitted the K0 Treasure timer route or T3 finish");
}

sim::State k0_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::MegaDragonite,
                sim::Card::Dragapult, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Powerglass};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::TapuLeleGX, sim::Card::LatiasEx,
                sim::Card::Gladion, sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool available_for(const char* label, sim::State state,
                   const bool deck_seen = false, const int max_turn = 5) {
  auto scenario = sim::scenario_by_label(label);
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2323 control fixture unavailable");
  scenario->max_turn = max_turn;
  std::mt19937_64 rng{2323};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(engine, deck_seen);
  return sim::EngineTestAccess::issue_2323_available(engine);
}

void erase_one(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto it = std::find(cards.begin(), cards.end(), card);
  if (it != cards.end()) cards.erase(it);
}

void test_k0_boundary_controls() {
  // The exception exists only for the pre-inspection two-distinct-payload state.
  // A single payload remains UDP under strict JIT, while K1 states and lock states
  // stay delegated to the established exact-deck selectors.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Strict-JIT and dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // K0/K1: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2323
  expect(available_for("strict-jit/go-second", k0_state()),
         "issue-2323 rejected its exact K0 structural state");

  auto one_payload = k0_state();
  erase_one(one_payload.hand, sim::Card::Dragapult);
  expect(!available_for("strict-jit/go-second", one_payload),
         "issue-2323 spent the final protected payload");

  auto duplicate_identity = k0_state();
  erase_one(duplicate_identity.hand, sim::Card::Dragapult);
  duplicate_identity.hand.push_back(sim::Card::MegaDragonite);
  expect(!available_for("strict-jit/go-second", duplicate_identity),
         "issue-2323 treated duplicate copies as two distinct payload identities");

  expect(!available_for("strict-jit/go-second", k0_state(), true),
         "issue-2323 bypassed the K0-only boundary after legal inspection");
  expect(!available_for("strict-jit-rulebox-ability-lock/go-second", k0_state()),
         "issue-2323 bypassed a registered lock scenario");
  expect(!available_for("strict-jit/go-first", k0_state()),
         "issue-2323 incorrectly used the going-second Crispin route going first");

  auto full_bench = k0_state();
  for (int i = 0; i < 5; ++i) {
    full_bench.bench.push_back(
        sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0, sim::Tool::None});
  }
  expect(!available_for("strict-jit/go-second", full_bench),
         "issue-2323 ignored full Bench capacity");

  auto no_crispin = k0_state();
  erase_one(no_crispin.hand, sim::Card::Crispin);
  expect(!available_for("strict-jit/go-second", no_crispin),
         "issue-2323 opened without observable Supporter Energy progression");

  auto no_grass = k0_state();
  erase_one(no_grass.hand, sim::Card::Grass);
  expect(!available_for("strict-jit/go-second", no_grass),
         "issue-2323 opened without the held manual Grass attachment");

  auto no_regi_possible = k0_state();
  no_regi_possible.discard.insert(no_regi_possible.discard.end(), 4,
                                  sim::Card::RegidragoV);
  expect(!available_for("strict-jit/go-second", no_regi_possible),
         "issue-2323 ignored public exhaustion of all Regidrago V copies");

  auto no_fire_possible = k0_state();
  no_fire_possible.discard.insert(no_fire_possible.discard.end(), 3,
                                  sim::Card::Fire);
  expect(!available_for("strict-jit/go-second", no_fire_possible),
         "issue-2323 ignored public exhaustion of all Fire Energy copies");

  expect(!available_for("strict-jit/go-second", k0_state(), false, 2),
         "issue-2323 opened after the evolution horizon could no longer improve T3");
}
}  // namespace

int main() {
  try {
    test_seed_871_reaches_t3_without_future_oracle();
    test_k0_boundary_controls();
    std::cout << "Issue 2323 early Treasure Regidrago tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
