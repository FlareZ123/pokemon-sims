#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
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
  static std::optional<Card> quick_ball_cost(const Engine& engine) {
    return engine.issue_1869_quick_ball_burnet_cost();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& expected) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return line.find(expected) != std::string::npos;
                     });
}

sim::State exact_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::Powerglass};
  state.hand = {sim::Card::QuickBall, sim::Card::ProfessorTuro,
                sim::Card::ErikasInvitation, sim::Card::Crispin,
                sim::Card::Fire};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::ProfessorBurnet,
                sim::Card::MegaDragonite, sim::Card::Dragapult,
                sim::Card::Grass};
  return state;
}

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1869", sim::DciProfile::StrictJit, lock, false, 5};
}

sim::Engine make_engine(const sim::Scenario& selected, std::mt19937_64& rng,
                        sim::TraceLog* trace = nullptr) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  if (deck == nullptr) throw std::runtime_error("Registered shell is unavailable.");
  return sim::Engine(selected, deck->recipe, rng, trace);
}

void test_deck_search_and_prize_inspection_k1() {
  std::mt19937_64 rng{1869};
  sim::Engine deck_k1 = make_engine(scenario(), rng);
  sim::EngineTestAccess::set_state(deck_k1, exact_state(), true, false);

  // Active GGF makes Turo route-replaced. Quick Ball may spend it, search Tapu,
  // and Wonder Tag may find Burnet for the current-turn Dragon payload. K1 may be
  // established through either a deck search or a complete legal Prize inspection:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1869
  expect(sim::EngineTestAccess::quick_ball_cost(deck_k1) ==
             sim::Card::ProfessorTuro,
         "Deck-search K1 did not admit the Burnet connector cost.");

  sim::Engine prize_k1 = make_engine(scenario(), rng);
  sim::EngineTestAccess::set_state(prize_k1, exact_state(), false, true);
  expect(sim::EngineTestAccess::quick_ball_cost(prize_k1) ==
             sim::Card::ProfessorTuro,
         "Prize-inspection K1 did not admit the Burnet connector cost.");

  sim::Engine k0 = make_engine(scenario(), rng);
  sim::EngineTestAccess::set_state(k0, exact_state(), false, false);
  const auto k0_cost = sim::EngineTestAccess::quick_ball_cost(k0);
  expect(k0_cost != sim::Card::ProfessorTuro &&
             k0_cost != sim::Card::ErikasInvitation,
         "True K0 admitted the public-K1-only Burnet connector cost.");
}

void test_legality_and_resource_boundaries() {
  std::mt19937_64 rng{1870};
  const auto rejected = [&rng](sim::State state, sim::Scenario selected,
                               const char* message) {
    sim::Engine candidate = make_engine(selected, rng);
    sim::EngineTestAccess::set_state(candidate, std::move(state));
    const auto cost = sim::EngineTestAccess::quick_ball_cost(candidate);
    expect(cost != sim::Card::ProfessorTuro &&
               cost != sim::Card::ErikasInvitation,
           message);
  };

  rejected(exact_state(), scenario(sim::LockMode::FullItem),
           "The cost bypassed Item lock.");
  rejected(exact_state(),
           sim::Scenario{"supporter-lock", sim::DciProfile::StrictJit,
                         sim::LockMode::FullSupporter, false, 5},
           "The cost bypassed Supporter lock.");
  {
    sim::State state = exact_state();
    state.bench.resize(5, sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                                      sim::Tool::None});
    rejected(std::move(state), scenario(),
             "The cost ignored a full Bench.");
  }
  for (const sim::Card missing : {sim::Card::TapuLeleGX,
                                  sim::Card::ProfessorBurnet}) {
    sim::State state = exact_state();
    state.deck.erase(std::remove(state.deck.begin(), state.deck.end(), missing),
                     state.deck.end());
    rejected(std::move(state), scenario(),
             "The cost ignored a missing connector target.");
  }
  {
    sim::State state = exact_state();
    state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(),
                                    sim::is_payload),
                     state.deck.end());
    rejected(std::move(state), scenario(),
             "The cost ignored the absence of every deck payload.");
  }
}

void test_registered_seed_108_reaches_t3() {
  const auto selected = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected.has_value() && deck != nullptr,
         "The registered issue-1869 fixture is unavailable.");

  std::mt19937_64 rng{108};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*selected, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  expect(outcome.first_ready_turn == 3,
         "Registered seed 108 did not improve from T4 to T3.");
  expect(trace_contains(trace, "T3 | PLAY ITEM") &&
             trace_contains(trace, "T3 | WONDER TAG") &&
             trace_contains(trace, "T3 | PLAY SUPPORTER") &&
             trace_contains(trace, "T3 | READY"),
         "Registered seed 108 did not execute the T3 Burnet connector.");
}
}  // namespace

int main() {
  test_deck_search_and_prize_inspection_k1();
  test_legality_and_resource_boundaries();
  test_registered_seed_108_reaches_t3();
  return 0;
}
