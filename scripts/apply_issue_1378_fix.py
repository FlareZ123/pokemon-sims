from __future__ import annotations
import base64, os, zlib
from contextlib import contextmanager
from pathlib import Path

SOURCE=Path("src/trace_engine_v2/part_issue_1030_steven_turo_override.inc")
TEST=Path("tests/issue_1378_no_lock_steven_grass_tests.cpp")
LOCK=Path(".issue-1378-fix.lock")
OLD='  bool issue_1223_full_item_lock_steven_grass_route_available() const {\n    if (!deck_seen_ || !supporter_allowed() ||\n        scenario_.dci != DciProfile::StrictJit ||\n        scenario_.locks != LockMode::FullItem ||\n'
NEW="  bool issue_1223_full_item_lock_steven_grass_route_available() const {\n    // The proven Steven package is determined by public K1 axes, not by Item lock.\n    // Admit the same fully guarded target split when no lock is present, while all\n    // other lock models retain their existing selectors:\n    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145\n    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133\n    // Original full-Item-lock route: https://github.com/FlareZ123/pokemon-sims/issues/1223\n    // Confirmed no-lock bug: https://github.com/FlareZ123/pokemon-sims/issues/1378\n    const bool supported_lock_state = scenario_.locks == LockMode::FullItem ||\n        scenario_.locks == LockMode::None;\n    if (!deck_seen_ || !supporter_allowed() ||\n        scenario_.dci != DciProfile::StrictJit || !supported_lock_state ||\n"
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

def atomic(path,data):
    tmp=path.with_suffix(path.suffix+".tmp")
    if isinstance(data,str): tmp.write_text(data,encoding="utf-8")
    else: tmp.write_bytes(data)
    os.replace(tmp,path)

def main():
    with locked(LOCK):
        text=SOURCE.read_text(encoding="utf-8")
        if NEW not in text:
            if text.count(OLD)!=1: raise RuntimeError(f"Unexpected Steven route anchor count: {text.count(OLD)}")
            atomic(SOURCE,text.replace(OLD,NEW,1))
        atomic(TEST,zlib.decompress(base64.b64decode(TEST_Z)))
    LOCK.unlink(missing_ok=True)
    return 0

if __name__=="__main__": raise SystemExit(main())
