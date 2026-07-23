from __future__ import annotations
import base64, os, zlib
from contextlib import contextmanager
from pathlib import Path

SOURCE=Path("src/trace_engine_v2/part_issue_1071_fss_oricorio_treasure_decomposition_override.inc")
ADMISSION=Path("src/trace_engine_v2/part_011_fss_latias_override.inc")
TEST=Path("tests/issue_1379_t3_fss_latias_burnet_tests.cpp")
LOCK=Path(".issue-1379-fix.lock")
ANCHOR='  bool fss_latias_vessel_promotion_route_available() const {\n'
FUNCTION='  bool fss_latias_post_crispin_burnet_route_available() const {\n    const bool latias_ability_available =\n        scenario_.locks != LockMode::FullRuleBoxAbility &&\n        scenario_.locks != LockMode::FullCombined;\n    if (!deck_seen_ || scenario_.dci != DciProfile::StrictJit ||\n        state_.turn != 3 || !supporter_allowed() || state_.retreat_used ||\n        !state_.active || state_.active->card != Card::Oricorio ||\n        bench_space() <= 0 || in_play(Card::LatiasEx) ||\n        hand_count(Card::LatiasEx) > 0 ||\n        deck_count_after_search_started(Card::LatiasEx) == 0 ||\n        !latias_ability_available || hand_count(Card::RegidragoVstar) == 0 ||\n        hand_count(Card::ProfessorBurnet) == 0 ||\n        !payload_might_be_in_deck()) {\n      return false;\n    }\n\n    const Pokemon* evolving_regi = nullptr;\n    for (const Pokemon& pokemon : state_.bench) {\n      if (pokemon.card == Card::RegidragoV &&\n          pokemon.entered_turn < state_.turn) {\n        evolving_regi = &pokemon;\n        break;\n      }\n    }\n    if (evolving_regi == nullptr) return false;\n\n    const int missing_grass = std::max(0, 2 - evolving_regi->grass);\n    const int missing_fire = std::max(0, 1 - evolving_regi->fire);\n    const bool energy_complete_now = missing_grass == 0 && missing_fire == 0;\n    const bool unused_manual_completes = !state_.manual_energy_used &&\n        missing_grass + missing_fire == 1 &&\n        ((missing_grass == 1 && hand_count(Card::Grass) > 0) ||\n         (missing_fire == 1 && hand_count(Card::Fire) > 0));\n    if (!energy_complete_now && !unused_manual_completes) return false;\n\n    // Professor Burnet already owns the current-turn deck-to-discard payload axis.\n    // Star Alchemy should search Latias ex for the unresolved Active-position axis;\n    // the held VSTAR evolves the prior-turn Regidrago V, the current or projected\n    // manual attachment establishes GGF, Skyliner gives Oricorio free Retreat, and\n    // Burnet supplies the strict-JIT Dragon payload in the same turn:\n    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156\n    // Latias ex and Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76\n    // Professor Burnet: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26\n    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164\n    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136\n    // Core Tool, search, attachment, evolution, Bench, Ability, Supporter, and Retreat procedure: https://www.pokemon.com/us/pokemon-tcg/rules\n    // Complementary-axis priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n    // Strict-JIT payload timing: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment\n    // Original turn-two projection: https://github.com/FlareZ123/pokemon-sims/issues/1137\n    // Preceding Steven correction: https://github.com/FlareZ123/pokemon-sims/issues/1378\n    // Confirmed post-Crispin bug: https://github.com/FlareZ123/pokemon-sims/issues/1379\n    return true;\n  }\n\n'
OLD_WRAP='  Card fss_target_after_search_started() const {\n    if (fss_latias_over_redundant_burnet_route_available()) {\n      return Card::LatiasEx;\n    }\n\n'
NEW_WRAP='  Card fss_target_after_search_started() const {\n    if (fss_latias_over_redundant_burnet_route_available() ||\n        fss_latias_post_crispin_burnet_route_available()) {\n      return Card::LatiasEx;\n    }\n\n'
OLD_ADMISSION='            return fss_latias_over_redundant_burnet_route_available() ||\n                fss_latias_vessel_promotion_route_available() ||\n'
NEW_ADMISSION='            return fss_latias_over_redundant_burnet_route_available() ||\n                fss_latias_post_crispin_burnet_route_available() ||\n                fss_latias_vessel_promotion_route_available() ||\n'
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
        text=SOURCE.read_text(encoding="utf-8")
        if FUNCTION not in text:
            text=replace_once(text,ANCHOR,FUNCTION+ANCHOR,"post-Crispin function")
        text=replace_once(text,OLD_WRAP,NEW_WRAP,"FSS target wrapper")
        atomic(SOURCE,text)
        admission=ADMISSION.read_text(encoding="utf-8")
        atomic(ADMISSION,replace_once(admission,OLD_ADMISSION,NEW_ADMISSION,"Latias target admission"))
        atomic(TEST,zlib.decompress(base64.b64decode(TEST_Z)))
    LOCK.unlink(missing_ok=True)
    return 0

if __name__=="__main__": raise SystemExit(main())
