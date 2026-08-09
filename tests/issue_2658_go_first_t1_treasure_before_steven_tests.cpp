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
  static bool pre_steven_treasure_available(Engine& engine) {
    return engine.pre_steven_treasure_regidrago_cost().has_value();
  }
  static bool banked_steven_t3_available(Engine& engine) {
    return engine.issue_2658_banked_steven_t3_route_available();
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

void erase_one(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto it = std::find(cards.begin(), cards.end(), card);
  if (it != cards.end()) cards.erase(it);
}

void test_seed_3_uses_t1_treasure_and_deterministic_steven() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2658 exact seed fixture unavailable");
  std::mt19937_64 rng{3};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  (void)engine.run();

  // Going first forbids a T1 Supporter, while Mysterious Treasure remains a legal
  // Item. Treasure can spend the route-replaced Erika, search/Bench Regidrago V,
  // and expose the T1 attachment window. The already-held Steven then reserves the
  // VSTAR, Blender, and any final Grass required after T2's actual attachment, so
  // readiness does not depend on the post-shuffle Crispin or Arven draw identities.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Erika's Invitation: https://api.pokemontcg.io/v2/cards/sv3pt5-160
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official first-turn, Item, Bench, attachment, evolution, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2658
  expect(has(trace, "T1 | DISCARD") && has(trace, "Erika's Invitation") &&
             has(trace, "T1 | DECK KNOWLEDGE") &&
             has(trace, "T1 | BENCH") && has(trace, "T1 | ATTACH"),
         "issue-2658 seed 3 did not use the legal T1 Treasure/Bench/attachment window");
  expect(has(trace, "T2 | PLAY SUPPORTER") &&
             has(trace, "deterministic T3 package") &&
             !has(trace, "T2 | PLAY SUPPORTER | rules: R-CRISPIN-01"),
         "issue-2658 seed 3 did not preserve the source-bounded Steven continuation");
  expect(has(trace, "T3 | PLAY ITEM") && has(trace, "Brilliant Blender") &&
             has(trace, "T3 | READY"),
         "issue-2658 seed 3 did not complete the deterministic strict-JIT T3 route");
}

sim::State route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::StevensResolve,
                sim::Card::ErikasInvitation, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Klara};
  state.deck = {sim::Card::RegidragoV, sim::Card::RegidragoVstar,
                sim::Card::BrilliantBlender, sim::Card::Grass,
                sim::Card::MegaDragonite, sim::Card::TapuLeleGX};
  return state;
}

bool available_for(const char* label, sim::State state,
                   const bool deck_seen = false) {
  auto scenario = sim::scenario_by_label(label);
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2658 control fixture unavailable");
  std::mt19937_64 rng{2658};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(engine, deck_seen);
  return sim::EngineTestAccess::pre_steven_treasure_available(engine);
}

void test_state_driven_boundaries() {
  // This is a K0 route projection. Public exhaustion may disqualify an axis, while
  // hidden Prize identities cannot be consulted before Treasure establishes K1.
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Official first-player T1 Supporter restriction: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1 and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2658
  expect(available_for("strict-jit/go-first", route_state()),
         "issue-2658 rejected its state-driven pre-Steven route");

  expect(!available_for("strict-jit/go-second", route_state()),
         "issue-2658 displaced the legal T1 Steven route going second");
  expect(!available_for("strict-jit-turn2-item-lock/go-first", route_state()),
         "issue-2658 banked a T3 Blender route through scheduled Item lock");
  expect(!available_for("strict-jit/go-first", route_state(), true),
         "issue-2658 used the K0 fallback after deck knowledge was established");

  auto full_bench = route_state();
  for (int i = 0; i < 5; ++i) {
    full_bench.bench.push_back(
        sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0, sim::Tool::None});
  }
  expect(!available_for("strict-jit/go-first", full_bench),
         "issue-2658 ignored full Bench capacity");

  auto no_cost = route_state();
  erase_one(no_cost.hand, sim::Card::ErikasInvitation);
  expect(!available_for("strict-jit/go-first", no_cost),
         "issue-2658 opened without its route-replaced Treasure cost");

  auto no_fire = route_state();
  erase_one(no_fire.hand, sim::Card::Fire);
  expect(!available_for("strict-jit/go-first", no_fire),
         "issue-2658 opened without the deterministic G/F attachment schedule");

  auto no_steven = route_state();
  erase_one(no_steven.hand, sim::Card::StevensResolve);
  expect(!available_for("strict-jit/go-first", no_steven),
         "issue-2658 spent Erika when Steven did not replace its T2 Supporter role");

  auto no_regi = route_state();
  no_regi.discard.insert(no_regi.discard.end(), 4, sim::Card::RegidragoV);
  expect(!available_for("strict-jit/go-first", no_regi),
         "issue-2658 ignored public exhaustion of Regidrago V");

  auto no_vstar = route_state();
  no_vstar.discard.insert(no_vstar.discard.end(), 4,
                          sim::Card::RegidragoVstar);
  expect(!available_for("strict-jit/go-first", no_vstar),
         "issue-2658 opened without a public VSTAR continuation");

  auto no_blender = route_state();
  no_blender.discard.push_back(sim::Card::BrilliantBlender);
  expect(!available_for("strict-jit/go-first", no_blender),
         "issue-2658 opened without a public Blender continuation");

  auto no_payload = route_state();
  for (const sim::Card card : {sim::Card::MegaDragonite, sim::Card::Dragapult,
                               sim::Card::GoodraVstar, sim::Card::DialgaGX,
                               sim::Card::Appletun}) {
    no_payload.discard.insert(no_payload.discard.end(), 4, card);
  }
  expect(!available_for("strict-jit/go-first", no_payload),
         "issue-2658 opened without a public strict-JIT payload continuation");
}

