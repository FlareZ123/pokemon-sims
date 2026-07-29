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


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if text.count(old) != 1:
        raise RuntimeError(f"Expected exactly one marker in {path}: {old!r}")
    atomic_write(path, text.replace(old, new, 1))


source = Path("src/trace_engine_v2/part_forretress_ex_combo.inc")
replace_once(
    source,
    """    trace("PLAY SUPPORTER", "R-DAWN-01; R-GAME-SUPPORTER; R-FORRETRESS-01",
          "Dawn searched and revealed: " + join_cards(selected) + ".");
    changed = true;
  }

  if (bench_pineco_if_useful()) changed = true;
""",
    """    trace("PLAY SUPPORTER", "R-DAWN-01; R-GAME-SUPPORTER; R-FORRETRESS-01",
          "Dawn searched and revealed: " + join_cards(selected) + ".");
    if (dawn_refills_secret_box_costs && play_secret_box()) {
      // Secret Box must resolve while the prior-turn Pineco and held Forretress ex
      // still prove the Grass line. Evolving and using Exploding Energy first removes
      // that line from the full-combo gate and strands the newly searched costs:
      // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
      // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
      // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
      // Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Core Supporter, Item, evolution, Ability, and attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
      // Deterministic current-route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1839
      trace("POLICY", "R-DAWN-01; R-SECRET-BOX-01; P-AXIS-01",
            "Dawn refilled and immediately paid Secret Box before the Forretress line resolved.");
    }
    changed = true;
  }

  if (bench_pineco_if_useful()) changed = true;
""",
)


test_path = Path("tests/issue_1839_dawn_secret_box_refill_tests.cpp")
test = test_path.read_text(encoding="utf-8")
replace_once(
    test_path,
    """  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(contains(state.hand, sim::Card::RegidragoV),
         "Dawn did not refill Secret Box's Basic cost slot");
  expect(contains(state.hand, sim::Card::Appletun),
         "Dawn did not refill Secret Box's Stage 1 cost slot");
  expect(contains(state.hand, sim::Card::MegaDragonite),
         "Dawn did not refill Secret Box's Stage 2 cost slot");
}
""",
    """  const sim::State& state = sim::EngineTestAccess::state(fixture.engine);
  expect(!contains(state.hand, sim::Card::SecretBox),
         "Dawn refilled the costs but Secret Box was not retried immediately");
  expect(contains(state.discard, sim::Card::SecretBox),
         "The immediate Secret Box resolution did not enter discard");
  expect(contains(state.hand, sim::Card::RegidragoVstar),
         "The complete route discarded the held evolution axis");
  expect(contains(state.hand, sim::Card::Fire),
         "The complete route discarded the held manual attachment");
  expect(!state.vstar_power_used,
         "The refill route consumed the once-per-game VSTAR Power");
  expect(contains(state.discard, sim::Card::Appletun) ||
             contains(state.discard, sim::Card::MegaDragonite) ||
             contains(state.discard, sim::Card::Dragapult),
         "Secret Box did not establish a same-turn Dragon payload");
}
""",
)
replace_once(
    test_path,
    """  expect(trace_contains("Dawn searched and revealed: Regidrago V, Appletun, Mega Dragonite ex"),
         "Dawn did not expose all three Secret Box cost categories");
""",
    """  expect(trace_contains("Dawn searched and revealed: Regidrago V, Appletun sv8-140, Mega Dragonite ex."),
         "Dawn did not expose all three Secret Box cost categories");
""",
)
