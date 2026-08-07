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
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_knowledge(Engine& engine, const bool deck_seen,
                            const bool prizes_revealed = false) {
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static void set_banked(Engine& engine, const bool value) {
    engine.issue_2301_banked_t4_route_ = value;
  }
  static bool banked(const Engine& engine) {
    return engine.issue_2301_banked_t4_route_;
  }
  static bool k0_available(Engine& engine) {
    return engine.issue_2301_quick_ball_timer_cost().has_value();
  }
  static bool k1_proven(Engine& engine) {
    return engine.issue_2301_k1_t4_route_proven();
  }
  static bool complete_t4(Engine& engine) {
    return engine.complete_issue_2301_banked_t4_route();
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

void test_exact_seed_38_is_deterministic_t4() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario && deck, "issue-2301 exact seed fixture unavailable");
  std::mt19937_64 rng{38};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1, DCI, strict-JIT, and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "issue-2301 seed 38 did not become ready on T4");
  expect(has(trace, "Quick Ball issue-2301 timer cost") &&
             has(trace, "T3 | BANK ROUTE") &&
             !has(trace, "T3 | PLAY SUPPORTER") &&
             has(trace, "T4 | DISCARD") &&
             has(trace, "Earthen Vessel issue-2301 T4 payload cost") &&
             has(trace, "T4 | COMPLETE ROUTE") && has(trace, "T4 | READY"),
         "issue-2301 seed 38 omitted the banked T3 action or deterministic T4 finish");
}

sim::State k0_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.stadium = sim::Stadium::ForestOfVitality;
  state.hand = {sim::Card::QuickBall, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::SecretBox,
                sim::Card::GoodraVstar, sim::Card::MegaDragonite};
  // K0 means the policy cannot inspect the hidden deck. The simulator still needs
  // a physical hidden zone so might_be_unseen() can represent fixed-list
  // plausibility without revealing exact Prize identities to the decision policy.
  // K0/K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  state.deck = {sim::Card::RegidragoV, sim::Card::ForestSealStone,
                sim::Card::Dawn, sim::Card::Pineco, sim::Card::ForretressEx,
                sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool k0_available_for(const char* label, sim::State state, const int max_turn = 4) {
  auto scenario = sim::scenario_by_label(label);
  const auto* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario && deck, "issue-2301 K0 fixture unavailable");
  scenario->max_turn = max_turn;
  std::mt19937_64 rng{2301};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(engine, false);
  return sim::EngineTestAccess::k0_available(engine);
}

void test_k0_controls() {
  // The dynamic-DCI exception is deliberately narrower than generic Quick Ball.
  // Lower-DCI cards still win, the final payload is protected, and all lock,
  // Bench, retreat, Forest, and horizon gates remain mandatory.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Core Item, Bench, retreat, and Ability procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(k0_available_for("strict-jit/go-second", k0_state()),
         "issue-2301 rejected its exact K0 public state");

  auto one_payload = k0_state();
  one_payload.hand.erase(std::find(one_payload.hand.begin(), one_payload.hand.end(),
                                   sim::Card::GoodraVstar));
  expect(!k0_available_for("strict-jit/go-second", one_payload),
         "issue-2301 spent the final held payload");

  auto no_vessel = k0_state();
  no_vessel.hand.erase(std::find(no_vessel.hand.begin(), no_vessel.hand.end(),
                                 sim::Card::EarthenVessel));
  expect(!k0_available_for("strict-jit/go-second", no_vessel),
         "issue-2301 opened without the reserved Vessel outlet");

  auto lower_dci = k0_state();
  lower_dci.hand.push_back(sim::Card::BattleVipPass);
  expect(!k0_available_for("strict-jit/go-second", lower_dci),
         "issue-2301 skipped an ordinary lower-DCI Quick Ball cost");

  auto full_bench = k0_state();
  full_bench.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 2, 0, 0, sim::Tool::None});
  expect(!k0_available_for("strict-jit/go-second", full_bench),
         "issue-2301 ignored the required empty/open Bench state");

  auto spent_retreat = k0_state();
  spent_retreat.retreat_used = true;
  expect(!k0_available_for("strict-jit/go-second", spent_retreat),
         "issue-2301 ignored spent retreat");

  auto no_forest = k0_state();
  no_forest.stadium = sim::Stadium::None;
  expect(!k0_available_for("strict-jit/go-second", no_forest),
         "issue-2301 opened without Forest of Vitality");

  expect(!k0_available_for("strict-jit-turn2-item-lock/go-second", k0_state()),
         "issue-2301 bypassed Item lock");
  expect(!k0_available_for("strict-jit-rulebox-ability-lock/go-second", k0_state()),
         "issue-2301 bypassed Rule Box Ability lock");
  expect(!k0_available_for("strict-jit-supporter-lock/go-second", k0_state()),
         "issue-2301 bypassed Supporter lock");
  expect(!k0_available_for("strict-jit/go-second", k0_state(), 3),
         "issue-2301 opened beyond the T4 setup horizon");
}

