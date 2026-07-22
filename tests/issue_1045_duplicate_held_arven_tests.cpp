#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void install_k1_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = true;
    engine.prizes_revealed_ = true;
  }

  static bool duplicate_arven_is_redundant(const Engine& engine) {
    return engine.wonder_tag_duplicate_held_arven_has_no_marginal_route();
  }

  static bool bench_tapu(Engine& engine) {
    return engine.bench_tapu_if_useful();
  }

  static void run_turn(Engine& engine) {
    engine.run_turn();
  }
};

}  // namespace sim

namespace {

void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

void remove_required(std::vector<sim::Card>& cards, const sim::Card card) {
  const auto found = std::find(cards.begin(), cards.end(), card);
  if (found == cards.end()) throw std::runtime_error("Exact-state card is unavailable");
  cards.erase(found);
}

std::vector<sim::Card> expanded_baseline() {
  std::vector<sim::Card> cards;
  for (const auto& [card, copies] : sim::baseline_recipe()) {
    cards.insert(cards.end(), static_cast<std::size_t>(copies), card);
  }
  return cards;
}

sim::State exact_issue_state() {
  sim::State state;
  state.turn = 3;
  state.deck = expanded_baseline();

  const auto take = [&state](const sim::Card card) {
    remove_required(state.deck, card);
  };

  take(sim::Card::DialgaGX);
  state.active = sim::Pokemon{sim::Card::DialgaGX, 0};

  take(sim::Card::TapuLeleGX);
  state.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});

  take(sim::Card::RegidragoVstar);
  take(sim::Card::RegidragoV);
  state.bench.push_back(
      sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1});

  take(sim::Card::RegidragoV);
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 2});

  for (const sim::Card card : {
           sim::Card::Channeler,
           sim::Card::Arven,
           sim::Card::Dipplin,
           sim::Card::Fire,
           sim::Card::TapuLeleGX,
       }) {
    take(card);
    state.hand.push_back(card);
  }

  take(sim::Card::BrilliantBlender);
  state.discard.push_back(sim::Card::BrilliantBlender);
  take(sim::Card::MegaDragonite);
  state.discard.push_back(sim::Card::MegaDragonite);
  state.discarded_this_turn.push_back(sim::Card::MegaDragonite);

  for (const sim::Card card : {
           sim::Card::ErikasInvitation,
           sim::Card::Guzma,
           sim::Card::Lusamine,
           sim::Card::TeamYellsCheer,
           sim::Card::RoseannesBackup,
           sim::Card::PathToPeak,
       }) {
    take(card);
    state.prizes.push_back(card);
  }

  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario,
                        const sim::DeckRecipe& recipe,
                        std::mt19937_64& rng,
                        sim::TraceLog* trace,
                        sim::State state) {
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::install_k1_state(engine, std::move(state));
  return engine;
}

void test_exact_k1_route_holds_second_tapu() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  expect(scenario.has_value(), "Missing matchup-flex-jit/go-second scenario");
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1045};
  sim::TraceLog trace{true, {}};
  sim::Engine engine = make_engine(*scenario, recipe, rng, &trace, exact_issue_state());

  expect(sim::EngineTestAccess::duplicate_arven_is_redundant(engine),
         "The exact K1 duplicate-Arven route was not recognized");
  expect(!sim::EngineTestAccess::bench_tapu(engine),
         "The exact K1 state still Benched the second Tapu Lele-GX");
  expect(std::count(engine.state().hand.begin(), engine.state().hand.end(),
                    sim::Card::Arven) == 1,
         "Holding Tapu must preserve the single held Arven");

  sim::EngineTestAccess::run_turn(engine);
  const sim::State& state = engine.state();
  const bool ready = state.active &&
      state.active->card == sim::Card::RegidragoVstar &&
      state.active->grass >= 2 && state.active->fire >= 1;
  const bool hold_trace = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("HOLD TAPU LELE-GX") != std::string::npos &&
               line.find("held Arven") != std::string::npos;
      });
  const bool duplicate_search = std::any_of(
      trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
        return line.find("WONDER TAG") != std::string::npos &&
               line.find("Arven") != std::string::npos;
      });

  // The held Arven searches the same Earthen Vessel and Forest Seal Stone package.
  // Earthen Vessel supplies the final Grass for the unused manual attachment, while
  // the Tool route completes the observable Active-position axis. A second Arven
  // cannot be played in the same turn and remains unused after readiness:
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // One Supporter and one manual attachment: https://www.pokemon.com/us/pokemon-tcg/rules
  // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed refined bug: https://github.com/FlareZ123/pokemon-sims/issues/1045
  expect(ready && state.supporter_used && hold_trace && !duplicate_search,
         "The held-Arven route did not complete cleanly without duplicate Wonder Tag");
}

void test_incomplete_route_preserves_second_copy_value() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  expect(scenario.has_value(), "Missing matchup-flex-jit/go-second scenario");
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1046};
  sim::State state = exact_issue_state();
  remove_required(state.deck, sim::Card::ForestSealStone);
  state.prizes.push_back(sim::Card::ForestSealStone);
  sim::Engine engine = make_engine(*scenario, recipe, rng, nullptr, std::move(state));

  // Without the Tool bridge, the held Arven plus Vessel does not complete every
  // observable setup axis. The narrow projection must retain Tapu and any separate
  // Supporter route instead of treating all duplicate Arven states as equivalent:
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://api.pokemontcg.io/v2/cards/swsh12-156
  // https://github.com/FlareZ123/pokemon-sims/issues/1045
  expect(!sim::EngineTestAccess::duplicate_arven_is_redundant(engine),
         "An incomplete Arven route incorrectly suppressed second-copy value");
}

void test_unpayable_vessel_preserves_connector() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  expect(scenario.has_value(), "Missing matchup-flex-jit/go-second scenario");
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1047};
  sim::State state = exact_issue_state();
  for (const sim::Card card : {sim::Card::Channeler, sim::Card::Dipplin}) {
    remove_required(state.hand, card);
    state.prizes.push_back(card);
  }
  sim::Engine engine = make_engine(*scenario, recipe, rng, nullptr, std::move(state));

  // Earthen Vessel requires another card from hand. Protected singleton resources
  // cannot advertise a fictional duplicate-Arven route when that cost is unpayable:
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // https://github.com/FlareZ123/pokemon-sims/issues/1045
  expect(!sim::EngineTestAccess::duplicate_arven_is_redundant(engine),
         "An unpayable Vessel route incorrectly suppressed Wonder Tag");
}

void test_missing_held_arven_never_suppresses_wonder_tag() {
  const auto scenario = sim::scenario_by_label("matchup-flex-jit/go-second");
  expect(scenario.has_value(), "Missing matchup-flex-jit/go-second scenario");
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  std::mt19937_64 rng{1048};
  sim::State state = exact_issue_state();
  remove_required(state.hand, sim::Card::Arven);
  state.deck.push_back(sim::Card::Arven);
  sim::Engine engine = make_engine(*scenario, recipe, rng, nullptr, std::move(state));

  // Wonder Tag remains a legal Supporter connector when no Arven is already held:
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sv1-166
  // https://github.com/FlareZ123/pokemon-sims/issues/1045
  expect(!sim::EngineTestAccess::duplicate_arven_is_redundant(engine),
         "The fix suppressed a unique Wonder Tag to Arven route");
}

}  // namespace

int main() {
  test_exact_k1_route_holds_second_tapu();
  test_incomplete_route_preserves_second_copy_value();
  test_unpayable_vessel_preserves_connector();
  test_missing_held_arven_never_suppresses_wonder_tag();
  return 0;
}
