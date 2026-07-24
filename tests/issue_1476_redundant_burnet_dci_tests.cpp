#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state,
                        const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = deck_seen;
  }

  static std::optional<Card> choose_discard(const Engine& engine) {
    return engine.choose_discard(
        false, false, true, Card::MysteriousTreasure, false);
  }

  static bool t2_route_available(const Engine& engine) {
    return engine.issue_1476_t2_oricorio_first_route_available_after_search_started();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool trace_contains(const sim::TraceLog& trace, const std::string& text) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&text](const std::string& line) {
                       return line.find(text) != std::string::npos;
                     });
}

std::string lowercase(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](const unsigned char character) {
                   return static_cast<char>(std::tolower(character));
                 });
  return value;
}

bool trace_contains_case_insensitive(const sim::TraceLog& trace,
                                     const std::string& text) {
  const std::string expected = lowercase(text);
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&expected](const std::string& line) {
                       return lowercase(line).find(expected) != std::string::npos;
                     });
}

std::size_t trace_index(const sim::TraceLog& trace, const std::string& text) {
  const auto it = std::find_if(trace.lines.begin(), trace.lines.end(),
                               [&text](const std::string& line) {
                                 return line.find(text) != std::string::npos;
                               });
  return static_cast<std::size_t>(std::distance(trace.lines.begin(), it));
}

sim::State public_t1_route_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0}};
  state.hand = {sim::Card::RegidragoVstar,
                sim::Card::MysteriousTreasure,
                sim::Card::QuickBall,
                sim::Card::ProfessorBurnet,
                sim::Card::BrilliantBlender,
                sim::Card::LatiasEx};
  return state;
}

sim::State inspected_t1_route_state() {
  sim::State state = public_t1_route_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::MysteriousTreasure));
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::ProfessorBurnet));
  state.discard = {sim::Card::MysteriousTreasure, sim::Card::ProfessorBurnet};
  // Vital Dance takes one Grass and one Fire. Crispin says "up to 2" and may
  // search only the remaining Grass, so G,G,F is the minimum legal deck inventory:
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  state.deck = {sim::Card::Oricorio, sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::Dragapult, sim::Card::MegaDragonite,
                sim::Card::GoodraVstar};
  state.prizes = {sim::Card::EarthenVessel, sim::Card::ForestSealStone,
                  sim::Card::Lusamine, sim::Card::Arven,
                  sim::Card::Gladion, sim::Card::RegidragoV};
  return state;
}

std::optional<sim::Card> choose_from_state(
    const sim::LockMode lock, sim::State state, const std::uint64_t seed,
    const int max_turn = 2) {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "The registered shell recipe is unavailable.");
  const sim::Scenario scenario{
      "issue-1476", sim::DciProfile::StrictJit, lock, true, max_turn};
  std::mt19937_64 rng(seed);
  sim::Engine engine(scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  return sim::EngineTestAccess::choose_discard(engine);
}

void test_blender_dominates_burnet_on_complete_public_t2_route() {
  // Held Blender covers Burnet's only modeled deck-payload function. The public
  // Oricorio -> Quick Ball -> Tapu Lele-GX -> Crispin route and in-place evolution
  // make Burnet legal Treasure fuel at K0 with a T2 deadline:
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1476#issuecomment-5068202693
  expect(choose_from_state(sim::LockMode::None, public_t1_route_state(), 147601) ==
             sim::Card::ProfessorBurnet,
         "The complete public T2 route did not expose redundant Burnet as Treasure fuel.");
  expect(!choose_from_state(sim::LockMode::None, public_t1_route_state(),
                            147602, 1),
         "Burnet became discardable without a T2 action window.");
}

void test_route_boundaries_keep_burnet_protected() {
  sim::State no_blender = public_t1_route_state();
  no_blender.hand.erase(std::find(no_blender.hand.begin(), no_blender.hand.end(),
                                  sim::Card::BrilliantBlender));
  expect(!choose_from_state(sim::LockMode::None, std::move(no_blender), 147603),
         "Burnet became discardable without the dominating Blender outlet.");

  sim::State no_vstar = public_t1_route_state();
  no_vstar.hand.erase(std::find(no_vstar.hand.begin(), no_vstar.hand.end(),
                                sim::Card::RegidragoVstar));
  expect(!choose_from_state(sim::LockMode::None, std::move(no_vstar), 147604),
         "Burnet became discardable without the held VSTAR card axis.");

  sim::State no_oricorio_possible = public_t1_route_state();
  no_oricorio_possible.discard.push_back(sim::Card::Oricorio);
  expect(!choose_from_state(sim::LockMode::None, std::move(no_oricorio_possible),
                            147605),
         "Burnet became discardable without a publicly possible Energy connector.");

  sim::State crowded_bench = public_t1_route_state();
  crowded_bench.bench.push_back(sim::Pokemon{sim::Card::LatiasEx, 0});
  crowded_bench.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 0});
  crowded_bench.bench.push_back(sim::Pokemon{sim::Card::CrobatV, 0});
  expect(!choose_from_state(sim::LockMode::None, std::move(crowded_bench), 147606),
         "Burnet became discardable without two open Bench slots.");

  // Item lock prevents Treasure, Quick Ball, and Blender:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sv8-164
  expect(!choose_from_state(sim::LockMode::FullItem, public_t1_route_state(),
                            147607),
         "Item lock incorrectly exposed Burnet as discard fuel.");
}

