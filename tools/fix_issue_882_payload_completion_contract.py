from __future__ import annotations

import fcntl
import os
import tempfile
from pathlib import Path

PATH = Path("tests/mysterious_treasure_payload_continuation_tests.cpp")
HELPER_OLD = """  static bool play_serena(Engine& engine) { return engine.play_serena(); }
"""
HELPER_NEW = """  static bool play_serena(Engine& engine,
                           const bool allow_zero_draw_payload_completion = false) {
    return engine.play_serena(allow_zero_draw_payload_completion);
  }
"""
CONTROL_OLD = """  // Serena's draw mode must discard at least one card. That discard is a real effect
  // even when the preceding search emptied the deck, so the payload bridge stays live:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/
"""
CONTROL_NEW = """  // Serena's draw mode must discard at least one card. This exact zero-draw
  // resolution is retained only through the dedicated branch because discarding the
  // fetched Mega Dragonite ex immediately completes Apex Dragon's final setup axis:
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://api.pokemontcg.io/v2/cards/swsh12-164
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://github.com/FlareZ123/pokemon-sims/issues/882
"""
CALL_OLD = """  expect(sim::EngineTestAccess::play_serena(engine),
         "Serena should discard the fetched payload even though no draw card remains.");
"""
CALL_NEW = """  expect(sim::EngineTestAccess::play_serena(engine, true),
         "The dedicated Serena completion branch should discard the fetched payload even when no draw card remains.");
"""

with PATH.open("r+", encoding="utf-8") as locked:
    fcntl.flock(locked.fileno(), fcntl.LOCK_EX)
    text = locked.read()
    updated = text
    for old, new, label in (
        (HELPER_OLD, HELPER_NEW, "Serena test-access helper"),
        (CONTROL_OLD, CONTROL_NEW, "payload-completion explanation"),
        (CALL_OLD, CALL_NEW, "payload-completion call"),
    ):
        if new in updated and old not in updated:
            continue
        if updated.count(old) != 1:
            raise SystemExit(f"Unable to find unique {label}")
        updated = updated.replace(old, new, 1)

    with tempfile.NamedTemporaryFile(
        "w", encoding="utf-8", dir=PATH.parent, delete=False
    ) as temporary:
        temporary.write(updated)
        temporary.flush()
        os.fsync(temporary.fileno())
        temporary_path = Path(temporary.name)
    os.replace(temporary_path, PATH)