sim::State k1_state() {
  sim::State state = k0_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::QuickBall));
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::GoodraVstar));
  state.hand.push_back(sim::Card::RegidragoV);
  state.deck = {sim::Card::ForestSealStone, sim::Card::Dawn, sim::Card::Pineco,
                sim::Card::ForretressEx, sim::Card::RegidragoVstar,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::QuickBall, sim::Card::RegidragoV};
  return state;
}

bool k1_proven_for(sim::State state) {
  auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario && deck, "issue-2301 K1 fixture unavailable");
  std::mt19937_64 rng{2301};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(engine, true);
  return sim::EngineTestAccess::k1_proven(engine);
}

void erase_all(std::vector<sim::Card>& cards, const sim::Card card) {
  cards.erase(std::remove(cards.begin(), cards.end(), card), cards.end());
}

void test_k1_exact_target_controls() {
  // K0 may only know fixed-list plausibility. After the legal Quick Ball search,
  // K1 must prove every physical target before banking the expensive future route.
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(k1_proven_for(k1_state()), "issue-2301 rejected the exact K1 route proof");

  for (const sim::Card missing : {sim::Card::ForestSealStone, sim::Card::Dawn,
                                  sim::Card::Pineco, sim::Card::ForretressEx,
                                  sim::Card::RegidragoVstar, sim::Card::Fire}) {
    auto state = k1_state();
    erase_all(state.deck, missing);
    erase_all(state.hand, missing);
    if (missing == sim::Card::RegidragoVstar) state.hand.push_back(sim::Card::RegidragoVstar);
    expect(!k1_proven_for(state), "issue-2301 banked with a required K1 target absent");
  }

  auto insufficient_grass = k1_state();
  erase_all(insufficient_grass.deck, sim::Card::Grass);
  insufficient_grass.deck.push_back(sim::Card::Grass);
  insufficient_grass.deck.push_back(sim::Card::Grass);
  expect(!k1_proven_for(insufficient_grass),
         "issue-2301 banked without enough Grass for arbitrary T4 draw plus Forretress");
}

sim::State t4_state(const sim::Card extra) {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 3, 1, 0, sim::Tool::None});
  state.stadium = sim::Stadium::ForestOfVitality;
  state.hand = {sim::Card::EarthenVessel, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::SecretBox,
                sim::Card::MegaDragonite, extra};
  state.deck = {sim::Card::ForestSealStone, sim::Card::Dawn, sim::Card::Pineco,
                sim::Card::ForretressEx, sim::Card::RegidragoVstar,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::QuickBall, sim::Card::RegidragoV};
  return state;
}

void test_t4_does_not_need_draw_as_secret_box_cost() {
  // Representative arbitrary T4 draws remain outside Secret Box's three known
  // route-replaced costs. The same completion succeeds for low-DCI, live Supporter,
  // Basic Pokémon, and protected singleton-like identities.
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  for (const sim::Card extra : {sim::Card::QuickBall, sim::Card::Arven,
                                sim::Card::RegidragoV, sim::Card::Gladion}) {
    auto scenario = sim::scenario_by_label("strict-jit/go-second");
    const auto* deck = sim::deck_by_id("regidrago-pineco");
    expect(scenario && deck, "issue-2301 T4 fixture unavailable");
    std::mt19937_64 rng{2301 + static_cast<unsigned>(extra)};
    sim::Engine engine(*scenario, deck->recipe, rng);
    sim::EngineTestAccess::set_state(engine, t4_state(extra));
    sim::EngineTestAccess::set_knowledge(engine, true);
    sim::EngineTestAccess::set_banked(engine, true);
    expect(sim::EngineTestAccess::complete_t4(engine),
           "issue-2301 deterministic T4 route failed for a representative arbitrary draw");
  }
}
}  // namespace

int main() {
  try {
    test_exact_seed_38_is_deterministic_t4();
    test_k0_controls();
    test_k1_exact_target_controls();
    test_t4_does_not_need_draw_as_secret_box_cost();
    std::cout << "Issue 2301 deterministic Pineco timer tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
