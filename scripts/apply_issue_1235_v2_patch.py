from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    descriptor, temporary_name = tempfile.mkstemp(prefix=f".{path.name}.", dir=path.parent)
    try:
        with os.fdopen(descriptor, "w", encoding="utf-8", newline="\n") as handle:
            handle.write(text)
            handle.flush()
            os.fsync(handle.fileno())
        os.replace(temporary_name, path)
    except BaseException:
        try:
            os.unlink(temporary_name)
        except FileNotFoundError:
            pass
        raise


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if text.count(old) != 1:
        raise RuntimeError(f"Expected one source anchor in {path}")
    atomic_write(path, text.replace(old, new, 1))


def main() -> None:
    regidrago = ROOT / "src/regidrago_sim.cpp"
    replace_once(
        regidrago,
        '''#define play_mysterious_treasure play_mysterious_treasure_issue1209_original
#include "trace_engine_v2/part_issue_1167_treasure_vessel_override.inc"
#undef play_mysterious_treasure
#include "trace_engine_v2/part_issue_1209_treasure_tapu_crispin_override.inc"
#include "trace_engine_v2/part_issue_1118_secret_box.inc"
''',
        '''#define play_mysterious_treasure play_mysterious_treasure_issue1209_original
#include "trace_engine_v2/part_issue_1167_treasure_vessel_override.inc"
#undef play_mysterious_treasure
#define play_quick_ball play_quick_ball_issue1235_original
#define play_mysterious_treasure play_mysterious_treasure_issue1235_original
#define choose_supporter choose_supporter_issue1235_original
#include "trace_engine_v2/part_issue_1209_treasure_tapu_crispin_override.inc"
#undef choose_supporter
#undef play_mysterious_treasure
#undef play_quick_ball
#include "trace_engine_v2/part_issue_1235_oricorio_treasure_tapu_override.inc"
#include "trace_engine_v2/part_issue_1118_secret_box.inc"
''',
    )

    override = ROOT / "src/trace_engine_v2/part_issue_1235_oricorio_treasure_tapu_override.inc"
    atomic_write(override, r'''  bool issue_1235_t1_quick_ball_connector_state() const {
    if (scenario_.dci != DciProfile::StrictJit || state_.turn != 1 ||
        scenario_.going_first || scenario_.locks != LockMode::None ||
        state_.manual_energy_used || !state_.active ||
        state_.active->card != Card::RegidragoV ||
        state_.active->grass != 0 || state_.active->fire != 0 ||
        bench_space() < 2 || hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::RegidragoV) == 0 ||
        hand_count(Card::TapuLeleGX) > 0 || in_play(Card::TapuLeleGX) ||
        hand_count(Card::Oricorio) > 0 || in_play(Card::Oricorio) ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }
    const bool prior_turn_benched_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn;
        });
    const auto cost = choose_discard(false, true, true, Card::QuickBall);
    return prior_turn_benched_regi && cost == Card::RegidragoV &&
        might_be_unseen(Card::Oricorio) && might_be_unseen(Card::TapuLeleGX);
  }

  bool issue_1235_oricorio_route_available_after_quick_ball_search() const {
    if (!deck_seen_ || hand_count(Card::MysteriousTreasure) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }
    // Vital Dance must leave one Grass and one Fire for the following turn's
    // Crispin search. Mysterious Treasure then discards the held Dragon payload,
    // searches Tapu Lele-GX, and Wonder Tag obtains Crispin. This comparison is
    // made only after Quick Ball legally exposes the deck, so it does not read
    // future draws or unrevealed Prize identities:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Ability, Supporter, Bench, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1 and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1235
    return deck_count_after_search_started(Card::Oricorio) > 0 &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::Fire) >= 2;
  }

  bool play_quick_ball(const bool permit_payload) {
    if (!issue_1235_t1_quick_ball_connector_state()) {
      return play_quick_ball_issue1235_original(permit_payload);
    }

    remove_one(state_.hand, Card::QuickBall);
    state_.discard.push_back(Card::QuickBall);
    discard_from_hand(Card::RegidragoV, "Quick Ball cost", "R-QB-01; P-DCI-01");
    record_deck_search_knowledge("Quick Ball");

    Card selected = Card::TapuLeleGX;
    if (issue_1235_oricorio_route_available_after_quick_ball_search()) {
      selected = Card::Oricorio;
    } else if (deck_count_after_search_started(Card::TapuLeleGX) == 0) {
      selected = fallback_quick_ball_target_after_search_started();
    }
    const bool found = move_deck_to_hand(selected);
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-QB-01; R-GAME-ITEM; P-KNOWLEDGE-01; P-COMPRESS-01",
          found ? "Discarded a redundant Regidrago V and searched the fastest K1 Basic connector: " +
                      std::string(name(selected)) + "."
                : "Discarded a redundant Regidrago V; no Basic Pokemon remained after search.");
    return true;
  }

  bool issue_1235_t2_treasure_tapu_context_available() const {
    if (scenario_.dci != DciProfile::StrictJit || state_.turn != 2 ||
        scenario_.locks != LockMode::None || !deck_seen_ ||
        !supporter_allowed() || state_.supporter_used || bench_space() == 0 ||
        hand_count(Card::MysteriousTreasure) == 0 ||
        (hand_count(Card::RegidragoVstar) == 0 && !active_is_vstar()) ||
        hand_count(Card::TapuLeleGX) > 0 || in_play(Card::TapuLeleGX) ||
        hand_count(Card::Crispin) > 0 ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        !state_.active ||
        (state_.active->card != Card::RegidragoV &&
         state_.active->card != Card::RegidragoVstar) ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass + state_.active->fire < 1 ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload) ||
        deck_count_after_search_started(Card::TapuLeleGX) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        deck_count_after_search_started(Card::Fire) == 0) {
      return false;
    }
    return state_.active->grass >= 1 &&
        state_.active->grass + deck_count_after_search_started(Card::Grass) >= 2 &&
        state_.active->fire + hand_count(Card::Fire) +
                deck_count_after_search_started(Card::Fire) >= 1;
  }

  bool play_mysterious_treasure(const bool permit_payload) {
    if (!permit_payload || !issue_1235_t2_treasure_tapu_context_available()) {
      return play_mysterious_treasure_issue1235_original(permit_payload);
    }

    const auto cost = choose_discard(true, true, true, Card::MysteriousTreasure);
    if (!cost || !is_payload(*cost)) return false;
    const Card discarded_payload = *cost;

    // Mysterious Treasure may pay the current-turn Dragon payload because the
    // searched Tapu Lele-GX obtains Crispin, which deterministically completes GGF:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, Ability, Supporter, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, earliest-route, and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1235
    remove_one(state_.hand, Card::MysteriousTreasure);
    state_.discard.push_back(Card::MysteriousTreasure);
    discard_from_hand(discarded_payload,
                      "Mysterious Treasure cost", "R-MT-01; P-JIT-01");
    record_deck_search_knowledge("Mysterious Treasure");
    if (!move_deck_to_hand(Card::TapuLeleGX)) {
      throw std::logic_error(
          "projected issue-1235 Mysterious Treasure Tapu Lele-GX target disappeared");
    }
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-MT-01; R-GAME-ITEM; P-JIT-01; P-COMPRESS-01",
          "Discarded the current-turn Dragon payload and searched Tapu Lele-GX for the Crispin T2 route.");
    return true;
  }

  bool issue_1235_t2_treasure_tapu_crispin_completion_available() const {
    if (!issue_1235_t2_treasure_tapu_context_available()) return false;
    Engine projected = *this;
    projected.trace_ = nullptr;
    if (!projected.play_mysterious_treasure(true)) return false;
    projected.play_basics_from_hand();
    if (projected.hand_count(Card::Crispin) == 0 ||
        !projected.play_crispin()) return false;
    projected.attach_manual();
    projected.evolve_best_regi();
    return projected.active_is_vstar() &&
        projected.state_.active->grass >= 2 &&
        projected.state_.active->fire >= 1 && projected.payload_ready();
  }

  void choose_supporter() {
    if (!issue_1235_t2_treasure_tapu_crispin_completion_available()) {
      choose_supporter_issue1235_original();
      return;
    }

    // Preserve the Supporter play for Treasure -> Tapu Lele-GX -> Crispin:
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Supporter, Item, Ability, attachment, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Earliest-route and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1235
    if (trace_ != nullptr) {
      trace_->add_policy_once(
          state_, "issue-1235-treasure-tapu-crispin-before-supporter", state_.turn,
          "HOLD SUPPORTER", "R-MT-01; R-TAPU-01; R-CRISPIN-01; P-COMPRESS-01",
          "Retained the Supporter play for the proven Active-Regidrago Treasure to Tapu Lele-GX to Crispin T2 finish.");
    }
  }
''')

    test = ROOT / "tests/issue_1235_oricorio_treasure_tapu_tests.cpp"
    atomic_write(test, r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool deck_seen = false) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = deck_seen;
  }
  static const State& state(const Engine& engine) { return engine.state_; }
  static bool t1_state(const Engine& engine) {
    return engine.issue_1235_t1_quick_ball_connector_state();
  }
  static bool play_quick_ball(Engine& engine) { return engine.play_quick_ball(false); }
  static void play_basics(Engine& engine) { engine.play_basics_from_hand(); }
  static void attach_manual(Engine& engine) { engine.attach_manual(); }
  static bool t2_completion(const Engine& engine) {
    return engine.issue_1235_t2_treasure_tapu_crispin_completion_available();
  }
  static void run_turn(Engine& engine) { engine.run_turn(); }
  static bool ready(const Engine& engine) {
    return engine.active_is_vstar() && engine.state_.active->grass >= 2 &&
        engine.state_.active->fire >= 1 && engine.payload_ready();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

int count_card(const std::vector<sim::Card>& cards, const sim::Card card) {
  return static_cast<int>(std::count(cards.begin(), cards.end(), card));
}

sim::Scenario strict_second(const sim::LockMode locks = sim::LockMode::None) {
  return sim::Scenario{"issue-1235", sim::DciProfile::StrictJit, locks, false, 5};
}

sim::State t1_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0}};
  state.hand = {sim::Card::QuickBall, sim::Card::RegidragoVstar,
                sim::Card::RegidragoV, sim::Card::DialgaGX,
                sim::Card::MysteriousTreasure, sim::Card::ChaoticSwell};
  state.deck = {sim::Card::Oricorio, sim::Card::TapuLeleGX,
                sim::Card::Crispin, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::Fire, sim::Card::Arven};
  return state;
}

