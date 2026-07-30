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
  static void set_state(Engine& engine, State state, const bool deck_k1,
                        const bool prize_k1 = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_k1;
    engine.prizes_revealed_ = prize_k1;
  }
  static bool attach_manual(Engine& engine) { return engine.attach_manual(); }
  static bool replay_available(const Engine& engine) {
    return engine.issue_1912_post_attachment_blender_available();
  }
  static const State& state(const Engine& engine) { return engine.state_; }
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

sim::Scenario scenario(const sim::DciProfile profile = sim::DciProfile::StrictJit,
                       const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1912-post-attachment-blender", profile, lock,
                       true, 5};
}

sim::State exact_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 0,
                              sim::Tool::ForestSealStone};
  state.hand = {sim::Card::Fire, sim::Card::BrilliantBlender,
                sim::Card::TateLiza, sim::Card::ChaoticSwell};
  state.deck = {sim::Card::Dragapult, sim::Card::Grass,
                sim::Card::QuickBall};
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  explicit Fixture(const sim::DciProfile profile = sim::DciProfile::StrictJit,
                   const sim::LockMode lock = sim::LockMode::None)
      : scenario_value(scenario(profile, lock)),
        recipe(sim::deck_by_id("regidrago-shell")->recipe),
        rng(1912),
        engine(scenario_value, recipe, rng) {}
};

void prize_inspection_k1_replays_blender_after_ggf_attachment() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, exact_state(), false, true);

  // Heavy Ball or Gladion inspection establishes the same K1 state as a deck
  // search. The final Fire attachment completes GGF without ending the turn, so
  // the held ACE SPEC may immediately discard a known Dragon payload:
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
  // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
  // Official Energy attachment, Item, search, discard, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 and current-turn JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1912
  expect(sim::EngineTestAccess::attach_manual(fixture.engine),
         "The final Fire attachment was rejected.");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(after.active && after.active->grass == 2 && after.active->fire == 1,
         "The final attachment did not complete GGF.");
  expect(contains(after.discard, sim::Card::BrilliantBlender),
         "The held Brilliant Blender was not replayed after attachment.");
  expect(contains(after.discarded_this_turn, sim::Card::Dragapult),
         "Brilliant Blender did not create the current-turn Dragon payload.");
}

void replay_gate_preserves_required_boundaries() {
  const auto blender_spent = [](const sim::Engine& engine) {
    return contains(sim::EngineTestAccess::state(engine).discard,
                    sim::Card::BrilliantBlender);
  };
  {
    Fixture k0;
    sim::EngineTestAccess::set_state(k0.engine, exact_state(), false, false);
    sim::EngineTestAccess::attach_manual(k0.engine);
    expect(!blender_spent(k0.engine), "K0 replayed Blender without inspection.");
  }
  {
    Fixture item_lock{sim::DciProfile::StrictJit, sim::LockMode::FullItem};
    sim::EngineTestAccess::set_state(item_lock.engine, exact_state(), true);
    sim::EngineTestAccess::attach_manual(item_lock.engine);
    expect(!blender_spent(item_lock.engine),
           "Item lock allowed the post-attachment Blender replay.");
  }
  {
    Fixture no_payload;
    sim::State state = exact_state();
    state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(),
                                    sim::is_payload),
                     state.deck.end());
    sim::EngineTestAccess::set_state(no_payload.engine, std::move(state), true);
    sim::EngineTestAccess::attach_manual(no_payload.engine);
    expect(!blender_spent(no_payload.engine),
           "Blender was spent with no known deck payload.");
  }
  {
    Fixture incomplete;
    sim::State state = exact_state();
    state.active->grass = 1;
    sim::EngineTestAccess::set_state(incomplete.engine, std::move(state), true);
    sim::EngineTestAccess::attach_manual(incomplete.engine);
    expect(!blender_spent(incomplete.engine),
           "Blender replayed while GGF remained incomplete.");
  }
  {
    Fixture payload_complete;
    sim::State state = exact_state();
    state.discard.push_back(sim::Card::MegaDragonite);
    state.discarded_this_turn.push_back(sim::Card::MegaDragonite);
    sim::EngineTestAccess::set_state(payload_complete.engine, std::move(state), true);
    sim::EngineTestAccess::attach_manual(payload_complete.engine);
    expect(!blender_spent(payload_complete.engine),
           "A completed current-turn payload axis spent Blender again.");
  }
  {
    Fixture no_blender;
    sim::State state = exact_state();
    state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                                 sim::Card::BrilliantBlender),
                     state.hand.end());
    sim::EngineTestAccess::set_state(no_blender.engine, std::move(state), true);
    sim::EngineTestAccess::attach_manual(no_blender.engine);
    expect(!sim::EngineTestAccess::replay_available(no_blender.engine),
           "The replay gate accepted a missing Blender.");
  }
}

void registered_witnesses_reach_the_earliest_turn() {
  struct Witness {
    const char* scenario_label;
    std::uint64_t seed;
    int expected_turn;
  };
  for (const Witness witness : {
           Witness{"strict-jit/go-first", 3698, 3},
           Witness{"matchup-flex-jit/go-first", 4984, 4},
           Witness{"matchup-flex-jit/go-second", 2166, 3},
       }) {
    const auto selected = sim::scenario_by_label(witness.scenario_label);
    const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
    expect(selected.has_value() && deck != nullptr,
           "A registered issue-1912 fixture is unavailable.");

    std::mt19937_64 rng(witness.seed);
    sim::TraceLog trace{true, {}};
    sim::Engine engine(*selected, deck->recipe, rng, &trace);
    const sim::TrialOutcome outcome = engine.run();

    expect(outcome.first_ready_turn == witness.expected_turn,
           "An issue-1912 witness missed its earliest ready turn.");
    expect(trace_contains(trace, "PLAY ITEM") &&
               trace_contains(trace, "Brilliant Blender") &&
               trace_contains(trace, "READY"),
           "An issue-1912 witness did not replay Blender before readiness.");
  }
}
}  // namespace

int main() {
  try {
    prize_inspection_k1_replays_blender_after_ggf_attachment();
    replay_gate_preserves_required_boundaries();
    registered_witnesses_reach_the_earliest_turn();
    std::cout << "Issue 1912 post-attachment Blender tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
