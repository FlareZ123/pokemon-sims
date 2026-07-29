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
            mode="w", encoding="utf-8", newline="", dir=path.parent,
            prefix=f".{path.name}.", delete=False
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


quick_ball_helper = r'''  bool issue_1797_quick_ball_tapu_steven_route_available() const {
    const int deck_grass = deck_count_after_search_started(Card::Grass);
    const int deck_fire = deck_count_after_search_started(Card::Fire);
    const bool deck_payload = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_payload);
    const bool legal_discard_cost = hand_count(Card::TateLiza) > 0 ||
        hand_count(Card::Lusamine) > 0;

    // Hisuian Heavy Ball has already established K1. Going first prevents a T1
    // Supporter play, while Quick Ball, Bench play, Wonder Tag, and the manual
    // attachment remain legal. Two held Grass preserve T1 and T2 attachment
    // throughput, while held Regidrago VSTAR preserves the evolution axis:
    // Hisuian Heavy Ball: https://api.pokemontcg.io/v2/cards/swsh10-146
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Lusamine: https://api.pokemontcg.io/v2/cards/sm4-96
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official first-turn, Item, Ability, search, Bench, attachment, evolution, and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, and earliest-route specifications: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1797
    return scenario_.dci == DciProfile::StrictJit && scenario_.going_first &&
        scenario_.locks == LockMode::None && state_.turn == 1 &&
        scenario_.max_turn >= 3 && prizes_known() && !item_locked() &&
        !state_.manual_energy_used && !state_.supporter_used && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->grass == 0 && state_.active->fire == 0 &&
        count_of(state_.discard, Card::HisuianHeavyBall) > 0 &&
        hand_count(Card::QuickBall) > 0 &&
        hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::Grass) >= 2 && legal_discard_cost &&
        bench_space() > 0 && ability_available_for_pokemon(Card::TapuLeleGX) &&
        hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) &&
        deck_count_after_search_started(Card::TapuLeleGX) > 0 &&
        deck_count_after_search_started(Card::StevensResolve) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        deck_grass > 0 && deck_fire > 0 && deck_grass + deck_fire >= 5 &&
        deck_payload;
  }

  std::optional<Card> issue_1797_quick_ball_cost() const {
    if (!issue_1797_quick_ball_tapu_steven_route_available()) {
      return std::nullopt;
    }
    // The complete K1 route replaces the next-turn Supporter role of Tate & Liza or
    // Lusamine with Steven's Resolve. Spend Tate & Liza first, then Lusamine, while
    // preserving the two Grass attachments, held VSTAR, and all three T2 package axes:
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Tate & Liza: https://api.pokemontcg.io/v2/cards/sm7-148
    // Lusamine: https://api.pokemontcg.io/v2/cards/sm4-96
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Dynamic DCI and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1797
    if (hand_count(Card::TateLiza) > 0) return Card::TateLiza;
    return hand_count(Card::Lusamine) > 0
        ? std::optional<Card>{Card::Lusamine}
        : std::nullopt;
  }

'''
replace_once(
    "src/trace_engine_v2/part_009b1.inc",
    "  bool play_quick_ball(const bool permit_payload) {\n",
    quick_ball_helper + "  bool play_quick_ball(const bool permit_payload) {\n",
)
replace_once(
    "src/trace_engine_v2/part_009b1.inc",
    "    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 && !in_play(Card::TapuLeleGX) && bench_space() > 0 &&\n"
    "                           needs_tapu_connector() && might_be_unseen(Card::TapuLeleGX);\n",
    "    const bool issue_1797_tapu_route =\n"
    "        issue_1797_quick_ball_tapu_steven_route_available();\n"
    "    const bool want_tapu = hand_count(Card::TapuLeleGX) == 0 &&\n"
    "        !in_play(Card::TapuLeleGX) && bench_space() > 0 &&\n"
    "        (needs_tapu_connector() || issue_1797_tapu_route) &&\n"
    "        might_be_unseen(Card::TapuLeleGX);\n",
)
replace_once(
    "src/trace_engine_v2/part_009b1.inc",
    "    auto cost = choose_discard(can_pay_payload_cost, true, true, Card::QuickBall);\n",
    "    std::optional<Card> cost;\n"
    "    if (issue_1797_tapu_route) {\n"
    "      cost = issue_1797_quick_ball_cost();\n"
    "    }\n"
    "    if (!cost) {\n"
    "      cost = choose_discard(can_pay_payload_cost, true, true, Card::QuickBall);\n"
    "    }\n",
)