sim::State t2_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 0, 1, 0};
  state.bench = {sim::Pokemon{sim::Card::RegidragoV, 0},
                 sim::Pokemon{sim::Card::Oricorio, 1}};
  state.hand = {sim::Card::MysteriousTreasure, sim::Card::DialgaGX,
                sim::Card::RegidragoVstar, sim::Card::Fire,
                sim::Card::Gladion};
  state.deck = {sim::Card::TapuLeleGX, sim::Card::Crispin,
                sim::Card::Grass, sim::Card::Fire,
                sim::Card::RegidragoV, sim::Card::Arven};
  return state;
}

sim::Engine make_engine(const sim::Scenario& scenario, std::mt19937_64& rng,
                        sim::State state, const bool deck_seen = false,
                        sim::TraceLog* trace = nullptr) {
  static const sim::DeckRecipe recipe = sim::baseline_recipe();
  sim::Engine engine(scenario, recipe, rng, trace);
  sim::EngineTestAccess::set_state(engine, std::move(state), deck_seen);
  return engine;
}

void test_t1_quick_ball_selects_oricorio_after_inspection() {
  std::mt19937_64 rng{123501};
  const sim::Scenario scenario = strict_second();
  sim::Engine engine = make_engine(scenario, rng, t1_state());

  // Quick Ball's legal deck inspection proves that Oricorio supplies the T1 Grass
  // attachment while leaving the T2 Treasure -> Tapu Lele-GX -> Crispin chain live.
  // This complete graph reaches T2, one turn before Tapu -> Steven's Resolve:
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/cel25c-60_A
  // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
  // Official Item, Ability, Supporter, Bench, and attachment procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1235
  expect(sim::EngineTestAccess::t1_state(engine),
         "The exact opening connector state must be recognized.");
  expect(sim::EngineTestAccess::play_quick_ball(engine),
         "Quick Ball must execute the cost-aware connector comparison.");
  const sim::State& searched = sim::EngineTestAccess::state(engine);
  expect(count_card(searched.discard, sim::Card::RegidragoV) == 1,
         "The redundant Regidrago V must pay Quick Ball.");
  expect(count_card(searched.hand, sim::Card::Oricorio) == 1,
         "Quick Ball must search Oricorio when the full T2 route is K1-proven.");
  expect(count_card(searched.hand, sim::Card::TapuLeleGX) == 0,
         "Tapu Lele-GX must remain for the T2 Treasure search.");
  sim::EngineTestAccess::play_basics(engine);
  sim::EngineTestAccess::attach_manual(engine);
  expect(sim::EngineTestAccess::state(engine).active->grass == 1,
         "Vital Dance plus the manual attachment must establish the T1 Grass.");
}

