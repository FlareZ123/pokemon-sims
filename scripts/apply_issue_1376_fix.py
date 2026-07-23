from __future__ import annotations
import base64, os, zlib
from contextlib import contextmanager
from pathlib import Path

SOURCE=Path("src/trace_engine_v2/part_issue_1118_secret_box.inc")
LEGACY=Path("tests/issue_1118_multi_deck_secret_box_tests.cpp")
TEST=Path("tests/issue_1376_t1_pineco_before_steven_tests.cpp")
LOCK=Path(".issue-1376-fix.lock")
SOURCE_OLD='      proactive_tapu_retreat_for_pineco();\n      attach_manual();\n\n      if (!state_.supporter_used && !play_secret_box_planned_supporter()) {'
SOURCE_NEW="      proactive_tapu_retreat_for_pineco();\n      attach_manual();\n\n      // A held Pineco is a public, costless Basic Bench action. Benching it before\n      // Steven's Resolve ends turn one preserves ordinary turn-two evolution and\n      // avoids spending Secret Box's Stadium channel on Forest of Vitality:\n      // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1\n      // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145\n      // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117\n      // Core Bench, Supporter, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules\n      // Resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1376\n      bench_pineco_if_useful();\n      if (!state_.supporter_used && !play_secret_box_planned_supporter()) {"
LEGACY_OLD='  // Seed 35 proves strict DCI may spend Grant, Wishful Baton, and a\n  // dynamically replaceable search Item, then use Dawn, Forest, Treasure,\n  // and Forretress. The exact third cost is intentionally policy-flexible.\n  expect_seeded_route(35, {\n      "Grant (Secret Box cost)",\n      "Wishful Baton (Secret Box cost)",\n      "Secret Box discarded three other cards",\n      "Dawn searched and revealed: Dragapult ex",\n      "Forest of Vitality allowed the newly played Pineco",\n      "Mysterious Treasure cost",\n      "T2 | READY",\n  });\n'
LEGACY_NEW='  // Seed 35 proves strict DCI may spend Grant, Wishful Baton, and a\n  // dynamically replaceable search Item, then Bench the held Pineco before\n  // Steven ends turn one. Dawn, ordinary evolution, Treasure, and Forretress\n  // preserve the T2 route without consuming Forest of Vitality:\n  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1\n  // Steven\'s Resolve: https://api.pokemontcg.io/v2/cards/sm7-145\n  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117\n  // Core Bench, Supporter, and evolution procedure: https://www.pokemon.com/us/pokemon-tcg/rules\n  // Original route contract: https://github.com/FlareZ123/pokemon-sims/issues/1118\n  // Stronger resource-preserving route: https://github.com/FlareZ123/pokemon-sims/issues/1376\n  expect_seeded_route(35, {\n      "Grant (Secret Box cost)",\n      "Wishful Baton (Secret Box cost)",\n      "Secret Box discarded three other cards",\n      "T1 | BENCH | rules: R-GAME-BENCH | Pineco from hand",\n      "Dawn searched and revealed: Dragapult ex",\n      "Pineco evolved into Forretress ex under normal prior-turn timing",\n      "Mysterious Treasure cost",\n      "T2 | READY",\n  });\n'
TEST_Z='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'

@contextmanager
def locked(path):
    h=path.open("a+",encoding="utf-8")
    try:
        if os.name=="nt":
            import msvcrt
            h.seek(0); msvcrt.locking(h.fileno(),msvcrt.LK_LOCK,1)
        else:
            import fcntl
            fcntl.flock(h.fileno(),fcntl.LOCK_EX)
        yield
    finally:
        if os.name=="nt":
            import msvcrt
            h.seek(0); msvcrt.locking(h.fileno(),msvcrt.LK_UNLCK,1)
        else:
            import fcntl
            fcntl.flock(h.fileno(),fcntl.LOCK_UN)
        h.close()

def replace_once(text,old,new,name):
    if new in text: return text
    if text.count(old)!=1: raise RuntimeError(f"Unexpected {name} anchor count: {text.count(old)}")
    return text.replace(old,new,1)

def atomic(path,data):
    tmp=path.with_suffix(path.suffix+".tmp")
    if isinstance(data,str): tmp.write_text(data,encoding="utf-8")
    else: tmp.write_bytes(data)
    os.replace(tmp,path)

def main():
    with locked(LOCK):
        atomic(SOURCE,replace_once(SOURCE.read_text(),SOURCE_OLD,SOURCE_NEW,"source"))
        atomic(LEGACY,replace_once(LEGACY.read_text(),LEGACY_OLD,LEGACY_NEW,"legacy test"))
        atomic(TEST,zlib.decompress(base64.b64decode(TEST_Z)))
    LOCK.unlink(missing_ok=True)
    return 0

if __name__=="__main__": raise SystemExit(main())