wonder_tag_helper = r'''  bool issue_1797_wonder_tag_steven_route_available() const {
    const int deck_grass = deck_count_after_search_started(Card::Grass);
    const int deck_fire = deck_count_after_search_started(Card::Fire);
    const bool deck_payload = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_payload);
    const bool route_cost_paid = count_of(state_.discard, Card::TateLiza) > 0 ||
        count_of(state_.discard, Card::Lusamine) > 0;

    // Wonder Tag resolves on T1 after K1 Quick Ball. It may search Steven's Resolve
    // even though the going-first rule prevents playing that Supporter until T2:
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Official first-turn and Ability search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1 and complete-route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1797
    return scenario_.dci == DciProfile::StrictJit && scenario_.going_first &&
        scenario_.locks == LockMode::None && state_.turn == 1 &&
        scenario_.max_turn >= 3 && prizes_known() && !state_.supporter_used &&
        state_.active && state_.active->card == Card::RegidragoV &&
        state_.active->grass == 0 && state_.active->fire == 0 &&
        count_of(state_.discard, Card::QuickBall) > 0 && route_cost_paid &&
        in_play(Card::TapuLeleGX) &&
        ability_available_for_pokemon(Card::TapuLeleGX) &&
        hand_count(Card::RegidragoVstar) > 0 && hand_count(Card::Grass) >= 2 &&
        deck_count_after_search_started(Card::StevensResolve) > 0 &&
        deck_count_after_search_started(Card::Crispin) > 0 &&
        deck_count_after_search_started(Card::EarthenVessel) > 0 &&
        deck_grass > 0 && deck_fire > 0 && deck_grass + deck_fire >= 5 &&
        deck_payload;
  }

'''
replace_once(
    "src/trace_engine_v2/part_007.inc",
    "  bool issue_1796_wonder_tag_steven_route_available() const {\n",
    wonder_tag_helper + "  bool issue_1796_wonder_tag_steven_route_available() const {\n",
)
replace_once(
    "src/trace_engine_v2/part_007.inc",
    "  Card choose_supporter_after_search_started() const {\n"
    "    if (issue_1796_wonder_tag_steven_route_available()) {\n",
    "  Card choose_supporter_after_search_started() const {\n"
    "    if (issue_1797_wonder_tag_steven_route_available()) {\n"
    "      return Card::StevensResolve;\n"
    "    }\n"
    "    if (issue_1796_wonder_tag_steven_route_available()) {\n",
)

