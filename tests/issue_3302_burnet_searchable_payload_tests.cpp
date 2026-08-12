#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <array>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }

  static bool route_available(const Engine& engine) {
    return engine.late_steven_burnet_route_available();
  }
};
}  // namespace sim

namespace {

void expect(const bool value, const char* message) {
  if (!value) throw std::runtime_error(message);
}

sim::State route_state(const sim::Card deck_payload) {
  sim::State state;
  state.turn = 3;
  // Pokemon aggregate contract: https://github.com/FlareZ123/pokemon-sims/blob/main/src/trace_engine_v2/core/game_state_types.inc
  // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
  state.active = sim::Pokemon{sim::Card::RegidragoV, 2, 1, 1};
  state.hand = {sim::Card::StevensResolve, sim::Card::ProfessorBurnet};
  state.deck = {sim::Card::RegidragoVstar, sim::Card::Grass, deck_payload};
  return state;
}

bool available(sim::State state, const bool deck_seen = true) {
  std::mt19937_64 rng(3302);
  sim::Engine engine(
      sim::Scenario{"issue-3302", sim::DciProfile::StrictJit,
                    sim::LockMode::None, true, 5},
      sim::baseline_recipe(), rng);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return sim::EngineTestAccess::route_available(engine);
}

void test_each_searchable_deck_payload_is_sufficient() {
  // Professor Burnet searches the deck for up to two cards and discards the cards
  // it found, so every modeled Apex Dragon payload is sufficient when K1 proves it
  // remains physically searchable in the deck.
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3302
  constexpr std::array<sim::Card, 5> payloads{
      sim::Card::Appletun, sim::Card::MegaDragonite, sim::Card::Dragapult,
      sim::Card::GoodraVstar, sim::Card::DialgaGX};
  for (const sim::Card payload : payloads) {
    expect(available(route_state(payload)),
           "searchable K1 deck payload did not admit the Steven-Burnet route");
  }
}

void test_hand_only_payload_is_insufficient() {
  // A Dragon already in hand cannot be selected by Burnet's deck-search effect.
  // Keeping only a hand payload must therefore reject the projected route.
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Advanced search procedure: https://github.com/FlareZ123/pokemon-sims/blob/main/EN_advanced_manual-2025-transcription-structured.md
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3302
  sim::State state = route_state(sim::Card::MegaDragonite);
  const auto payload = std::find(state.deck.begin(), state.deck.end(),
                                 sim::Card::MegaDragonite);
  state.deck.erase(payload);
  state.hand.push_back(sim::Card::MegaDragonite);
  expect(!available(std::move(state)),
         "hand-only payload falsely satisfied Professor Burnet's deck search");
}

void test_k0_still_rejects_projected_search() {
  // The route is deterministic only after K1 establishes the remaining deck and
  // Prize identities. A physically present payload does not bypass true K0.
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/3302
  expect(!available(route_state(sim::Card::Dragapult), false),
         "true K0 admitted the deterministic Steven-Burnet route");
}

}  // namespace

int main() {
  try {
    test_each_searchable_deck_payload_is_sufficient();
    test_hand_only_payload_is_insufficient();
    test_k0_still_rejects_projected_search();
    std::cout << "Issue 3302 Burnet searchable-payload tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
