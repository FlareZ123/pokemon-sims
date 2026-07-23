from __future__ import annotations
import base64, os, zlib
from contextlib import contextmanager
from pathlib import Path

SOURCE=Path("src/trace_engine_v2/part_forretress_ex_combo.inc")
TEST=Path("tests/issue_1377_prior_turn_pineco_forest_tests.cpp")
LOCK=Path(".issue-1377-fix.lock")
OLD='  const bool combo_stadium_is_useful = !state_.stadium_used && need_energy() &&\n      hand_count(Card::ForestOfVitality) > 0 &&\n      state_.stadium != Stadium::ForestOfVitality;\n'
NEW='  const auto prior_turn_pineco = [this](const Pokemon& pokemon) {\n    return pokemon.card == Card::Pineco &&\n        pokemon.entered_turn < state_.turn;\n  };\n  const bool pineco_can_evolve_normally =\n      (state_.active && prior_turn_pineco(*state_.active)) ||\n      std::any_of(state_.bench.begin(), state_.bench.end(), prior_turn_pineco);\n  const bool current_turn_pineco =\n      (state_.active && state_.active->card == Card::Pineco &&\n       state_.active->entered_turn == state_.turn) ||\n      std::any_of(state_.bench.begin(), state_.bench.end(),\n                  [this](const Pokemon& pokemon) {\n                    return pokemon.card == Card::Pineco &&\n                        pokemon.entered_turn == state_.turn;\n                  });\n  const bool pineco_can_enter_this_turn = in_play_count(Card::Pineco) == 0 &&\n      bench_space() > 0 &&\n      (hand_count(Card::Pineco) > 0 || might_be_unseen(Card::Pineco));\n  const bool forretress_available = hand_count(Card::ForretressEx) > 0 ||\n      might_be_unseen(Card::ForretressEx);\n  // Forest of Vitality changes only entry-turn Grass evolution. Preserve it when\n  // a Pineco is already eligible under ordinary prior-turn timing, and spend it\n  // only for a current-turn or still-to-be-Benched Pineco with a live Forretress:\n  // Pineco: https://api.pokemontcg.io/v2/cards/sv4pt5-1\n  // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2\n  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117\n  // Core evolution and Stadium procedure: https://www.pokemon.com/us/pokemon-tcg/rules\n  // Resource-preservation policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1377\n  const bool combo_stadium_is_useful = !state_.stadium_used && need_energy() &&\n      hand_count(Card::ForestOfVitality) > 0 &&\n      state_.stadium != Stadium::ForestOfVitality &&\n      in_play_count(Card::ForretressEx) == 0 && !pineco_can_evolve_normally &&\n      forretress_available && (current_turn_pineco || pineco_can_enter_this_turn);\n'
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
            if text.count(OLD)!=1: raise RuntimeError(f"Unexpected Forest admission anchor count: {text.count(OLD)}")
            atomic(SOURCE,text.replace(OLD,NEW,1))
        atomic(TEST,zlib.decompress(base64.b64decode(TEST_Z)))
    LOCK.unlink(missing_ok=True)
    return 0

if __name__=="__main__": raise SystemExit(main())