schedule_block = r'''  int issue_1797_finish_turn_ = 0;

  bool complete_issue_1797_t2_steven_package() {
    if (state_.turn != 2 || issue_1797_finish_turn_ != 0) return false;

    const bool tapu_in_play = in_play(Card::TapuLeleGX);
    const bool quick_ball_paid = count_of(state_.discard, Card::QuickBall) > 0 &&
        (count_of(state_.discard, Card::TateLiza) > 0 ||
         count_of(state_.discard, Card::Lusamine) > 0);
    const bool payload_available = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload) ||
        std::any_of(state_.deck.begin(), state_.deck.end(), is_payload);
    const bool route_available = scenario_.dci == DciProfile::StrictJit &&
        scenario_.going_first && scenario_.locks == LockMode::None &&
        scenario_.max_turn >= 3 && prizes_known() && supporter_allowed() &&
        !state_.manual_energy_used && !item_locked() && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass == 1 && state_.active->fire == 0 && tapu_in_play &&
        quick_ball_paid && hand_count(Card::RegidragoVstar) > 0 &&
        hand_count(Card::Grass) > 0 && hand_count(Card::StevensResolve) > 0 &&
        (hand_count(Card::Crispin) > 0 ||
         deck_count_after_search_started(Card::Crispin) > 0) &&
        (hand_count(Card::EarthenVessel) > 0 ||
         deck_count_after_search_started(Card::EarthenVessel) > 0) &&
        payload_available;
    if (!route_available) return false;

    remove_one(state_.hand, Card::Grass);
    ++state_.active->grass;
    state_.manual_energy_used = true;
    trace("ATTACH ENERGY", "R-GAME-ATTACH; P-COMPRESS-01",
          "Attached the held second Grass Energy before Steven's Resolve ends T2.");

    remove_one(state_.hand, Card::StevensResolve);
    state_.discard.push_back(Card::StevensResolve);
    state_.supporter_used = true;
    outcome_.used_steven = true;
    record_deck_search_knowledge("Steven's Resolve issue-1797 package");

    std::vector<Card> found;
    if (hand_count(Card::Crispin) == 0) {
      if (!move_deck_to_hand(Card::Crispin)) {
        throw std::logic_error("Issue-1797 Crispin target disappeared");
      }
      found.push_back(Card::Crispin);
    }
    if (hand_count(Card::EarthenVessel) == 0) {
      if (!move_deck_to_hand(Card::EarthenVessel)) {
        throw std::logic_error("Issue-1797 Earthen Vessel target disappeared");
      }
      found.push_back(Card::EarthenVessel);
    }
    if (!std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      const auto payload_it = std::find_if(
          state_.deck.begin(), state_.deck.end(), is_payload);
      if (payload_it == state_.deck.end()) {
        throw std::logic_error("Issue-1797 Dragon payload target disappeared");
      }
      const Card payload = *payload_it;
      if (!move_deck_to_hand(payload)) {
        throw std::logic_error("Issue-1797 Dragon payload search failed");
      }
      found.push_back(payload);
    }
    shuffle(state_.deck);
    state_.turn_ended = true;
    issue_1797_finish_turn_ = 3;

    // Steven searches the missing parts of the known Crispin, Vessel, and Dragon
    // package, then ends T2 after the second Grass attachment has been preserved:
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mega Dragonite ex example payload: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Official Supporter, attachment, and search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Complete observable route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1797
    trace("PLAY SUPPORTER", "R-STEVEN-01; R-GAME-SUPPORTER; P-KNOWLEDGE-01; P-COMPRESS-01",
          "Banked the issue-1797 T3 package: " + join_cards(found) +
              ". Turn ended.");
    return true;
  }

  bool complete_issue_1797_t3_finish() {
    if (issue_1797_finish_turn_ != state_.turn) return false;

    std::optional<Card> payload;
    for (const Card card : state_.hand) {
      if (is_payload(card)) {
        payload = card;
        break;
      }
    }
    const int deck_grass = deck_count_after_search_started(Card::Grass);
    const int deck_fire = deck_count_after_search_started(Card::Fire);
    const int crispin_search_count =
        (deck_fire > 0 ? 1 : 0) + (deck_grass > 0 ? 1 : 0);
    const bool fire_accessible = hand_count(Card::Fire) > 0 || deck_fire > 0;
    const bool vessel_target_survives =
        deck_grass + deck_fire > crispin_search_count;
    const bool route_available = scenario_.dci == DciProfile::StrictJit &&
        scenario_.going_first && scenario_.locks == LockMode::None &&
        state_.turn == 3 && state_.turn <= scenario_.max_turn && prizes_known() &&
        !state_.turn_ended && supporter_allowed() && !state_.manual_energy_used &&
        !item_locked() && state_.active &&
        state_.active->card == Card::RegidragoV &&
        state_.active->entered_turn < state_.turn &&
        state_.active->grass >= 2 && state_.active->fire == 0 &&
        hand_count(Card::RegidragoVstar) > 0 && hand_count(Card::Crispin) > 0 &&
        hand_count(Card::EarthenVessel) > 0 && payload.has_value() &&
        fire_accessible && crispin_search_count > 0 && vessel_target_survives;
    if (!route_available) {
      issue_1797_finish_turn_ = 0;
      return false;
    }

    evolve_best_regi();
    if (!active_is_vstar()) {
      throw std::logic_error("Issue-1797 Active Regidrago V did not evolve");
    }

    remove_one(state_.hand, Card::Crispin);
    state_.discard.push_back(Card::Crispin);
    state_.supporter_used = true;
    record_deck_search_knowledge("Crispin issue-1797 finish");
    bool searched_fire = false;
    bool searched_grass = false;
    if (deck_fire > 0 && move_deck_to_hand(Card::Fire)) {
      searched_fire = true;
    }
    if (deck_grass > 0 && move_deck_to_hand(Card::Grass)) {
      searched_grass = true;
    }
    shuffle(state_.deck);

    if (searched_fire) {
      remove_one(state_.hand, Card::Fire);
      ++state_.active->fire;
      trace("CRISPIN ATTACH", "R-CRISPIN-01; R-GAME-ATTACH",
            "Searched Fire and attached it to the Active Regidrago VSTAR.");
    } else {
      if (hand_count(Card::Fire) == 0) {
        throw std::logic_error("Issue-1797 held Fire disappeared");
      }
      remove_one(state_.hand, Card::Fire);
      ++state_.active->fire;
      state_.manual_energy_used = true;
      trace("ATTACH ENERGY", "R-GAME-ATTACH; R-CRISPIN-01",
            "Used the held Fire because the intervening draw removed Fire from Crispin's deck targets.");
    }
    trace("PLAY SUPPORTER", "R-CRISPIN-01; R-GAME-SUPPORTER",
          std::string("Crispin searched ") +
              (searched_fire ? "Fire" : "no Fire") + " and " +
              (searched_grass ? "Grass" : "no Grass") + ".");

    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(*payload, "Earthen Vessel issue-1797 Dragon cost",
                           "R-EV-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("Issue-1797 Vessel payload cost disappeared");
    }
    record_deck_search_knowledge("Earthen Vessel issue-1797 finish");
    bool vessel_found = false;
    if (move_deck_to_hand(Card::Grass)) vessel_found = true;
    if (move_deck_to_hand(Card::Fire)) vessel_found = true;
    if (!vessel_found) {
      throw std::logic_error("Issue-1797 Vessel Energy target disappeared");
    }
    shuffle(state_.deck);
    issue_1797_finish_turn_ = 0;

    // Crispin attaches the missing Fire when it remains in deck. If a public draw
    // moved that Fire into hand, the manual attachment supplies it instead. Vessel
    // then discards the Steven-searched Dragon during the same T3 ready check:
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mega Dragonite ex example payload: https://api.pokemontcg.io/v2/cards/me2pt5-152
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official evolution, Supporter, attachment, Item, discard, search, and attack procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Strict-JIT and setup horizon: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#ready-state-and-t5-policy
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1797
    trace("EARTHEN VESSEL", "R-EV-01; R-GAME-ITEM; P-DCI-01; P-JIT-01",
          "Discarded the held Dragon payload and searched the remaining Basic Energy.");
    return active_is_vstar() && state_.active->grass >= 2 &&
        state_.active->fire >= 1 && payload_ready();
  }

'''
replace_once(
    "src/trace_engine_v2/part_014c_issue_1152_bridge.inc",
    "  int issue_1796_finish_turn_ = 0;\n",
    schedule_block + "  int issue_1796_finish_turn_ = 0;\n",
)

