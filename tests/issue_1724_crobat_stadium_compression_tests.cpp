#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
  }
  static State& state(Engine& engine) { return engine.state_; }
  static bool complete(Engine& engine) {
    return engine.complete_issue_1724_crobat_stadium_compression();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

std::size_t trace_index(const sim::TraceLog& trace, const std::string& needle) {
  const auto found = std::find_if(
      trace.lines.begin(), trace.lines.end(),
      [&needle](const std::string& line) {
        return line.find(needle) != std::string::npos;
      });
  return found == trace.lines.end()
      ? trace.lines.size()
      : static_cast<std::size_t>(std::distance(trace.lines.begin(), found));
}

sim::State exact_public_state() {
  sim::State state;
  state.turn = 5;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::ForestSealStone};
  state.bench = {
      sim::Pokemon{sim::Card::LatiasEx, 1, 0, 0, sim::Tool::None}};
  state.hand = {sim::Card::PathToPeak, sim::Card::Powerglass,
                sim::Card::ChaoticSwell, sim::Card::Lusamine,
                sim::Card::Crispin, sim::Card::TeamYellsCheer,
                sim::Card::CrobatV};
  state.deck = {sim::Card::ProfessorBurnet, sim::Card::BrilliantBlender,
                sim::Card::Dragapult, sim::Card::Grass};
  state.discard = {sim::Card::RoseannesBackup, sim::Card::QuickBall};
  state.vstar_power_used = true;
  return state;
}

bool compression_for(sim::State state,
                     const sim::LockMode locks = sim::LockMode::None,
                     const bool pineco_recipe = false,
                     const bool k1 = true) {
  sim::Scenario scenario{"issue-1724-unit", sim::DciProfile::StrictJit,
                         locks, false, 5};
  sim::DeckRecipe recipe = pineco_recipe
      ? sim::pineco_recipe()
      : sim::DeckRecipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1724};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), k1);
  return sim::EngineTestAccess::complete(engine);
}

void test_exact_public_route() {
  sim::Scenario scenario{"issue-1724-unit", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 5};
  sim::DeckRecipe recipe(sim::kDeckRecipe.begin(), sim::kDeckRecipe.end());
  std::mt19937_64 rng{1724};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(scenario, recipe, rng, &trace);
  sim::EngineTestAccess::set_state(engine, exact_public_state());

  expect(sim::EngineTestAccess::complete(engine),
         "The exact late Chaotic Swell compression route was rejected.");
  const sim::State& state = sim::EngineTestAccess::state(engine);

  // Playing Chaotic Swell leaves six cards, playing Crobat V leaves five, and
  // Dark Asset therefore draws exactly one card. K1 proves that Professor Burnet
  // and Brilliant Blender remain direct current-turn payload outlets, while the
  // selected action never depends on which card is next:
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
  // Official Stadium, Bench, and Ability procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Decision priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Future-card-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1724
  expect(state.stadium == sim::Stadium::ChaoticSwell && state.stadium_used &&
             state.dark_asset_used,
         "The route did not preserve its legal Stadium and Dark Asset state.");
  expect(trace_index(trace, "PLAY STADIUM") < trace_index(trace, "BENCH") &&
             trace_index(trace, "BENCH") < trace_index(trace, "DARK ASSET") &&
             trace_index(trace, "Drew 1 card(s)") < trace.lines.size(),
         "The exact Stadium-Crobat-Dark Asset order or draw count was lost.");
}

void test_negative_controls() {
  auto state = exact_public_state();
  state.stadium_used = true;
  expect(!compression_for(state), "The route ignored a spent Stadium action.");

  state = exact_public_state();
  state.stadium = sim::Stadium::ForestOfVitality;
  expect(!compression_for(state), "The route replaced a live Stadium.");

  state = exact_public_state();
  state.hand.push_back(sim::Card::Arven);
  expect(!compression_for(state),
         "The route drew zero cards from an eight-card hand.");

  state = exact_public_state();
  state.dark_asset_used = true;
  expect(!compression_for(state),
         "The route ignored the once-per-turn Dark Asset limit.");

  state = exact_public_state();
  state.deck.clear();
  expect(!compression_for(state), "The route ignored an empty deck.");

  state = exact_public_state();
  while (state.bench.size() < 5U) {
    state.bench.push_back(
        sim::Pokemon{sim::Card::Oricorio, 1, 0, 0, sim::Tool::None});
  }
  expect(!compression_for(state), "The route ignored a full Bench.");

  state = exact_public_state();
  state.discard.push_back(sim::Card::Dragapult);
  state.discarded_this_turn.push_back(sim::Card::Dragapult);
  expect(!compression_for(state),
         "The route ran after every setup axis was complete.");

  state = exact_public_state();
  state.hand[0] = sim::Card::TapuLeleGX;
  expect(!compression_for(state),
         "The random draw preempted a deterministic Wonder Tag payload route.");

  state = exact_public_state();
  state.deck = {sim::Card::Dragapult, sim::Card::Grass};
  expect(!compression_for(state),
         "The route invented a direct current-turn outlet absent from K1.");

  expect(!compression_for(exact_public_state(), sim::LockMode::None, true),
         "The route consumed the Pineco shell's Stadium channel.");
  expect(!compression_for(exact_public_state(),
                          sim::LockMode::FullRuleBoxAbility),
         "The route ignored a remaining Rule Box Ability lock.");
  expect(!compression_for(exact_public_state(), sim::LockMode::None,
                          false, false),
         "The route used K1-only deck counts from K0.");
}

void test_seed_1337_orders_stadium_before_dark_asset() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck =
      sim::crobat_modeling_deck_by_id("crobat2-erika-channeler");
  expect(scenario && deck, "Issue-1724 seed fixture is unavailable.");

  std::mt19937_64 rng{1337};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  engine.run();

  const std::size_t stadium = trace_index(trace, "T5 | PLAY STADIUM");
  const std::size_t crobat = trace_index(trace, "T5 | BENCH");
  const std::size_t dark_asset = trace_index(trace, "T5 | DARK ASSET");

  // Source-bound regression for the K1 seven-card hand. The chosen order is
  // Chaotic Swell, Crobat V, then Dark Asset for one current-turn draw:
  // Chaotic Swell: https://api.pokemontcg.io/v2/cards/sm12-187
  // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
  // Official Trainer and action-order procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Future-card-oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1724
  expect(stadium < crobat && crobat < dark_asset,
         "Seed 1337 did not execute Stadium before Crobat and Dark Asset.");
  expect(trace_index(trace, "T5 | DARK ASSET") < trace.lines.size() &&
             trace_index(trace, "Drew 1 card(s)") < trace.lines.size(),
         "Seed 1337 did not gain the legal Dark Asset draw.");
}
}  // namespace

int main() {
  test_exact_public_route();
  test_negative_controls();
  test_seed_1337_orders_stadium_before_dark_asset();
}
