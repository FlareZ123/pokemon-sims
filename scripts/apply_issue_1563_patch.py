from __future__ import annotations

import os
import tempfile
from pathlib import Path
from typing import BinaryIO


def _lock(file: BinaryIO) -> None:
    if os.name == "nt":
        import msvcrt

        file.seek(0, os.SEEK_END)
        if file.tell() == 0:
            file.write(b"\0")
            file.flush()
        file.seek(0)
        msvcrt.locking(file.fileno(), msvcrt.LK_LOCK, 1)
        return

    import fcntl

    fcntl.flock(file.fileno(), fcntl.LOCK_EX)


def _unlock(file: BinaryIO) -> None:
    if os.name == "nt":
        import msvcrt

        file.seek(0)
        msvcrt.locking(file.fileno(), msvcrt.LK_UNLCK, 1)
        return

    import fcntl

    fcntl.flock(file.fileno(), fcntl.LOCK_UN)


def locked_atomic_write(path: Path, content: str) -> None:
    lock_path = path.with_name(f".{path.name}.lock")
    lock_path.parent.mkdir(parents=True, exist_ok=True)
    with lock_path.open("a+b") as lock_file:
        _lock(lock_file)
        temp_name: str | None = None
        try:
            with tempfile.NamedTemporaryFile(
                mode="w",
                encoding="utf-8",
                newline="",
                dir=path.parent,
                prefix=f".{path.name}.",
                suffix=".tmp",
                delete=False,
            ) as temp_file:
                temp_file.write(content)
                temp_file.flush()
                os.fsync(temp_file.fileno())
                temp_name = temp_file.name
            os.replace(temp_name, path)
        finally:
            if temp_name is not None and os.path.exists(temp_name):
                os.unlink(temp_name)
            _unlock(lock_file)
    lock_path.unlink(missing_ok=True)


source_path = Path("src/trace_engine_v2/part_issue_1118_secret_box.inc")
source = source_path.read_text(encoding="utf-8")

predicate_anchor = """  bool secret_box_regi_can_evolve_now() const {
    if (has_vstar()) return true;
    const auto eligible = [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV &&
             pokemon.entered_turn < state_.turn;
    };
    return (state_.active && eligible(*state_.active)) ||
        std::any_of(state_.bench.begin(), state_.bench.end(), eligible);
  }

"""
predicate_replacement = predicate_anchor + """  bool secret_box_direct_treasure_route_visible() const {
    // Once a prior-turn Regidrago V already has GGF, Secret Box does not need the
    // Pineco line. It may pay three other cards, fetch Mysterious Treasure, and
    // leave one held Dragon as Treasure's separate discard cost. Treasure then
    // searches Regidrago VSTAR and the held Dragon becomes the current-turn Apex
    // Dragon payload. Every target is public after K1 in the confirmed seed:
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Item, discard, search, evolution, and attack procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // K1, dynamic DCI, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1563
    return secret_box_combo_enabled() && !item_locked() && state_.turn > 1 &&
        hand_count(Card::SecretBox) > 0 && !need_energy() && need_vstar() &&
        need_payload() && secret_box_regi_can_evolve_now() &&
        hand_count(Card::MysteriousTreasure) == 0 &&
        might_be_unseen(Card::MysteriousTreasure) &&
        might_be_unseen(Card::RegidragoVstar) &&
        std::any_of(state_.hand.begin(), state_.hand.end(), is_payload);
  }

"""
if source.count(predicate_anchor) != 1:
    raise SystemExit(
        f"issue-1563 predicate anchor count: {source.count(predicate_anchor)}"
    )
source = source.replace(predicate_anchor, predicate_replacement, 1)

cost_anchor = """    const auto copies = [&remaining](const Card target) {
      const auto found = remaining.find(target);
      return found == remaining.end() ? 0 : found->second;
    };

"""
cost_replacement = cost_anchor + """    // The direct Box-to-Treasure route has two independent discard payments.
    // Reserve the sole held Dragon for Mysterious Treasure; Secret Box may spend a
    // Dragon only when another physical payload remains after that choice:
    // https://api.pokemontcg.io/v2/cards/sv6-163
    // https://api.pokemontcg.io/v2/cards/sm6-113
    // https://github.com/FlareZ123/pokemon-sims/issues/1563
    if (secret_box_direct_treasure_route_visible() && is_payload(card)) {
      int remaining_payloads = 0;
      for (const auto& [candidate, count] : remaining) {
        if (is_payload(candidate)) remaining_payloads += count;
      }
      if (remaining_payloads <= 1) return -1;
    }

"""
if source.count(cost_anchor) != 1:
    raise SystemExit(f"issue-1563 cost anchor count: {source.count(cost_anchor)}")