replace_once(
    "src/trace_engine_v2/part_issue_1675_latias_late_promotion_override.inc",
    "  void run_turn() {\n    if (complete_issue_1796_t3_finish()) {\n",
    "  void run_turn() {\n"
    "    if (complete_issue_1797_t2_steven_package()) return;\n"
    "    if (complete_issue_1797_t3_finish()) {\n"
    "      // The K1 Heavy Ball, Quick Ball, Wonder Tag, Steven, Crispin, and Vessel route\n"
    "      // completes the registered strict-JIT setup on T3:\n"
    "      // https://api.pokemontcg.io/v2/cards/swsh10-146 https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/sm2-60 https://api.pokemontcg.io/v2/cards/sm7-145 https://api.pokemontcg.io/v2/cards/sv7-133 https://api.pokemontcg.io/v2/cards/sv4-163\n"
    "      // https://github.com/FlareZ123/pokemon-sims/issues/1797\n"
    "      trace(\"POLICY\", \"P-AXIS-01; R-QB-01; R-STEVEN-01; R-EV-01; P-JIT-01; P-COMPRESS-01\",\n"
    "            \"Completed the issue-1797 deterministic T3 route: \" + state_line());\n"
    "      return;\n"
    "    }\n"
    "    if (complete_issue_1796_t3_finish()) {\n",
)

