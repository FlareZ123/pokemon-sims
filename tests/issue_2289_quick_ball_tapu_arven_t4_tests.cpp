#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true,
                        const bool prizes_revealed = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static const State& state(const Engine& engine) { return engine.state_; }

  static bool route_available(const Engine& engine) {
    return engine.issue_2289_quick_ball_tapu_arven_route_available();
  }

  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
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

sim::Scenario route_scenario(
    const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{"issue-2289", sim::DciProfile::StrictJit, locks, true, 4};
}

sim::State route_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::QuickBall, sim::Card::MegaDragonite,
                sim::Card::Fire, sim::Card::Klara,
                sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Arven,
                sim::Card::ForestSealStone, sim::Card::EarthenVessel,
                sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire, sim::Card::RegidragoV};
  state.prizes = {sim::Card::Crispin};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, sim::TraceLog* trace = nullptr,
                        const bool deck_seen = true,
                        const bool prizes_revealed = true) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen,
                                   prizes_revealed);
  return engine;
}

void exact_composed_connector() {
  std::mt19937_64 rng{228901};
  sim::TraceLog trace{true, {}};
  const sim::Scenario scenario = route_scenario();
  sim::Engine engine = make_engine(scenario, rng, route_state(), &trace);

  // Quick Ball discards one other card and searches a Basic Pokemon. The Dragon
  // cost is the required current-turn strict-JIT payload. Wonder Tag obtains Arven;
  // Arven takes Forest Seal Stone plus Earthen Vessel; Vessel spends the held Fire
  // that is route-surplus because the Active Regidrago V already has Fire and takes
  // the missing Grass for the unused manual attachment.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official Item, discard, search, Bench, Ability, Supporter, Tool, VSTAR Power,
  // evolution, and Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1, DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed stale-claim bug: https://github.com/FlareZ123/pokemon-sims/issues/2289
  expect(sim::EngineTestAccess::route_available(engine),
         "issue-2289 exact K1 connector was not available");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "issue-2289 Quick Ball connector did not execute");

  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(std::count(after.discard.begin(), after.discard.end(),
                    sim::Card::QuickBall) == 1,
         "issue-2289 Quick Ball did not enter discard");
  expect(std::count(after.discard.begin(), after.discard.end(),
                    sim::Card::MegaDragonite) == 1,
         "issue-2289 Dragon payload did not pay Quick Ball");
  expect(std::any_of(after.bench.begin(), after.bench.end(),
                     [](const sim::Pokemon& pokemon) {
                       return pokemon.card == sim::Card::TapuLeleGX;
                     }),
         "issue-2289 Tapu Lele-GX was not played to the Bench");
  expect(after.supporter_used &&
             std::count(after.discard.begin(), after.discard.end(),
                        sim::Card::Arven) == 1,
         "issue-2289 Arven was not played as the T4 Supporter");
  expect(std::count(after.hand.begin(), after.hand.end(),
                    sim::Card::ForestSealStone) == 1,
         "issue-2289 Arven did not obtain Forest Seal Stone");
  expect(std::count(after.discard.begin(), after.discard.end(),
                    sim::Card::EarthenVessel) == 1 &&
             std::count(after.discard.begin(), after.discard.end(),
                        sim::Card::Fire) == 1,
         "issue-2289 Earthen Vessel did not spend the route-surplus Fire");
  expect(std::count(after.hand.begin(), after.hand.end(), sim::Card::Grass) == 1,
         "issue-2289 Earthen Vessel did not obtain Grass Energy");
  expect(has(trace, "issue-2289") && has(trace, "WONDER TAG") &&
             has(trace, "PLAY SUPPORTER") && has(trace, "EARTHEN VESSEL"),
         "issue-2289 connector trace was incomplete");
}

