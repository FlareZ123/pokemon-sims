from __future__ import annotations

import os
import tempfile
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
TARGET = ROOT / "tests/issue_1255_evolution_incense_appletun_continuation_tests.cpp"


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


def main() -> None:
    text = TARGET.read_text(encoding="utf-8")
    old = '''void test_forretress_incense_fallback_takes_appletun() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = ready_vstar();
  state.bench = {sim::Pokemon{sim::Card::Pineco, 1, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::EvolutionIncense};
  state.deck = {sim::Card::Appletun};
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state));
'''
    new = '''void test_forretress_incense_fallback_takes_appletun() {
  Fixture fixture;
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 0, 0,
                              sim::Tool::None};
  state.bench = {sim::Pokemon{sim::Card::Pineco, 1, 0, 0,
                              sim::Tool::None}};
  state.hand = {sim::Card::EvolutionIncense};
  state.deck = {sim::Card::Appletun};
  // K0 keeps the registered Forretress ex plausible until Evolution Incense
  // performs its legal deck inspection and discovers Appletun as the fallback:
  // https://api.pokemontcg.io/v2/cards/swsh1-163
  // https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
  // https://github.com/FlareZ123/pokemon-sims/issues/1255
  sim::EngineTestAccess::set_state(fixture.engine, std::move(state), false);
'''
    if text.count(old) != 1:
        raise RuntimeError("Expected one Forretress fixture anchor")
    atomic_write(TARGET, text.replace(old, new, 1))
    Path(__file__).unlink(missing_ok=True)


if __name__ == "__main__":
    main()