atomic_write(Path("tests/issue_1797_quick_ball_tapu_steven_tests.cpp"), r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {

struct EngineTestAccess {
  static void set_state(Engine& engine, State state, const bool k1 = true) {
    engine.state_ = std::move(state);
    engine.deck_seen_ = k1;
    engine.prizes_revealed_ = k1;
  }
  static bool quick_ball_route(const Engine& engine) {
    return engine.issue_1797_quick_ball_tapu_steven_route_available();
  }
  static std::optional<Card> quick_ball_cost(const Engine& engine) {
    return engine.issue_1797_quick_ball_cost();
  }
  static bool play_quick_ball(Engine& engine) {
    return engine.play_quick_ball(false);
  }
  static bool wonder_tag_route(const Engine& engine) {
    return engine.issue_1797_wonder_tag_steven_route_available();
  }
  static Card supporter_target(const Engine& engine) {
    return engine.choose_supporter_after_search_started();
  }
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

sim::Scenario scenario(
    const sim::LockMode locks = sim::LockMode::None,
    const int max_turn = 5) {
  return sim::Scenario{"issue-1797-quick-ball-tapu-steven",
                       sim::DciProfile::StrictJit, locks, true, max_turn};
}

sim::State t1_state() {
  sim::State state;
  state.turn = 1;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {
      sim::Card::QuickBall,
      sim::Card::TateLiza,
      sim::Card::Lusamine,
      sim::Card::RegidragoVstar,
      sim::Card::Grass,
      sim::Card::Grass,
  };
  state.deck = {
      sim::Card::TapuLeleGX,
      sim::Card::StevensResolve,
      sim::Card::Crispin,
      sim::Card::EarthenVessel,
      sim::Card::MegaDragonite,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Grass,
      sim::Card::Fire,
      sim::Card::Fire,
      sim::Card::QuickBall,
  };
  state.discard = {sim::Card::HisuianHeavyBall};
  state.prizes = {
      sim::Card::ProfessorTuro,
      sim::Card::Dragapult,
      sim::Card::MysteriousTreasure,
      sim::Card::PathToPeak,
      sim::Card::Guzma,
      sim::Card::Oricorio,
  };
  return state;
}

struct Fixture {
  sim::Scenario scenario_value;
  sim::DeckRecipe recipe;
  std::mt19937_64 rng;
  sim::Engine engine;

  Fixture(sim::Scenario selected_scenario = scenario(),
          sim::DeckRecipe selected_recipe = sim::baseline_recipe(),
          const std::uint64_t seed = 1797)
      : scenario_value(std::move(selected_scenario)),
        recipe(std::move(selected_recipe)),
        rng(seed),
        engine(scenario_value, recipe, rng) {}
};

void quick_ball_selects_tapu_and_low_dci_cost() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, t1_state());

  // K1 proves the entire T1-T3 package before Quick Ball pays its cost:
  // https://api.pokemontcg.io/v2/cards/swsh10-146
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm2-60
  // https://api.pokemontcg.io/v2/cards/sm7-145
  // https://github.com/FlareZ123/pokemon-sims/issues/1797
  expect(sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The complete K1 Quick Ball route was not recognized");
  expect(sim::EngineTestAccess::quick_ball_cost(fixture.engine) ==
             sim::Card::TateLiza,
         "Quick Ball did not prefer Tate & Liza as the replaced Supporter cost");
  expect(sim::EngineTestAccess::play_quick_ball(fixture.engine),
         "Quick Ball did not resolve");
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(after.hand, sim::Card::TapuLeleGX),
         "Quick Ball did not search Tapu Lele-GX");
  expect(contains(after.discard, sim::Card::TateLiza),
         "Quick Ball did not discard Tate & Liza");
  expect(contains(after.hand, sim::Card::RegidragoVstar),
         "Quick Ball discarded the held VSTAR");
  expect(std::count(after.hand.begin(), after.hand.end(), sim::Card::Grass) == 2,
         "Quick Ball spent a required Grass attachment");
}

void lusamine_is_legal_fallback_cost() {
  Fixture fixture;
  sim::State state = t1_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::TateLiza));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(sim::EngineTestAccess::quick_ball_cost(fixture.engine) ==
             sim::Card::Lusamine,
         "Quick Ball did not use Lusamine after Tate & Liza was absent");
}

void wonder_tag_banks_steven_going_first() {
  Fixture fixture;
  sim::State state = t1_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::QuickBall));
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::TateLiza));
  state.discard.push_back(sim::Card::QuickBall);
  state.discard.push_back(sim::Card::TateLiza);
  state.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                                     sim::Tool::None});
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::TapuLeleGX));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  expect(sim::EngineTestAccess::wonder_tag_route(fixture.engine),
         "Wonder Tag did not recognize the complete going-first route");
  expect(sim::EngineTestAccess::supporter_target(fixture.engine) ==
             sim::Card::StevensResolve,
         "Wonder Tag did not bank Steven's Resolve on T1");
}

