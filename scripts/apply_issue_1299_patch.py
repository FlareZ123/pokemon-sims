from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
LOCK_PATH = ROOT / ".apply_issue_1299_patch.lock"


def atomic_write(path: Path, text: str) -> None:
    descriptor, temporary_name = tempfile.mkstemp(
        prefix=f".{path.name}.", suffix=".tmp", dir=path.parent
    )
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
    count = text.count(old)
    if count != 1:
        raise RuntimeError(f"Expected one source anchor in {path}, found {count}")
    atomic_write(path, text.replace(old, new, 1))


def apply_patch() -> None:
    combo = ROOT / "src/trace_engine_v2/part_forretress_ex_combo.inc"
    replace_once(
        combo,
        """Card Engine::fallback_combo_pokemon_after_search_started() const {
  for (const Card target : {Card::ForretressEx, Card::Pineco,
                            Card::RegidragoVstar, Card::RegidragoV,
                            Card::LatiasEx, Card::Oricorio,
                            Card::TapuLeleGX, Card::MawileGX,
                            Card::DialgaGX, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::Dipplin}) {
""",
        """Card Engine::fallback_combo_pokemon_after_search_started() const {
  // Ultra Ball searches any Pokémon, so the post-inspection fallback includes
  // the registered Appletun payload:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/1299
  for (const Card target : {Card::ForretressEx, Card::Pineco,
                            Card::RegidragoVstar, Card::RegidragoV,
                            Card::LatiasEx, Card::Oricorio,
                            Card::TapuLeleGX, Card::MawileGX,
                            Card::DialgaGX, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::Appletun, Card::Dipplin}) {
""",
    )

    test = ROOT / "tests/issue_1118_multi_deck_secret_box_tests.cpp"
    replace_once(
        test,
        """  static void run_secret_box_turn(Engine& engine) {
    engine.run_secret_box_turn();
  }
};
""",
        """  static void run_secret_box_turn(Engine& engine) {
    engine.run_secret_box_turn();
  }
  static bool play_combo_ultra_ball(Engine& engine) {
    return engine.play_ultra_ball_for_forretress_combo();
  }
};
""",
    )
    replace_once(
        test,
        """void test_route_lock_and_bench_controls() {
""",
        """void test_combo_ultra_ball_fallback_includes_appletun() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::UltraBall, sim::Card::Grant,
                sim::Card::WishfulBaton};
  state.deck = {sim::Card::Appletun};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));

  // Ultra Ball pays two cards, legally inspects the deck, and must take the only
  // remaining Pokémon, Appletun, after the preferred Pineco target is absent:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/sv8-140
  // https://github.com/FlareZ123/pokemon-sims/issues/1299
  if (!sim::EngineTestAccess::play_combo_ultra_ball(fixture.engine)) {
    throw std::runtime_error("The combo Ultra Ball route did not begin.");
  }
  const sim::State& after = sim::EngineTestAccess::state(fixture.engine);
  if (!contains(after.hand, sim::Card::Appletun) ||
      contains(after.deck, sim::Card::Appletun)) {
    throw std::runtime_error(
        "The combo Ultra Ball fallback left the only legal Pokémon in deck.");
  }

  Fixture no_pokemon;
  state = sim::State{};
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoV, 1, 0, 0,
                              sim::Tool::None};
  state.hand = {sim::Card::UltraBall, sim::Card::Grant,
                sim::Card::WishfulBaton};
  state.deck = {sim::Card::Grass};
  sim::EngineTestAccess::set_state(no_pokemon.engine, std::move(state));
  if (!sim::EngineTestAccess::play_combo_ultra_ball(no_pokemon.engine) ||
      contains(sim::EngineTestAccess::state(no_pokemon.engine).hand,
               sim::Card::Appletun)) {
    throw std::runtime_error("The no-Pokémon combo fallback control diverged.");
  }
}

void test_route_lock_and_bench_controls() {
""",
    )
    replace_once(
        test,
        """  test_reviewed_seeded_routes();
  test_route_lock_and_bench_controls();
}
""",
        """  test_reviewed_seeded_routes();
  test_combo_ultra_ball_fallback_includes_appletun();
  test_route_lock_and_bench_controls();
}
""",
    )


def main() -> None:
    LOCK_PATH.touch(exist_ok=True)
    with LOCK_PATH.open("r+") as lock_handle:
        fcntl.flock(lock_handle.fileno(), fcntl.LOCK_EX)
        apply_patch()
        fcntl.flock(lock_handle.fileno(), fcntl.LOCK_UN)
    LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
