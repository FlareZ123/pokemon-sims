from __future__ import annotations

import os
import runpy
from contextlib import contextmanager
from pathlib import Path
from typing import Iterator, TextIO


HELPER = Path("scripts/apply_issue_1393_fix.py")
TEST_PATH = Path("tests/issue_1393_crispin_before_gladion_tests.cpp")
TEST_LOCK = TEST_PATH.with_suffix(TEST_PATH.suffix + ".lock")


@contextmanager
def locked_file(path: Path) -> Iterator[TextIO]:
    path.parent.mkdir(parents=True, exist_ok=True)
    handle = path.open("a+", encoding="utf-8")
    try:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_LOCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_EX)
        yield handle
    finally:
        if os.name == "nt":
            import msvcrt

            handle.seek(0)
            msvcrt.locking(handle.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl

            fcntl.flock(handle.fileno(), fcntl.LOCK_UN)
        handle.close()


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(text: str, old: str, new: str, description: str) -> str:
    if new in text:
        return text
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Unexpected {description} anchor count: {count}")
    return text.replace(old, new, 1)


ADDITIONAL_TESTS = r'''void test_missing_grass_blocks_the_crispin_route() {
  Fixture fixture;
  sim::State state = exact_state();
  state.deck.erase(
      std::remove(state.deck.begin(), state.deck.end(), sim::Card::Grass),
      state.deck.end());
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Crispin requires two differently typed Basic Energy in the deck to supply the
  // second Grass plus Fire sequence. A missing Grass keeps Gladion ahead:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://github.com/FlareZ123/pokemon-sims/issues/1393
  if (sim::EngineTestAccess::crispin_preempts_gladion(fixture.engine)) {
    throw std::runtime_error(
        "Crispin preempted Gladion without a searchable second Grass.");
  }
}

void test_supporter_lock_blocks_both_supporter_routes() {
  Fixture fixture(strict_second_scenario(sim::LockMode::FullSupporter));
  sim::EngineTestAccess::set_state(fixture.engine, exact_state());

  // Full Supporter lock prevents Crispin and Gladion from being played from hand:
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#fully-modeled-parts
  // https://github.com/FlareZ123/pokemon-sims/issues/1393
  if (sim::EngineTestAccess::crispin_preempts_gladion(fixture.engine)) {
    throw std::runtime_error(
        "Crispin preempted Gladion through full Supporter lock.");
  }
}

void test_wrong_active_blocks_the_current_turn_finish() {
  Fixture fixture;
  sim::State state = exact_state();
  state.bench.push_back(
      sim::Pokemon{sim::Card::RegidragoV, 1, 1, 0, sim::Tool::None});
  state.active =
      sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Crispin completes Energy, while the filed T2 route also requires the attacker
  // to be Active. No switch route is present in this exact projection:
  // https://api.pokemontcg.io/v2/cards/sv7-133
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://www.pokemon.com/us/pokemon-tcg/rules
  // https://github.com/FlareZ123/pokemon-sims/issues/1393
  if (sim::EngineTestAccess::crispin_preempts_gladion(fixture.engine)) {
    throw std::runtime_error(
        "Crispin preempted Gladion without an Active Regidrago route.");
  }
}

void test_missing_vstar_preserves_a_genuinely_advancing_gladion() {
  Fixture fixture;
  sim::State state = exact_state();
  state.hand.erase(
      std::remove(state.hand.begin(), state.hand.end(),
                  sim::Card::RegidragoVstar),
      state.hand.end());
  state.prizes[0] = sim::Card::RegidragoVstar;
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // A known prized Regidrago VSTAR is an exact missing card axis. Gladion must keep
  // priority when Crispin cannot complete the current turn without that evolution:
  // https://api.pokemontcg.io/v2/cards/sm4-95
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // https://github.com/FlareZ123/pokemon-sims/issues/1393
  if (sim::EngineTestAccess::crispin_preempts_gladion(fixture.engine)) {
    throw std::runtime_error(
        "Crispin preempted a genuinely advancing prized VSTAR Gladion route.");
  }

  sim::EngineTestAccess::choose_supporter(fixture.engine);
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!after.supporter_used || contains(after.hand, sim::Card::Gladion) ||
      !contains(after.hand, sim::Card::RegidragoVstar)) {
    throw std::runtime_error(
        "Gladion did not recover the known prized Regidrago VSTAR.");
  }
}

'''


def add_required_controls() -> None:
    with locked_file(TEST_LOCK):
        source = TEST_PATH.read_text(encoding="utf-8")
        source = replace_once(
            source,
            "void test_seed_61_holds_star_alchemy_and_reaches_t2() {\n",
            ADDITIONAL_TESTS
            + "void test_seed_61_holds_star_alchemy_and_reaches_t2() {\n",
            "additional issue-1393 control insertion",
        )
        source = replace_once(
            source,
            "  test_current_turn_regidrago_cannot_evolve();\n"
            "  test_seed_61_holds_star_alchemy_and_reaches_t2();\n",
            "  test_current_turn_regidrago_cannot_evolve();\n"
            "  test_missing_grass_blocks_the_crispin_route();\n"
            "  test_supporter_lock_blocks_both_supporter_routes();\n"
            "  test_wrong_active_blocks_the_current_turn_finish();\n"
            "  test_missing_vstar_preserves_a_genuinely_advancing_gladion();\n"
            "  test_seed_61_holds_star_alchemy_and_reaches_t2();\n",
            "additional issue-1393 control registration",
        )
        atomic_write(TEST_PATH, source)
    try:
        TEST_LOCK.unlink()
    except FileNotFoundError:
        pass


def install_one_shot_post_commit_hook() -> None:
    hook = Path(".git/hooks/post-commit")
    hook.parent.mkdir(parents=True, exist_ok=True)
    hook.write_text(
        "#!/bin/sh\n"
        "set -eu\n"
        "rm -f .git/hooks/post-commit\n"
        "git checkout HEAD^ -- "
        ".github/workflows/issue-1393-crispin-before-gladion.yml "
        "scripts/apply_issue_1393_fix.py scripts/run_issue_1393_fix.py\n"
        "git commit --amend --no-edit --no-verify\n",
        encoding="utf-8",
        newline="\n",
    )
    hook.chmod(0o755)


def main() -> int:
    namespace = runpy.run_path(str(HELPER), run_name="issue_1393_helper")
    result = int(namespace["main"]())
    add_required_controls()
    install_one_shot_post_commit_hook()
    return result


if __name__ == "__main__":
    raise SystemExit(main())