sim::State banked_steven_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::LatiasEx, 0, 0, 0,
                              sim::Tool::None};
  state.bench.push_back(
      sim::Pokemon{sim::Card::RegidragoV, 1, 0, 1, sim::Tool::None});
  state.hand = {sim::Card::StevensResolve, sim::Card::Fire,
                sim::Card::Crispin, sim::Card::Klara};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::BrilliantBlender,
                sim::Card::Grass, sim::Card::MegaDragonite,
                sim::Card::TapuLeleGX};
  return state;
}

bool banked_available_for(const char* label, sim::State state) {
  auto scenario = sim::scenario_by_label(label);
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2658 banked-Steven fixture unavailable");
  std::mt19937_64 rng{2658};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(engine, true);
  return sim::EngineTestAccess::banked_steven_t3_available(engine);
}

void test_banked_steven_boundaries() {
  // The T2 continuation is evaluated from K1 public state. The prior-turn Regidrago
  // has one Grass, held Fire guarantees the second attachment channel, and Steven
  // can reserve VSTAR, Blender, and the possible final Grass without using Crispin.
  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
  // Official attachment, Supporter, evolution, Item, and Retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, strict-JIT, and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2658
  expect(banked_available_for("strict-jit/go-first", banked_steven_state()),
         "issue-2658 rejected its deterministic K1 T2 Steven continuation");

  expect(!banked_available_for("strict-jit/go-second", banked_steven_state()),
         "issue-2658 admitted the banked route outside the going-first schedule");
  expect(!banked_available_for("strict-jit-turn2-item-lock/go-first",
                               banked_steven_state()),
         "issue-2658 projected Blender through the scheduled Item lock");

  auto no_fire = banked_steven_state();
  erase_one(no_fire.hand, sim::Card::Fire);
  expect(!banked_available_for("strict-jit/go-first", no_fire),
         "issue-2658 admitted the T2 continuation without held Fire");

  auto no_steven = banked_steven_state();
  erase_one(no_steven.hand, sim::Card::StevensResolve);
  expect(!banked_available_for("strict-jit/go-first", no_steven),
         "issue-2658 admitted the T2 continuation without Steven");

  auto blender_held = banked_steven_state();
  blender_held.hand.push_back(sim::Card::BrilliantBlender);
  expect(!banked_available_for("strict-jit/go-first", blender_held),
         "issue-2658 displaced a state where Blender no longer needs Steven");

  auto fresh_regi = banked_steven_state();
  fresh_regi.bench.front().entered_turn = 2;
  expect(!banked_available_for("strict-jit/go-first", fresh_regi),
         "issue-2658 ignored the evolution timer");
}
}  // namespace

int main() {
  try {
    test_seed_3_uses_t1_treasure_and_deterministic_steven();
    test_state_driven_boundaries();
    test_banked_steven_boundaries();
    std::cout << "Issue 2658 pre-Steven Treasure tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
