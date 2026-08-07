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
  static bool issue_2323_protects_final_payload(Engine& engine) {
    return engine.issue_2323_protect_final_t1_payload();
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

int t1_payload_discards(const sim::TraceLog& trace) {
  int count = 0;
  for (const std::string& line : trace.lines) {
    if (line.find("T1 | DISCARD") == std::string::npos) continue;
    if (line.find("Appletun") != std::string::npos ||
        line.find("Mega Dragonite ex") != std::string::npos ||
        line.find("Dragapult ex") != std::string::npos ||
        line.find("Hisuian Goodra VSTAR") != std::string::npos ||
        line.find("Dialga-GX") != std::string::npos) {
      ++count;
    }
  }
  return count;
}

void test_seed_871_starts_timer_and_preserves_final_payload() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2323 exact seed fixture unavailable");
  std::mt19937_64 rng{871};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  (void)engine.run();

  // The refined regression is intentionally limited to the observable T1 route.
  // A legal Mysterious Treasure search shuffles the deck, so the no-action
  // baseline's later fixed T2/T3 draws are not valid post-search expectations.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Official discard, search, shuffle, Bench, Supporter, attachment, and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1, strict-JIT, dynamic DCI, and earliest-route contracts: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Refined bug and source-bound original reproduction: https://github.com/FlareZ123/pokemon-sims/issues/2323 https://github.com/FlareZ123/pokemon-sims/pull/2321
  expect(has(trace, "Mysterious Treasure issue-2323 redundant-payload cost") &&
             has(trace, "T1 | DECK KNOWLEDGE") &&
             has(trace, "T1 | BENCH") && has(trace, "Regidrago V"),
         "issue-2323 seed 871 omitted the K0 Treasure timer route");
  expect(t1_payload_discards(trace) == 1,
         "issue-2323 seed 871 consumed the final protected T1 Dragon payload");
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
  // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2323
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

void test_final_payload_guard_boundary() {
  auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2323 payload-guard fixture unavailable");
  std::mt19937_64 rng{2323};
  sim::Engine engine(*scenario, deck->recipe, rng);

  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::DialgaGX, 0, 0, 0,
                              sim::Tool::None};
  state.bench.push_back(
      sim::Pokemon{sim::Card::RegidragoV, 1, 2, 0, sim::Tool::None});
  state.hand = {sim::Card::Dragapult, sim::Card::EarthenVessel,
                sim::Card::Fire};
  state.discard = {sim::Card::MysteriousTreasure,
                   sim::Card::MegaDragonite};
  state.discarded_this_turn = {sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(engine, true);

  // A T1 payload discarded before an eventual ready turn does not satisfy strict
  // JIT. Once the earliest Regidrago V timer is established, the final held Dragon
  // stays UDP until a same-turn readiness outlet exists.
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Strict-JIT / dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/2323
  expect(sim::EngineTestAccess::issue_2323_protects_final_payload(engine),
         "issue-2323 did not protect the final post-Treasure T1 payload");
}
}  // namespace

int main() {
  try {
    test_seed_871_starts_timer_and_preserves_final_payload();
    test_k0_boundary_controls();
    test_final_payload_guard_boundary();
    std::cout << "Issue 2323 early Treasure Regidrago tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
