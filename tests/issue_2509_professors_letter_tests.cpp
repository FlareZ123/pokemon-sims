#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <numeric>
#include <random>
#include <stdexcept>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool play_professors_letter(Engine& engine) {
    return engine.play_professors_letter();
  }
  static bool deck_seen(const Engine& engine) { return engine.deck_seen_; }
  static void set_deck_seen(Engine& engine, const bool value) {
    engine.deck_seen_ = value;
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count_card(const std::vector<sim::Card>& zone, const sim::Card card) {
  return static_cast<int>(std::count(zone.begin(), zone.end(), card));
}

sim::State live_letter_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1};
  state.active->fire = 1;
  state.hand = {sim::Card::ProfessorsLetter, sim::Card::MegaDragonite};
  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire};
  return state;
}

void test_professors_letter_searches_two_basic_energy_without_discard_cost() {
  sim::Scenario scenario{"issue-2509", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.push_back({sim::Card::ProfessorsLetter, 1});
  std::mt19937_64 rng{2509};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(engine, live_letter_state());

  expect(sim::EngineTestAccess::play_professors_letter(engine),
         "Professor's Letter did not resolve with live Basic Energy targets");
  const sim::State& state = sim::EngineTestAccess::state(engine);

  // Professor's Letter searches up to two Basic Energy and has no discard cost:
  // https://api.pokemontcg.io/v2/cards/xy1-123
  // https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy1/123/
  // Core Item/search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Enhancement: https://github.com/FlareZ123/pokemon-sims/issues/2509
  expect(count_card(state.hand, sim::Card::Grass) == 2,
         "Professor's Letter did not take both legal same-type Basic Energy");
  expect(count_card(state.hand, sim::Card::MegaDragonite) == 1,
         "Professor's Letter incorrectly discarded another hand card");
  expect(count_card(state.discard, sim::Card::ProfessorsLetter) == 1,
         "Professor's Letter did not enter discard after resolution");
  expect(count_card(state.discard, sim::Card::MegaDragonite) == 0,
         "Professor's Letter invented Earthen Vessel's discard cost");
  expect(sim::EngineTestAccess::deck_seen(engine),
         "Professor's Letter deck search did not establish K1");
}

void test_professors_letter_respects_item_lock_and_energy_value() {
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.push_back({sim::Card::ProfessorsLetter, 1});

  sim::Scenario locked{"issue-2509-lock", sim::DciProfile::StrictJit,
                       sim::LockMode::FullItem, false, 4};
  std::mt19937_64 locked_rng{2509};
  sim::Engine locked_engine(locked, recipe, locked_rng);
  sim::EngineTestAccess::set_state(locked_engine, live_letter_state());
  expect(!sim::EngineTestAccess::play_professors_letter(locked_engine),
         "Professor's Letter ignored Item lock");

  sim::Scenario open{"issue-2509-complete", sim::DciProfile::StrictJit,
                     sim::LockMode::None, false, 4};
  std::mt19937_64 open_rng{2510};
  sim::Engine complete_engine(open, recipe, open_rng);
  sim::State complete = live_letter_state();
  complete.active->grass = 2;
  sim::EngineTestAccess::set_state(complete_engine, complete);
  expect(!sim::EngineTestAccess::play_professors_letter(complete_engine),
         "Professor's Letter was burned after the Energy axis was complete");
}

void test_professors_letter_k1_rejects_exhausted_needed_energy() {
  sim::Scenario scenario{"issue-2509-k1", sim::DciProfile::StrictJit,
                         sim::LockMode::None, false, 4};
  sim::DeckRecipe recipe = sim::baseline_recipe();
  recipe.push_back({sim::Card::ProfessorsLetter, 1});
  std::mt19937_64 rng{2511};
  sim::Engine engine(scenario, recipe, rng);
  sim::State state = live_letter_state();
  state.deck = {sim::Card::Fire};
  sim::EngineTestAccess::set_state(engine, state);
  sim::EngineTestAccess::set_deck_seen(engine, true);
  expect(!sim::EngineTestAccess::play_professors_letter(engine),
         "K1 played Professor's Letter after all needed Grass was absent");
}

void test_professors_letter_is_not_a_registered_deck_card() {
  // The user-requested comparison is an unregistered paper-Expanded derivative:
  // https://api.pokemontcg.io/v2/cards/xy1-123
  // https://github.com/FlareZ123/pokemon-sims/issues/2509
  expect(sim::deck_registry().size() == 2U,
         "Professor's Letter changed the named-deck registry");
  for (const sim::NamedDeck& deck : sim::deck_registry()) {
    const int copies = std::accumulate(
        deck.recipe.begin(), deck.recipe.end(), 0,
        [](const int total, const auto& entry) {
          return total +
              (entry.first == sim::Card::ProfessorsLetter ? entry.second : 0);
        });
    expect(copies == 0, "Professor's Letter leaked into a registered recipe");
  }
}
}  // namespace

int main() {
  test_professors_letter_searches_two_basic_energy_without_discard_cost();
  test_professors_letter_respects_item_lock_and_energy_value();
  test_professors_letter_k1_rejects_exhausted_needed_energy();
  test_professors_letter_is_not_a_registered_deck_card();
}
