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
