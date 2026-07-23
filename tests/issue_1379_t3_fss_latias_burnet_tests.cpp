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
  static void set_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }
  static Card fss_target(const Engine& engine) {
    return engine.fss_target_after_search_started();
  }
};
}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&needle](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

const sim::DeckRecipe& recipe() {
  static const sim::DeckRecipe value = sim::baseline_recipe();
  return value;
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1379", sim::DciProfile::StrictJit,
                       lock, false, 5};
}

sim::State post_crispin_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};
  state.bench = {
      sim::Pokemon{sim::Card::TapuLeleGX, 1},
      sim::Pokemon{sim::Card::RegidragoV, 2, 2, 0,
                   sim::Tool::ForestSealStone},
  };
  state.hand = {sim::Card::ProfessorBurnet, sim::Card::RegidragoVstar,
                sim::Card::Fire, sim::Card::GoodraVstar};
  state.deck = {sim::Card::LatiasEx, sim::Card::BrilliantBlender,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::QuickBall, sim::Card::TateLiza};
  state.prizes = {sim::Card::Powerglass, sim::Card::Lusamine,
                  sim::Card::ChaoticSwell, sim::Card::Arven,
                  sim::Card::MysteriousTreasure, sim::Card::DialgaGX};
  return state;
}

sim::Card target_for(sim::State state,
                     const sim::LockMode lock = sim::LockMode::None) {
  std::mt19937_64 rng{1379};
  const sim::Scenario selected = scenario(lock);
  sim::Engine engine(selected, recipe(), rng);
  sim::EngineTestAccess::set_k1_state(engine, std::move(state));
  return sim::EngineTestAccess::fss_target(engine);
}

void test_exact_projected_and_post_attachment_states_take_latias() {
  // Burnet already covers the payload axis. Latias is the direct connector from
  // Star Alchemy to Skyliner and the unresolved Active-position axis:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Original projection: https://github.com/FlareZ123/pokemon-sims/issues/1137
  // Preceding fix: https://github.com/FlareZ123/pokemon-sims/issues/1378
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1379
  expect(target_for(post_crispin_state()) == sim::Card::LatiasEx,
         "The projected T3 post-Crispin state did not choose Latias.");

  sim::State completed = post_crispin_state();
  completed.bench[1].fire = 1;
  completed.manual_energy_used = true;
  expect(target_for(std::move(completed)) == sim::Card::LatiasEx,
         "The actual post-attachment GGF state did not choose Latias.");
}

void test_route_preserves_blender_when_any_required_axis_is_missing() {
  sim::State no_burnet = post_crispin_state();
  no_burnet.hand.erase(std::remove(no_burnet.hand.begin(), no_burnet.hand.end(),
                                   sim::Card::ProfessorBurnet),
                       no_burnet.hand.end());
  expect(target_for(std::move(no_burnet)) == sim::Card::BrilliantBlender,
         "Missing Burnet failed to preserve the direct Blender payload route.");

  sim::State no_fire = post_crispin_state();
  no_fire.hand.erase(std::remove(no_fire.hand.begin(), no_fire.hand.end(),
                                 sim::Card::Fire), no_fire.hand.end());
  expect(target_for(std::move(no_fire)) == sim::Card::BrilliantBlender,
         "Missing Fire failed to preserve Blender.");

  sim::State spent_attachment = post_crispin_state();
  spent_attachment.manual_energy_used = true;
  expect(target_for(std::move(spent_attachment)) == sim::Card::BrilliantBlender,
         "An incomplete Energy axis with a spent attachment selected Latias.");

  sim::State same_turn_regi = post_crispin_state();
  same_turn_regi.bench[1].entered_turn = same_turn_regi.turn;
  expect(target_for(std::move(same_turn_regi)) == sim::Card::BrilliantBlender,
         "A same-turn Regidrago V selected the impossible evolution route.");

  sim::State full_bench = post_crispin_state();
  full_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  full_bench.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 1});
  expect(target_for(std::move(full_bench)) == sim::Card::BrilliantBlender,
         "A full Bench selected Latias.");

  expect(target_for(post_crispin_state(), sim::LockMode::FullRuleBoxAbility) ==
             sim::Card::BrilliantBlender,
         "Rule Box Ability lock selected the Skyliner route.");
}

void test_exact_seed_12_reaches_t3_after_issue_1378() {
  const auto selected = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value() && deck != nullptr,
         "The registered no-lock seed-12 fixture is unavailable.");

  std::mt19937_64 rng{12};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*selected, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 3,
         "The corrected combined seed-12 route did not reach T3.");
  expect(trace_contains(trace, "Searched any card: Latias ex"),
         "Star Alchemy did not select Latias ex in the exact continuation.");
  expect(trace_contains(trace, "Searched and discarded:"),
         "Professor Burnet did not establish the same-turn payload.");
  expect(trace_contains(trace, "Latias ex gives the Basic Active no Retreat Cost"),
         "Latias did not resolve the Active-position axis.");
  expect(trace_contains(trace, "T3 | READY"),
         "The exact trace omitted the T3 ready state.");
}

}  // namespace

int main() {
  try {
    test_exact_projected_and_post_attachment_states_take_latias();
    test_route_preserves_blender_when_any_required_axis_is_missing();
    test_exact_seed_12_reaches_t3_after_issue_1378();
    std::cout << "Issue 1379 T3 Star Alchemy Latias tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
