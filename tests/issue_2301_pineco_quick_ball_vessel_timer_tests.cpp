#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
                            const bool prizes_revealed) {
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool issue_2301_available(Engine& engine) {
    return engine.issue_2301_quick_ball_vessel_timer_cost().has_value();
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

void test_exact_seed_38_reaches_t4() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario && deck, "issue-2301 exact-seed fixture unavailable");

  std::mt19937_64 rng{38};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1, strict-JIT, DCI, and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "issue-2301 seed 38 did not reach strict-JIT readiness on T4");
  expect(has(trace, "Quick Ball issue-2301 timer cost") &&
             has(trace, "T3 | BENCH") && has(trace, "Regidrago V from hand") &&
             has(trace, "T3 | ATTACH") && has(trace, "T4 | READY"),
         "issue-2301 seed 38 omitted the T3 timer route or T4 finish");
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.stadium = sim::Stadium::ForestOfVitality;
  state.hand = {sim::Card::QuickBall, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::SecretBox,
                sim::Card::GoodraVstar, sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoV, sim::Card::ForestSealStone,
                sim::Card::Dawn, sim::Card::Pineco, sim::Card::ForretressEx,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool available_for(const char* label, sim::State state,
                   const bool deck_seen = false, const int max_turn = 4) {
  auto scenario = sim::scenario_by_label(label);
  const auto* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario && deck, "issue-2301 control fixture unavailable");
  scenario->max_turn = max_turn;
  std::mt19937_64 rng{2301};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(engine, deck_seen, false);
  return sim::EngineTestAccess::issue_2301_available(engine);
}

void test_public_route_controls() {
  // The exception must never spend the last held payload, bypass a lower-DCI
  // Quick Ball cost, cross a modeled lock, or rely on a horizon that cannot reach T4.
  // Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // K0/K1 and no-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(available_for("strict-jit/go-second", route_state()),
         "issue-2301 rejected the exact public K0 route shape");

  auto one_payload = route_state();
  one_payload.hand.erase(std::find(one_payload.hand.begin(), one_payload.hand.end(),
                                   sim::Card::GoodraVstar));
  expect(!available_for("strict-jit/go-second", one_payload),
         "issue-2301 spent the final held payload");

  auto no_vessel = route_state();
  no_vessel.hand.erase(std::find(no_vessel.hand.begin(), no_vessel.hand.end(),
                                 sim::Card::EarthenVessel));
  expect(!available_for("strict-jit/go-second", no_vessel),
         "issue-2301 opened without the reserved Vessel outlet");

  auto lower_dci = route_state();
  lower_dci.hand.push_back(sim::Card::BattleVipPass);
  expect(!available_for("strict-jit/go-second", lower_dci),
         "issue-2301 skipped an ordinary lower-DCI Quick Ball cost");

  expect(!available_for("strict-jit-turn2-item-lock/go-second", route_state()),
         "issue-2301 bypassed Item lock");
  expect(!available_for("strict-jit-rulebox-ability-lock/go-second", route_state()),
         "issue-2301 bypassed Rule Box Ability lock");
  expect(!available_for("strict-jit-supporter-lock/go-second", route_state()),
         "issue-2301 bypassed Supporter lock");
  expect(!available_for("strict-jit/go-second", route_state(), false, 3),
         "issue-2301 opened beyond the setup horizon");

  auto no_forest = route_state();
  no_forest.stadium = sim::Stadium::None;
  expect(!available_for("strict-jit/go-second", no_forest),
         "issue-2301 opened without Forest of Vitality");

  auto no_regi_k1 = route_state();
  no_regi_k1.deck.erase(std::find(no_regi_k1.deck.begin(), no_regi_k1.deck.end(),
                                  sim::Card::RegidragoV));
  expect(!available_for("strict-jit/go-second", no_regi_k1, true),
         "issue-2301 used an absent K1 Regidrago V target");
}
}  // namespace

int main() {
  test_exact_seed_38_reaches_t4();
  test_public_route_controls();
  return 0;
}