source = source.replace(cost_anchor, cost_replacement, 1)

play_gate_anchor = """    ++outcome_.secret_box_combo_attempted;
    if (bench_space() == 0 && in_play_count(Card::Pineco) == 0 &&
        in_play_count(Card::ForretressEx) == 0) {
      ++outcome_.secret_box_bench_blocked;
      return false;
    }
    if (!secret_box_full_combo_axes_visible()) {
      ++outcome_.secret_box_missing_axis;
      record_secret_box_missing_axes();
      return false;
    }
"""
play_gate_replacement = """    ++outcome_.secret_box_combo_attempted;
    const bool direct_treasure_route =
        secret_box_direct_treasure_route_visible();
    if (!direct_treasure_route && bench_space() == 0 &&
        in_play_count(Card::Pineco) == 0 &&
        in_play_count(Card::ForretressEx) == 0) {
      ++outcome_.secret_box_bench_blocked;
      return false;
    }
    if (!direct_treasure_route && !secret_box_full_combo_axes_visible()) {
      ++outcome_.secret_box_missing_axis;
      record_secret_box_missing_axes();
      return false;
    }
"""
if source.count(play_gate_anchor) != 1:
    raise SystemExit(
        f"issue-1563 play gate anchor count: {source.count(play_gate_anchor)}"
    )
source = source.replace(play_gate_anchor, play_gate_replacement, 1)

selection_anchor = """    const auto take = [this, &selected](const Card card) {
      if (move_deck_to_hand(card)) selected.push_back(card);
    };

"""
selection_replacement = selection_anchor + """    if (direct_treasure_route) {
      // The printed Secret Box searches are optional. The proven direct route needs
      // only Mysterious Treasure, so do not add a Pineco Supporter or Stadium whose
      // branch cannot improve the current T4 completion:
      // https://api.pokemontcg.io/v2/cards/sv6-163
      // https://api.pokemontcg.io/v2/cards/sm6-113
      // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // https://github.com/FlareZ123/pokemon-sims/issues/1563
      take(Card::MysteriousTreasure);
      shuffle(state_.deck);
      trace("PLAY ITEM", "R-SECRET-BOX-01; R-MT-01; R-GAME-ITEM; P-DCI-01",
            "Secret Box discarded three other cards and searched the direct "
            "Mysterious Treasure completion channel.");
      return true;
    }

"""
if source.count(selection_anchor) != 1:
    raise SystemExit(
        f"issue-1563 selection anchor count: {source.count(selection_anchor)}"
    )
source = source.replace(selection_anchor, selection_replacement, 1)
locked_atomic_write(source_path, source)


test_path = Path("tests/issue_1118_multi_deck_secret_box_tests.cpp")
test_source = test_path.read_text(encoding="utf-8")

access_anchor = """  static void run_secret_box_turn(Engine& engine) {
    engine.run_secret_box_turn();
  }
"""
access_replacement = access_anchor + """  static void set_deck_seen(Engine& engine, const bool seen = true) {
    engine.deck_seen_ = seen;
  }
"""
if test_source.count(access_anchor) != 1:
    raise SystemExit(
        f"issue-1563 access anchor count: {test_source.count(access_anchor)}"
    )
test_source = test_source.replace(access_anchor, access_replacement, 1)

helper_anchor = """sim::State complete_route_state() {
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::SecretBox, sim::Card::Grant,
                sim::Card::WishfulBaton, sim::Card::ErikasInvitation};
  state.deck = {
      sim::Card::MysteriousTreasure, sim::Card::ForestSealStone,
      sim::Card::Dawn, sim::Card::ForestOfVitality, sim::Card::Pineco,
      sim::Card::ForretressEx, sim::Card::Dragapult,
      sim::Card::RegidragoVstar, sim::Card::Fire,
      sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
      sim::Card::Grass, sim::Card::Grass,
  };
  return state;
}

"""
helper_replacement = helper_anchor + """sim::State direct_treasure_route_state() {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 2, 3, 1,
                              sim::Tool::ForestSealStone};
  state.hand = {
      sim::Card::SecretBox, sim::Card::Dragapult, sim::Card::Grass,
      sim::Card::Grass, sim::Card::ForestOfVitality,
      sim::Card::ForestOfVitality, sim::Card::Guzma,
      sim::Card::WishfulBaton,
  };
  state.deck = {sim::Card::MysteriousTreasure,
                sim::Card::RegidragoVstar, sim::Card::Grant};
  return state;
}

"""
if test_source.count(helper_anchor) != 1:
    raise SystemExit(
        f"issue-1563 helper anchor count: {test_source.count(helper_anchor)}"
    )
