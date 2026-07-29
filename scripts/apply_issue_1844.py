from __future__ import annotations

import fcntl
import os
import tempfile
from contextlib import contextmanager
from pathlib import Path


@contextmanager
def locked_path(path: Path):
    lock_path = path.with_suffix(path.suffix + ".lock")
    descriptor = os.open(lock_path, os.O_CREAT | os.O_RDWR, 0o600)
    try:
        fcntl.flock(descriptor, fcntl.LOCK_EX)
        yield
    finally:
        fcntl.flock(descriptor, fcntl.LOCK_UN)
        os.close(descriptor)
        lock_path.unlink(missing_ok=True)


def atomic_write(path: Path, content: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with locked_path(path):
        with tempfile.NamedTemporaryFile(
            mode="w",
            encoding="utf-8",
            newline="",
            dir=path.parent,
            prefix=f".{path.name}.",
            delete=False,
        ) as handle:
            handle.write(content)
            temporary = Path(handle.name)
        os.replace(temporary, path)


def replace_once(path_text: str, old: str, new: str) -> None:
    path = Path(path_text)
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if text.count(old) != 1:
        raise RuntimeError(f"Expected exactly one marker in {path}: {old!r}")
    atomic_write(path, text.replace(old, new, 1))


source_path = "src/trace_engine_v2/part_013_legacy_star_override.inc"

replace_once(
    source_path,
    """    // When Powerglass is the exact end-of-turn Energy completion, Legacy Star can
    // preserve Earthen Vessel for the first later attack turn. Vessel must retain a
    // legal Basic Energy target after the seven-card discard:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/944
    const bool delayed_vessel_payload_line =
        hold_payload_outlet_for_post_powerglass_turn() && earthen_vessel_has_legal_target();
    if (recovered.size() < 2U && !item_locked() &&
        (immediate_vessel_energy_line || delayed_vessel_payload_line) &&
        count_of(state_.discard, Card::EarthenVessel) > 0) {
""",
    """    // When Powerglass is the exact end-of-turn Energy completion, Legacy Star can
    // preserve Earthen Vessel for the first later attack turn. Vessel must retain a
    // legal Basic Energy target after the seven-card discard:
    // https://api.pokemontcg.io/v2/cards/swsh12-136
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://api.pokemontcg.io/v2/cards/sv6pt5-63
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // https://github.com/FlareZ123/pokemon-sims/issues/944
    const bool post_powerglass_vessel_payload_line =
        hold_payload_outlet_for_post_powerglass_turn() && earthen_vessel_has_legal_target();

    // A spent current-turn attachment does not make a newly discarded Vessel dead
    // when K1 already proves the complete ordinary next-turn route. The selected
    // attacker must already be Active at exactly one missing Basic Energy, the
    // inspected deck must contain that Energy, and a held permitted Dragon must pay
    // Vessel while establishing the next turn's strict-JIT payload:
    // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Official Item, discard-cost, attachment, Active-position, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, dynamic DCI, strict-JIT, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
    const bool held_next_turn_vessel_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), [](const Card card) {
          return is_payload(card) && is_dragon_or_psychic(card);
        });
    const bool known_next_turn_vessel_energy =
        (grass_needed() == 1 && fire_needed() == 0 &&
         count_of(state_.deck, Card::Grass) > 0) ||
        (fire_needed() == 1 && grass_needed() == 0 &&
         count_of(state_.deck, Card::Fire) > 0);
    const bool ordinary_next_turn_vessel_payload_line =
        deck_seen_ && scenario_.dci == DciProfile::StrictJit &&
        state_.manual_energy_used && active_is_vstar() &&
        held_next_turn_vessel_payload && known_next_turn_vessel_energy &&
        earthen_vessel_has_legal_target();
    const bool delayed_vessel_payload_line =
        post_powerglass_vessel_payload_line ||
        ordinary_next_turn_vessel_payload_line;
    const bool permit_vessel_payload_cost =
        can_play_payload_this_turn() || delayed_vessel_payload_line;
    if (recovered.size() < 2U && !item_locked() &&
        (immediate_vessel_energy_line || delayed_vessel_payload_line) &&
        count_of(state_.discard, Card::EarthenVessel) > 0) {
""",
)

replace_once(
    source_path,
    """      bool found_vessel_bridge = false;
      if (recover_discard_to_hand(Card::EarthenVessel) &&
          choose_discard(can_play_payload_this_turn(), true, true, Card::EarthenVessel).has_value()) {
""",
    """      bool found_vessel_bridge = false;
      if (recover_discard_to_hand(Card::EarthenVessel) &&
          choose_discard(permit_vessel_payload_cost, true, true,
                         Card::EarthenVessel).has_value()) {
""",
)

replace_once(
    source_path,
    """          if (!recover_discard_to_hand(Card::EarthenVessel) || !recover_discard_to_hand(candidate)) continue;
          if (!choose_discard(can_play_payload_this_turn(), true, true, Card::EarthenVessel).has_value()) continue;
""",
    """          if (!recover_discard_to_hand(Card::EarthenVessel) ||
              !recover_discard_to_hand(candidate)) continue;
          if (!choose_discard(permit_vessel_payload_cost, true, true,
                              Card::EarthenVessel).has_value()) continue;
""",
)


test_content = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool use_legacy_star(Engine& engine) { return engine.use_legacy_star(); }
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

sim::Scenario scenario(const sim::LockMode lock = sim::LockMode::None) {
  return sim::Scenario{"issue-1844-legacy-star-delayed-vessel",
                       sim::DciProfile::StrictJit, lock, false, 5};
}

sim::State delayed_vessel_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 1, 1,
                              sim::Tool::None};
  state.hand = {
      sim::Card::Dragapult,
      sim::Card::MegaDragonite,
      sim::Card::Fire,
  };
  // Legacy Star removes from the back. Four Grass and an additional modeled
  // payload remain known after the seven-card discard.
  state.deck = {
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Appletun,
      sim::Card::ForestOfVitality,
      sim::Card::Grant,
      sim::Card::RegidragoV,
      sim::Card::Fire,
      sim::Card::DialgaGX,
      sim::Card::GoodraVstar,
      sim::Card::HisuianHeavyBall,
      sim::Card::QuickBall,
      sim::Card::EarthenVessel,
  };
  state.prizes = {
      sim::Card::Grass,
      sim::Card::RegidragoV,
      sim::Card::RegidragoVstar,
      sim::Card::Arven,
      sim::Card::MysteriousTreasure,
      sim::Card::Klara,
  };
  state.discard = {
      sim::Card::Channeler,
      sim::Card::WishfulBaton,
  };
  state.manual_energy_used = true;
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::TraceLog trace;
  sim::Engine engine;

  explicit Fixture(const sim::LockMode lock = sim::LockMode::None)
      : scenario_value(scenario(lock)),
        recipe(sim::deck_by_id("regidrago-shell")->recipe),
        rng(1844),
        trace{true, {}},
        engine(scenario_value, recipe, rng, &trace) {}
};