void k0_rejects_route() {
  Fixture fixture;
  sim::EngineTestAccess::set_state(fixture.engine, t1_state(), false);
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route read deck or Prize identities at K0");
}

void missing_discard_cost_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::TateLiza), state.hand.end());
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::Lusamine), state.hand.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented a Quick Ball discard cost");
}

void missing_tapu_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::TapuLeleGX), state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented Tapu Lele-GX");
}

void missing_vstar_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.hand.erase(std::remove(state.hand.begin(), state.hand.end(),
                               sim::Card::RegidragoVstar), state.hand.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented the held VSTAR");
}

void missing_second_grass_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(),
                             sim::Card::Grass));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented the second Grass attachment");
}

void item_lock_rejects_route() {
  Fixture fixture{scenario(sim::LockMode::FullItem)};
  sim::EngineTestAccess::set_state(fixture.engine, t1_state());
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route projected Quick Ball through Item lock");
}

void ability_lock_rejects_route() {
  Fixture fixture{scenario(sim::LockMode::Ability)};
  sim::EngineTestAccess::set_state(fixture.engine, t1_state());
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route projected Wonder Tag through Ability lock");
}

void full_bench_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  while (state.bench.size() < 5U) {
    state.bench.push_back(
        sim::Pokemon{sim::Card::CrobatV, 1, 0, 0, sim::Tool::None});
  }
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented Tapu Bench space");
}

void missing_package_piece_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::EarthenVessel), state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented Earthen Vessel");
}

void missing_payload_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.deck.erase(std::remove_if(state.deck.begin(), state.deck.end(),
                                  sim::is_payload), state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route invented a permitted Dragon payload");
}

void insufficient_energy_rejects_route() {
  Fixture fixture;
  sim::State state = t1_state();
  state.deck.erase(std::find(state.deck.begin(), state.deck.end(),
                             sim::Card::Grass));
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route ignored future draw and Vessel Energy contention");
}

void expired_horizon_rejects_route() {
  Fixture fixture{scenario(sim::LockMode::None, 2)};
  sim::EngineTestAccess::set_state(fixture.engine, t1_state());
  expect(!sim::EngineTestAccess::quick_ball_route(fixture.engine),
         "The route exceeded the T3 setup horizon");
}

void exact_seed_reaches_turn_three() {
  const auto selected_scenario =
      sim::scenario_by_label("strict-jit/go-first");
  const sim::NamedDeck* deck = sim::deck_by_id("regidrago-shell");
  expect(selected_scenario.has_value(), "Missing strict-JIT going-first scenario");
  expect(deck != nullptr, "Missing registered shell deck");

  std::mt19937_64 rng{854};
  sim::TraceLog trace{true, {}};
  sim::Engine engine{*selected_scenario, deck->recipe, rng, &trace};
  const sim::TrialOutcome outcome = engine.run();
  const auto trace_contains = [&trace](const std::string& text) {
    return std::any_of(trace.lines.begin(), trace.lines.end(),
                       [&text](const std::string& line) {
                         return line.find(text) != std::string::npos;
                       });
  };

  expect(outcome.first_ready_turn == 3,
         "Seed 854 did not reach the deterministic T3 route");
  expect(trace_contains("Quick Ball") && trace_contains("Tapu Lele-GX"),
         "Seed 854 did not use Quick Ball for Tapu Lele-GX");
  expect(trace_contains("WONDER TAG") && trace_contains("Steven's Resolve"),
         "Seed 854 did not bank Steven's Resolve through Wonder Tag");
  expect(trace_contains("T3 | READY"),
         "Seed 854 did not become ready on T3");
  expect(!trace_contains("Celestial Roar"),
         "Seed 854 still depended on random Celestial Roar");
}

}  // namespace

int main() {
  try {
    quick_ball_selects_tapu_and_low_dci_cost();
    lusamine_is_legal_fallback_cost();
    wonder_tag_banks_steven_going_first();
    k0_rejects_route();
    missing_discard_cost_rejects_route();
    missing_tapu_rejects_route();
    missing_vstar_rejects_route();
    missing_second_grass_rejects_route();
    item_lock_rejects_route();
    ability_lock_rejects_route();
    full_bench_rejects_route();
    missing_package_piece_rejects_route();
    missing_payload_rejects_route();
    insufficient_energy_rejects_route();
    expired_horizon_rejects_route();
    exact_seed_reaches_turn_three();
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
  return 0;
}
''')
