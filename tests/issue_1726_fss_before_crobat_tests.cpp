#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {
struct EngineTestAccess {
  static State& state(Engine& engine) { return engine.state_; }
  static bool available(Engine& engine) {
    return engine.fss_crobat_compression_available();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

std::size_t trace_index(const sim::TraceLog& trace, const std::string& needle) {
  const auto found = std::find_if(trace.lines.begin(), trace.lines.end(),
                                  [&needle](const std::string& line) {
                                    return line.find(needle) != std::string::npos;
                                  });
  if (found == trace.lines.end()) throw std::runtime_error("Expected trace line missing.");
  return static_cast<std::size_t>(std::distance(trace.lines.begin(), found));
}

sim::Engine make_engine(sim::Scenario& scenario, sim::DeckRecipe& recipe,
                        std::mt19937_64& rng) {
  return sim::Engine(scenario, recipe, rng);
}

void install_state(sim::Engine& engine) {
  auto& state = sim::EngineTestAccess::state(engine);
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 0, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::MegaDragonite, sim::Card::GoodraVstar,
                sim::Card::TeamYellsCheer, sim::Card::CrobatV,
                sim::Card::ForestSealStone};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass,
                sim::Card::Fire};
}

bool available_with(const sim::LockMode lock, const auto& mutate) {
  sim::Scenario scenario{"issue-1726-unit", sim::DciProfile::StrictJit,
                         lock, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1726};
  sim::Engine engine = make_engine(scenario, recipe, rng);
  install_state(engine);
  mutate(sim::EngineTestAccess::state(engine));
  return sim::EngineTestAccess::available(engine);
}

void test_public_negative_controls() {
  expect(available_with(sim::LockMode::None, [](sim::State&) {}),
         "The exact public Forest Seal Stone compression was rejected.");
  expect(!available_with(sim::LockMode::FullRuleBoxAbility, [](sim::State&) {}),
         "The route ignored Rule Box Ability lock.");
  expect(!available_with(sim::LockMode::None, [](sim::State& state) {
           state.bench.clear();
         }), "The route invented a Pokémon V holder.");
  expect(!available_with(sim::LockMode::None, [](sim::State& state) {
           state.bench.front().tool = sim::Tool::Powerglass;
         }), "The route ignored an occupied Tool slot.");
  expect(!available_with(sim::LockMode::None, [](sim::State& state) {
           state.vstar_power_used = true;
         }), "The route attached a dead Forest Seal Stone.");
  expect(!available_with(sim::LockMode::None, [](sim::State& state) {
           state.deck.clear();
         }), "The route used Dark Asset with an empty deck.");
  expect(!available_with(sim::LockMode::None, [](sim::State& state) {
           state.hand.insert(state.hand.end(), {sim::Card::Lusamine,
                                                sim::Card::Channeler,
                                                sim::Card::RoseannesBackup});
         }), "The route accepted a hand too large for one-card compression.");
  expect(!available_with(sim::LockMode::None, [](sim::State& state) {
           state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                                       sim::Tool::None};
           state.bench.clear();
           state.discard.push_back(sim::Card::MegaDragonite);
           state.discarded_this_turn.push_back(sim::Card::MegaDragonite);
         }), "The route drew after every setup axis was complete.");
  expect(!available_with(sim::LockMode::None, [](sim::State& state) {
           state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                                       sim::Tool::None};
           state.bench.clear();
           state.hand.push_back(sim::Card::Powerglass);
           state.discard.push_back(sim::Card::Grass);
         }), "The route displaced a live Powerglass attachment.");
}

void test_seed_7_attaches_fss_before_dark_asset() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::crobat_modeling_deck_by_id("crobat1-erika");
  expect(scenario.has_value() && deck != nullptr,
         "Issue-1726 seed fixture unavailable.");
  std::mt19937_64 rng{7};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  engine.run();

  // Attaching Forest Seal Stone before Crobat V leaves three cards in hand after
  // Crobat is Benched, so Dark Asset draws three cards to six:
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  // Official Tool, Bench, Ability, and action-order procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Pokémon Tool errata: https://www.pokemon.com/us/pokemon-news/2023-pokemon-tcg-standard-format-rotation-and-pokemon-tool-errata
  // Repository connector and future-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1726
  const std::size_t regi = trace_index(trace, "T1 | BENCH | rules: R-GAME-BENCH | Regidrago V");
  const std::size_t stone = trace_index(trace, "T1 | ATTACH TOOL");
  const std::size_t crobat = trace_index(trace, "T1 | BENCH | rules: R-GAME-BENCH | Crobat V");
  const std::size_t dark_asset = trace_index(trace, "T1 | DARK ASSET");
  expect(regi < stone && stone < crobat && crobat < dark_asset,
         "Seed 7 did not attach Forest Seal Stone between Regidrago V and Crobat V.");
  expect(std::any_of(trace.lines.begin(), trace.lines.end(), [](const std::string& line) {
           return line.find("Drew 3 card(s) until the hand contained six cards") !=
               std::string::npos;
         }), "Seed 7 did not gain the third Dark Asset draw.");
}
}  // namespace

int main() {
  test_public_negative_controls();
  test_seed_7_attaches_fss_before_dark_asset();
}
