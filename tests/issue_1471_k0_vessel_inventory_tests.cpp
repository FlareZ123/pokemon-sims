#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <iostream>
#include <stdexcept>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen,
                        const bool prizes_revealed = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }

  static bool hold_vessel(const Engine& engine) {
    return engine.hold_earthen_vessel_for_next_turn_latias_jit();
  }

  static bool play_vessel(Engine& engine) {
    return engine.play_earthen_vessel(true);
  }

  static const State& state(const Engine& engine) { return engine.state_; }
};

}  // namespace sim

namespace {

int count_card(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::State vessel_window_state(const bool hidden_fire_in_deck) {
  sim::State state;
  state.turn = 2;
  state.manual_energy_used = true;
  state.active = sim::Pokemon{sim::Card::Oricorio, 1};

  sim::Pokemon setup_regi{sim::Card::RegidragoVstar, 1};
  setup_regi.grass = 1;
  setup_regi.fire = 1;
  state.bench = {
      sim::Pokemon{sim::Card::LatiasEx, 1},
      setup_regi,
  };
  state.hand = {
      sim::Card::EarthenVessel,
      sim::Card::Dragapult,
  };
  state.deck = {sim::Card::Grass};
  state.prizes = {sim::Card::QuickBall};
  if (hidden_fire_in_deck) {
    state.deck.push_back(sim::Card::Fire);
    state.prizes.push_back(sim::Card::MysteriousTreasure);
  } else {
    state.deck.push_back(sim::Card::MysteriousTreasure);
    state.prizes.push_back(sim::Card::Fire);
  }
  return state;
}

sim::Engine make_engine(std::mt19937_64& rng, sim::State state,
                        const bool deck_seen,
                        const bool prizes_revealed = false) {
  const sim::Scenario scenario{"issue-1471", sim::DciProfile::StrictJit,
                               sim::LockMode::None, true, 3};
  const sim::DeckRecipe recipe{sim::baseline_recipe()};
  sim::Engine engine(scenario, recipe, rng);
  sim::EngineTestAccess::set_state(
      engine, std::move(state), deck_seen, prizes_revealed);
  return engine;
}

void test_paired_k0_hidden_placements_make_same_safe_hold() {
  std::mt19937_64 rng_a{1471};
  std::mt19937_64 rng_b{1472};
  sim::Engine energy_in_deck =
      make_engine(rng_a, vessel_window_state(true), false);
  sim::Engine energy_prized =
      make_engine(rng_b, vessel_window_state(false), false);

  // Before a legal inspection, the player cannot distinguish whether the hidden
  // Fire is in the deck or the Prize cards. Both identical public states must use
  // fixed-list/public-zone possibility and preserve Vessel plus the Dragon:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv8-130
  // Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
  // Core deck-search and Prize procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1471
  if (!sim::EngineTestAccess::hold_vessel(energy_in_deck) ||
      !sim::EngineTestAccess::hold_vessel(energy_prized)) {
    throw std::runtime_error(
        "Paired K0 hidden placements must both preserve Vessel and the payload.");
  }
}

void test_k0_play_attempt_preserves_vessel_and_payload() {
  std::mt19937_64 rng{1473};
  sim::Engine engine = make_engine(rng, vessel_window_state(false), false);

  // The safe K0 decision is a hold. It must occur before Earthen Vessel can pay
  // the irreversible Dragon discard cost and legally inspect the deck:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core cost-before-search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K0 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k0-before-a-legal-inspection
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1471
  if (sim::EngineTestAccess::play_vessel(engine)) {
    throw std::runtime_error("K0 Vessel should be held for the next strict-JIT turn.");
  }
  const sim::State& after = sim::EngineTestAccess::state(engine);
  if (count_card(after.hand, sim::Card::EarthenVessel) != 1 ||
      count_card(after.hand, sim::Card::Dragapult) != 1 ||
      count_card(after.discard, sim::Card::Dragapult) != 0) {
    throw std::runtime_error(
        "The K0 hold must preserve both Earthen Vessel and the Dragon payload.");
  }
}

void test_k1_uses_exact_sufficient_deck_inventory() {
  std::mt19937_64 rng{1474};
  sim::Engine engine = make_engine(rng, vessel_window_state(true), true);

  // After a legal deck inspection, exact remaining-deck inventory is known. One
  // Grass plus a second Basic Energy keeps the next-turn Vessel route live:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k1-after-a-legal-deck-or-prize-inspection
  // Existing route: https://github.com/FlareZ123/pokemon-sims/issues/1009
  // Confirmed bug boundary: https://github.com/FlareZ123/pokemon-sims/issues/1471
  if (!sim::EngineTestAccess::hold_vessel(engine)) {
    throw std::runtime_error(
        "K1 sufficient Grass and Basic Energy inventory must preserve the Vessel route.");
  }
}

void test_k1_rejects_exact_insufficient_deck_inventory() {
  std::mt19937_64 rng{1475};
  sim::Engine engine = make_engine(rng, vessel_window_state(false), true);

  // K1 proves that only one Basic Energy remains in the deck. The next draw could
  // remove the sole legal Grass target, so the two-Energy hold requirement fails:
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Core deck-search procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k1-after-a-legal-deck-or-prize-inspection
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1471
  if (sim::EngineTestAccess::hold_vessel(engine)) {
    throw std::runtime_error(
        "K1 insufficient remaining Basic Energy must not preserve the Vessel route.");
  }
}

void test_prize_reveal_uses_exact_complementary_deck_inventory() {
  std::mt19937_64 rng{1476};
  sim::Engine engine =
      make_engine(rng, vessel_window_state(false), false, true);

  // A legal full Prize reveal also establishes K1 through the fixed recipe and
  // public zones, so the exact one-Energy deck must fail the hold predicate:
  // Core Prize procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#k1-after-a-legal-deck-or-prize-inspection
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1471
  if (sim::EngineTestAccess::hold_vessel(engine)) {
    throw std::runtime_error(
        "Prize-reveal K1 must use the exact complementary deck inventory.");
  }
}

}  // namespace

int main() {
  try {
    test_paired_k0_hidden_placements_make_same_safe_hold();
    test_k0_play_attempt_preserves_vessel_and_payload();
    test_k1_uses_exact_sufficient_deck_inventory();
    test_k1_rejects_exact_insufficient_deck_inventory();
    test_prize_reveal_uses_exact_complementary_deck_inventory();
    std::cout << "Issue 1471 K0 Vessel inventory tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