void test_t2_holds_gladion_and_finishes() {
  std::mt19937_64 rng{123502};
  sim::TraceLog trace;
  trace.enabled = true;
  const sim::Scenario scenario = strict_second();
  sim::Engine engine = make_engine(scenario, rng, t2_state(), true, &trace);
  expect(sim::EngineTestAccess::t2_completion(engine),
         "The Active-Regidrago Treasure-Tapu-Crispin route must project T2 readiness.");
  sim::EngineTestAccess::run_turn(engine);
  const sim::State& after = sim::EngineTestAccess::state(engine);
  expect(sim::EngineTestAccess::ready(engine),
         "The complete route must produce an Active GGF VSTAR and current-turn payload.");
  expect(count_card(after.discard, sim::Card::DialgaGX) == 1,
         "Mysterious Treasure must discard Dialga-GX on T2.");
  expect(count_card(after.discard, sim::Card::Crispin) == 1,
         "Wonder Tag must obtain and play Crispin.");
  expect(count_card(after.hand, sim::Card::Gladion) == 1,
         "The inferior held Supporter must remain unused.");
}

void test_controls_preserve_tapu_priority() {
  const auto blocked = [](sim::State state, const sim::Scenario scenario,
                          const std::uint64_t seed, const char* message) {
    std::mt19937_64 rng{seed};
    sim::Engine engine = make_engine(scenario, rng, std::move(state));
    expect(!sim::EngineTestAccess::t1_state(engine), message);
  };

  sim::State no_treasure = t1_state();
  no_treasure.hand.erase(std::find(no_treasure.hand.begin(), no_treasure.hand.end(),
                                  sim::Card::MysteriousTreasure));
  blocked(no_treasure, strict_second(), 123503,
          "Missing Treasure must block Oricorio priority.");

  sim::State no_payload = t1_state();
  no_payload.hand.erase(std::find(no_payload.hand.begin(), no_payload.hand.end(),
                                 sim::Card::DialgaGX));
  blocked(no_payload, strict_second(), 123504,
          "Missing payload must block the T2 continuation.");

  blocked(t1_state(), strict_second(sim::LockMode::FullRuleBoxAbility), 123505,
          "Rule Box Ability lock must block the Tapu continuation.");

  sim::State one_open_slot = t1_state();
  one_open_slot.bench.push_back(sim::Pokemon{sim::Card::DialgaGX, 0});
  one_open_slot.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 0});
  one_open_slot.bench.push_back(sim::Pokemon{sim::Card::Oricorio, 0});
  blocked(one_open_slot, strict_second(), 123506,
          "Fewer than two open Bench slots must block the two-connector route.");
}
}  // namespace

int main() {
  try {
    test_t1_quick_ball_selects_oricorio_after_inspection();
    test_t2_holds_gladion_and_finishes();
    test_controls_preserve_tapu_priority();
    std::cout << "issue 1235 tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
''')

    (ROOT / ".github/workflows/apply-issue-1235-v2.yml").unlink(missing_ok=True)
    Path(__file__).unlink(missing_ok=True)


if __name__ == "__main__":
    main()