void test_k1_t2_route_requires_every_inspected_axis() {
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(deck != nullptr, "The registered shell recipe is unavailable.");
  const sim::Scenario scenario{
      "issue-1476-k1", sim::DciProfile::StrictJit,
      sim::LockMode::None, true, 2};

  const auto route_available = [&](sim::State state, const std::uint64_t seed) {
    std::mt19937_64 rng(seed);
    sim::Engine engine(scenario, deck->recipe, rng);
    sim::EngineTestAccess::set_state(engine, std::move(state), true);
    return sim::EngineTestAccess::t2_route_available(engine);
  };

  expect(route_available(inspected_t1_route_state(), 147610),
         "The minimum legal inspected T2 route was rejected.");

  const auto rejected_without = [&](const sim::Card card, const char* message,
                                    const std::uint64_t seed) {
    sim::State state = inspected_t1_route_state();
    state.deck.erase(std::find(state.deck.begin(), state.deck.end(), card));
    expect(!route_available(std::move(state), seed), message);
  };

  rejected_without(sim::Card::Oricorio,
                   "The route survived without Oricorio in the inspected deck.",
                   147611);
  rejected_without(sim::Card::TapuLeleGX,
                   "The route survived without Tapu Lele-GX in the inspected deck.",
                   147612);
  rejected_without(sim::Card::Crispin,
                   "The route survived without Crispin in the inspected deck.",
                   147613);
  rejected_without(sim::Card::Grass,
                   "The route survived with only one inspected Grass Energy.",
                   147614);
  rejected_without(sim::Card::Fire,
                   "The route survived without inspected Fire Energy.",
                   147615);

  sim::State one_payload = inspected_t1_route_state();
  one_payload.deck.erase(std::find(one_payload.deck.begin(), one_payload.deck.end(),
                                   sim::Card::MegaDragonite));
  one_payload.deck.erase(std::find(one_payload.deck.begin(), one_payload.deck.end(),
                                   sim::Card::GoodraVstar));
  expect(!route_available(std::move(one_payload), 147616),
         "The route failed to reserve a Blender payload against the T2 draw.");
}

void test_seed_129_reaches_t2_through_oricorio_first() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(scenario.has_value() && deck != nullptr,
         "The registered issue-1476 seed fixture is unavailable.");

  std::mt19937_64 rng(129);
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const sim::TrialOutcome outcome = engine.run();

  // Burnet pays Treasure's cost at K0. The resolved Treasure establishes K1 and
  // searches Oricorio. Vital Dance, one T1 attachment, Quick Ball for Tapu Lele-GX,
  // Wonder Tag for Crispin, T2 evolution, the second manual attachment, Crispin,
  // and same-turn Blender complete strict-JIT readiness on T2:
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // K0/K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1476#issuecomment-5068202693
  expect(outcome.first_ready_turn == 2 && !outcome.setup_failed,
         "Seed 129 did not complete the legal strict-JIT T2 route.");
  expect(trace_contains(trace, "Professor Burnet (Mysterious Treasure cost)"),
         "Seed 129 did not spend redundant Burnet before inspection.");
  expect(trace_contains_case_insensitive(trace, "T1 | Vital Dance"),
         "Seed 129 did not use the inspected Oricorio Energy route.");
  expect(trace_contains(trace, "Latias ex (Quick Ball cost)"),
         "Seed 129 did not spend route-inert Latias ex after K1.");
  expect(trace_contains(trace, "T1 | WONDER TAG"),
         "Seed 129 did not bank Crispin on T1.");
  expect(trace_contains(trace, "T2 | READY"),
         "Seed 129 did not complete GGF and the same-turn payload on T2.");

  const std::size_t cost = trace_index(
      trace, "Professor Burnet (Mysterious Treasure cost)");
  const std::size_t knowledge = trace_index(
      trace, "Mysterious Treasure: deck inspected");
  expect(cost < knowledge,
         "Seed 129 used deck knowledge before paying Treasure's cost.");
}

}  // namespace

int main() {
  test_blender_dominates_burnet_on_complete_public_t2_route();
  test_route_boundaries_keep_burnet_protected();
  test_k1_t2_route_requires_every_inspected_axis();
  test_seed_129_reaches_t2_through_oricorio_first();
  return 0;
}