void exact_k1_route_recovers_vessel_for_next_turn() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, delayed_vessel_state());

  // The current attachment is spent, while K1 proves the next-turn Grass target
  // and a held Dragon cost. Legacy Star must preserve Vessel for T3 rather than
  // treating the Item as dead after the T2 attachment:
  // Legacy Star / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
  // Official Item, discard, attachment, and turn procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1844
  expect(sim::EngineTestAccess::use_legacy_star(fixture.engine),
         "Legacy Star did not resolve the delayed Vessel state");
  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(state.hand, sim::Card::EarthenVessel),
         "Legacy Star did not recover Earthen Vessel for T3");
  expect(contains(state.hand, sim::Card::Dragapult) ||
             contains(state.hand, sim::Card::MegaDragonite),
         "Legacy Star consumed every held next-turn payload cost");
  expect(state.vstar_power_used,
         "Legacy Star did not consume the once-per-game VSTAR Power");
}

void k0_does_not_invent_the_delayed_target() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, delayed_vessel_state(), false);
  expect(sim::EngineTestAccess::use_legacy_star(fixture.engine),
         "Legacy Star should remain legal at K0");
  expect(!contains(sim::EngineTestAccess::state(fixture.engine).hand,
                   sim::Card::EarthenVessel),
         "K0 invented the known next-turn Vessel route");
}