void preflight_controls() {
  const auto blocked = [](sim::State state, const sim::Scenario scenario,
                          const std::uint64_t seed, const char* message,
                          const bool deck_seen = true,
                          const bool prizes_revealed = true) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state), nullptr,
                                     deck_seen, prizes_revealed);
    expect(!sim::EngineTestAccess::route_available(engine), message);
  };

  // The route is K1-only and every searched card must be known available. Arven
  // must be able to find both Forest Seal Stone and Earthen Vessel; Star Alchemy
  // must still be unused; Vessel must have the route-surplus Fire cost and a Grass
  // target; the prior-turn Active Regidrago V must remain legally evolvable.
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Official Tool, VSTAR Power, Energy attachment, evolution, Item, Ability, and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and resource-preserving route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed stale-claim bug: https://github.com/FlareZ123/pokemon-sims/issues/2289
  blocked(route_state(), route_scenario(), 228902,
          "issue-2289 admitted K0", false, false);

  std::uint64_t seed = 228903;
  for (const sim::Card target : {sim::Card::TapuLeleGX, sim::Card::Arven,
                                 sim::Card::ForestSealStone,
                                 sim::Card::EarthenVessel,
                                 sim::Card::RegidragoVstar,
                                 sim::Card::Grass}) {
    sim::State state = route_state();
    erase_one(state.deck, target);
    blocked(std::move(state), route_scenario(), seed++,
            "issue-2289 admitted a missing K1 route target");
  }

  sim::State no_payload = route_state();
  erase_one(no_payload.hand, sim::Card::MegaDragonite);
  blocked(std::move(no_payload), route_scenario(), seed++,
          "issue-2289 admitted no current-turn Dragon payload");

  sim::State no_vessel_cost = route_state();
  erase_one(no_vessel_cost.hand, sim::Card::Fire);
  blocked(std::move(no_vessel_cost), route_scenario(), seed++,
          "issue-2289 admitted no route-surplus Vessel cost");

  sim::State spent_attachment = route_state();
  spent_attachment.manual_energy_used = true;
  blocked(std::move(spent_attachment), route_scenario(), seed++,
          "issue-2289 admitted a spent manual attachment");

  sim::State spent_vstar = route_state();
  spent_vstar.vstar_power_used = true;
  blocked(std::move(spent_vstar), route_scenario(), seed++,
          "issue-2289 admitted a spent VSTAR Power");

  sim::State spent_supporter = route_state();
  spent_supporter.supporter_used = true;
  blocked(std::move(spent_supporter), route_scenario(), seed++,
          "issue-2289 admitted a spent Supporter action");

  sim::State same_turn = route_state();
  same_turn.active->entered_turn = 4;
  blocked(std::move(same_turn), route_scenario(), seed++,
          "issue-2289 admitted a same-turn Regidrago V evolution");

  sim::State wrong_energy = route_state();
  wrong_energy.active->grass = 2;
  blocked(std::move(wrong_energy), route_scenario(), seed++,
          "issue-2289 admitted an already-complete Grass axis");

  sim::State no_fire_on_attacker = route_state();
  no_fire_on_attacker.active->fire = 0;
  blocked(std::move(no_fire_on_attacker), route_scenario(), seed++,
          "issue-2289 treated the held Fire as surplus before Fire was attached");

  sim::State occupied_tool = route_state();
  occupied_tool.active->tool = sim::Tool::Powerglass;
  blocked(std::move(occupied_tool), route_scenario(), seed++,
          "issue-2289 admitted an occupied Tool slot");

  sim::State full_bench = route_state();
  for (int index = 0; index < 5; ++index) {
    full_bench.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1});
  }
  blocked(std::move(full_bench), route_scenario(), seed++,
          "issue-2289 admitted a full Bench");

  blocked(route_state(), route_scenario(sim::LockMode::FullItem), seed++,
          "issue-2289 admitted Item lock");
  blocked(route_state(), route_scenario(sim::LockMode::FullRuleBoxAbility), seed++,
          "issue-2289 admitted Rule Box Ability lock");
}

void exact_seed_52_reaches_t4() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const auto* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario && deck, "issue-2289 registered seed fixture unavailable");

  std::mt19937_64 rng{52};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Quick Ball -> Tapu Lele-GX -> Arven -> Forest Seal Stone + Earthen Vessel
  // is deterministic from the T1 Heavy Ball K1 state. Vessel spends the held Fire
  // after the Active Regidrago V already has Fire, Star Alchemy finds VSTAR, and
  // the searched Grass supplies the unused manual attachment for GGF on T4.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Earliest-route, K1, DCI, and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed stale-claim bug: https://github.com/FlareZ123/pokemon-sims/issues/2289
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "issue-2289 seed 52 did not reach readiness on T4");
  expect(has(trace, "Quick Ball issue-2289 current-turn payload cost") &&
             has(trace, "Arven searched Forest Seal Stone and Earthen Vessel") &&
             has(trace, "Earthen Vessel issue-2289 route-surplus Fire cost") &&
             has(trace, "T4 | READY"),
         "issue-2289 seed 52 omitted the proven T4 connector route");
}

}  // namespace

int main() {
  exact_composed_connector();
  preflight_controls();
  exact_seed_52_reaches_t4();
  return 0;
}