test_source = test_source.replace(helper_anchor, helper_replacement, 1)

test_anchor = """void test_route_lock_and_bench_controls() {
"""
test_function = """void test_direct_secret_box_treasure_completion() {
  Fixture reservation(sim::LockMode::None, sim::DciProfile::MatchupFlexJit);
  sim::EngineTestAccess::set_state(reservation.engine,
                                   direct_treasure_route_state());
  sim::EngineTestAccess::set_deck_seen(reservation.engine);

  // Secret Box and Mysterious Treasure have separate printed discard costs. The
  // sole held Dragapult ex must survive the three-card Box payment so Treasure can
  // discard it on the actual ready turn and search the Dragon VSTAR:
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
  // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
  // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
  // Core discard, search, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1563
  if (!sim::EngineTestAccess::play_secret_box(reservation.engine)) {
    throw std::runtime_error(
        "The Energy-complete direct Secret Box route was still blocked.");
  }
  const sim::State& paid = sim::EngineTestAccess::state(reservation.engine);
  if (!contains(paid.hand, sim::Card::Dragapult) ||
      !contains(paid.hand, sim::Card::MysteriousTreasure) ||
      contains(paid.discard, sim::Card::Dragapult)) {
    throw std::runtime_error(
        "Secret Box failed to reserve the sole Treasure payload cost.");
  }

  Fixture complete(sim::LockMode::None, sim::DciProfile::MatchupFlexJit);
  sim::State state = direct_treasure_route_state();
  for (int index = 0; index < 5; ++index) {
    state.bench.push_back(sim::Pokemon{sim::Card::TapuLeleGX, 1});
  }
  sim::EngineTestAccess::set_state(complete.engine, std::move(state));
  sim::EngineTestAccess::set_deck_seen(complete.engine);
  sim::EngineTestAccess::run_secret_box_turn(complete.engine);
  const sim::State& final_state = sim::EngineTestAccess::state(complete.engine);
  if (!final_state.active ||
      final_state.active->card != sim::Card::RegidragoVstar ||
      !contains(final_state.discard, sim::Card::Dragapult) ||
      !contains(final_state.discard, sim::Card::MysteriousTreasure) ||
      !sim::EngineTestAccess::outcome(complete.engine).used_secret_box) {
    throw std::runtime_error(
        "The direct Box-Treasure-VSTAR route did not complete on a full Bench.");
  }

  const auto must_reject = [](sim::State state, const sim::LockMode lock,
                              const char* message) {
    Fixture fixture(lock, sim::DciProfile::MatchupFlexJit);
    sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
    sim::EngineTestAccess::set_deck_seen(fixture.engine);
    if (sim::EngineTestAccess::play_secret_box(fixture.engine)) {
      throw std::runtime_error(message);
    }
  };

  state = direct_treasure_route_state();
  state.deck.erase(std::remove(state.deck.begin(), state.deck.end(),
                               sim::Card::MysteriousTreasure),
                   state.deck.end());
  must_reject(std::move(state), sim::LockMode::None,
              "The direct route ignored a K1-missing Treasure target.");

  state = direct_treasure_route_state();
  state.active->fire = 0;
  must_reject(std::move(state), sim::LockMode::None,
              "The direct route ignored an incomplete Energy axis.");

  state = direct_treasure_route_state();
  state.active->entered_turn = state.turn;
  must_reject(std::move(state), sim::LockMode::None,
              "The direct route ignored the evolution timing window.");

  state = direct_treasure_route_state();
  state.hand = {sim::Card::SecretBox, sim::Card::Dragapult,
                sim::Card::WishfulBaton, sim::Card::ForestOfVitality};
  must_reject(std::move(state), sim::LockMode::None,
              "The direct route spent its reserved payload with too few Box costs.");

  must_reject(direct_treasure_route_state(), sim::LockMode::FullItem,
              "Item lock admitted the direct Secret Box route.");
}

"""
if test_source.count(test_anchor) != 1:
    raise SystemExit(
        f"issue-1563 test insertion anchor count: {test_source.count(test_anchor)}"
    )
test_source = test_source.replace(test_anchor, test_function + test_anchor, 1)

main_anchor = """  test_combo_ultra_ball_fallback_includes_appletun();
  test_route_lock_and_bench_controls();
"""
main_replacement = """  test_combo_ultra_ball_fallback_includes_appletun();
  test_direct_secret_box_treasure_completion();
  test_route_lock_and_bench_controls();
"""
if test_source.count(main_anchor) != 1:
    raise SystemExit(
        f"issue-1563 main anchor count: {test_source.count(main_anchor)}"
    )
test_source = test_source.replace(main_anchor, main_replacement, 1)
locked_atomic_write(test_path, test_source)