void item_lock_preserves_the_discarded_vessel() {
  Fixture fixture{sim::LockMode::FullItem};
  sim::EngineTestAccess::set_state(fixture.engine, delayed_vessel_state());
  expect(sim::EngineTestAccess::use_legacy_star(fixture.engine),
         "Legacy Star should remain legal through Item lock");
  expect(!contains(sim::EngineTestAccess::state(fixture.engine).hand,
                   sim::Card::EarthenVessel),
         "Legacy Star recovered an Item-locked Vessel");
}

void missing_held_payload_rejects_the_delayed_route() {
  Fixture fixture;
  sim::State state = delayed_vessel_state();
  state.hand.erase(std::remove_if(state.hand.begin(), state.hand.end(),
                                  [](const sim::Card card) {
                                    return sim::is_payload(card);
                                  }),
                   state.hand.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::use_legacy_star(fixture.engine),
         "Legacy Star did not resolve without a held payload");
  expect(!contains(sim::EngineTestAccess::state(fixture.engine).hand,
                   sim::Card::EarthenVessel),
         "Legacy Star recovered Vessel without its next-turn Dragon cost");
}

void two_missing_energy_rejects_the_delayed_route() {
  Fixture fixture;
  sim::State state = delayed_vessel_state();
  state.active->fire = 0;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::use_legacy_star(fixture.engine),
         "Legacy Star did not resolve the two-Energy-gap state");
  expect(!contains(sim::EngineTestAccess::state(fixture.engine).hand,
                   sim::Card::EarthenVessel),
         "Legacy Star recovered Vessel with two manual attachments missing");
}

void absent_known_energy_target_rejects_the_delayed_route() {
  Fixture fixture;
  sim::State state = delayed_vessel_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::Grass),
                   state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::use_legacy_star(fixture.engine),
         "Legacy Star did not resolve without a Grass target");
  expect(!contains(sim::EngineTestAccess::state(fixture.engine).hand,
                   sim::Card::EarthenVessel),
         "Legacy Star recovered a no-effect K1 Vessel");
}

void unresolved_active_position_rejects_the_delayed_route() {
  Fixture fixture;
  sim::State state = delayed_vessel_state();
  state.bench.push_back(*state.active);
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::use_legacy_star(fixture.engine),
         "Legacy Star did not resolve the unresolved Active state");
  expect(!contains(sim::EngineTestAccess::state(fixture.engine).hand,
                   sim::Card::EarthenVessel),
         "Legacy Star assumed an unproven next-turn promotion");
}

void exact_seed_reaches_turn_three_through_vessel() {
  const auto selected_scenario = sim::scenario_by_label("strict-jit/go-second");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected_scenario.has_value(), "Missing strict-JIT going-second scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{314159};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  // Source-bound regression: T2 Legacy Star preserves Vessel, then T3 Vessel
  // discards a held Dragon, searches Grass, and the manual attachment completes
  // GGF without depending on the T3 draw identity:
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv4-163
  // https://api.pokemontcg.io/v2/cards/sv6-130
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://github.com/FlareZ123/pokemon-sims/issues/1844
  expect(outcome.first_ready_turn == 3,
         "Seed 314159 did not improve to deterministic T3 readiness");
  expect(trace_contains("recovered: Earthen Vessel"),
         "Legacy Star did not preserve Earthen Vessel");
  expect(trace_contains("T3 | PLAY ITEM") && trace_contains("Earthen Vessel"),
         "Seed 314159 did not use Vessel on T3");
  expect(trace_contains("T3 | READY"),
         "Seed 314159 did not become ready on T3");
}

}  // namespace

int main() {
  try {
    exact_k1_route_recovers_vessel_for_next_turn();
    k0_does_not_invent_the_delayed_target();
    item_lock_preserves_the_discarded_vessel();
    missing_held_payload_rejects_the_delayed_route();
    two_missing_energy_rejects_the_delayed_route();
    absent_known_energy_target_rejects_the_delayed_route();
    unresolved_active_position_rejects_the_delayed_route();
    exact_seed_reaches_turn_three_through_vessel();
  } catch (const std::exception& error) {
    std::cerr << "issue-1844 delayed Vessel test failure: "
              << error.what() << '\n';
    return 1;
  }
  return 0;
}
'''

atomic_write(Path("tests/issue_1844_legacy_star_delayed_vessel_tests.cpp"), test_content)
