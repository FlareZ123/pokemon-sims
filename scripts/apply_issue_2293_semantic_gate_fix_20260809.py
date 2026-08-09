from pathlib import Path
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]
part7 = ROOT / "src/trace_engine_v2/part_007.inc"
text = part7.read_text(encoding="utf-8")

old_gate = "        !is_basic(state_.active->card) || state_.active->card == Card::RegidragoV ||\n        !need_active_vstar() || bench_space() == 0 ||\n"
new_gate = "        !is_basic(state_.active->card) || state_.active->card == Card::RegidragoV ||\n        bench_space() == 0 ||\n"
if text.count(old_gate) != 1:
    raise RuntimeError("#2293 pre-evolution mobility gate anchor mismatch")
text = text.replace(old_gate, new_gate, 1)

axis_anchor = "    if (!attacker_axis_finishes) return false;\n\n    const bool current_turn_payload = std::any_of(\n"
axis_gate = "    if (!attacker_axis_finishes) return false;\n\n    // The reclaimed fix is deliberately route-semantic: require the exact observable\n    // supporter-independent axes described by #2293, rather than allowing any held\n    // payload or Blender state to globally outrank established Wonder Tag priorities.\n    // Quick Ball / Mysterious Treasure / Dialga-GX: https://api.pokemontcg.io/v2/cards/swsh1-179 https://api.pokemontcg.io/v2/cards/sm6-113 https://api.pokemontcg.io/v2/cards/sm5-100\n    // Regidrago V/VSTAR and Latias ex: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136 https://api.pokemontcg.io/v2/cards/sv8-76\n    // K1 and shortest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities\n    const bool prior_turn_regi = std::any_of(\n        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {\n          return pokemon.card == Card::RegidragoV &&\n              pokemon.entered_turn < state_.turn;\n        });\n    if (!prior_turn_regi || hand_count(Card::RegidragoVstar) == 0 ||\n        prize_count_after_reveal(Card::HisuianHeavyBall) == 0 ||\n        hand_count(Card::QuickBall) == 0 ||\n        hand_count(Card::MysteriousTreasure) < 2 ||\n        deck_count_after_search_started(Card::DialgaGX) == 0) {\n      return false;\n    }\n\n    const bool current_turn_payload = std::any_of(\n"
if text.count(axis_anchor) != 1:
    raise RuntimeError("#2293 semantic route anchor mismatch")
text = text.replace(axis_anchor, axis_gate, 1)

old_return = "    return current_turn_payload || blender_payload || direct_item_payload ||\n        quick_ball_treasure_payload;\n"
new_return = "    return quick_ball_treasure_payload;\n"
if text.count(old_return) != 1:
    raise RuntimeError("#2293 payload-route return anchor mismatch")
text = text.replace(old_return, new_return, 1)

fd, temp_name = tempfile.mkstemp(prefix=part7.name + ".", dir=part7.parent, text=True)
try:
    with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
        handle.write(text)
        handle.flush()
        os.fsync(handle.fileno())
    os.replace(temp_name, part7)
except BaseException:
    try:
        os.unlink(temp_name)
    except FileNotFoundError:
        pass
    raise

latias = ROOT / "src/trace_engine_v2/part_014c_latias_bench_override.inc"
ltext = latias.read_text(encoding="utf-8")
late_anchor = "  bool complete_late_latias_mobility_axis() {\n    if (state_.turn_ended || hand_count(Card::LatiasEx) == 0 ||\n"
late_gate = "  bool complete_late_latias_mobility_axis() {\n    // This late pass is only for a Supporter-resolved Prize bridge. Gladion itself\n    // occupies the exchanged Prize slot, so K1 can distinguish that transition from\n    // an ordinary Latias held before the Supporter action.\n    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95\n    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76\n    // Prize and Supporter procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf\n    if (!state_.supporter_used || !prizes_known() ||\n        prize_count_after_reveal(Card::Gladion) == 0 ||\n        state_.turn_ended || hand_count(Card::LatiasEx) == 0 ||\n"
if ltext.count(late_anchor) != 1:
    raise RuntimeError("#2293 late-Latias semantic gate anchor mismatch")
ltext = ltext.replace(late_anchor, late_gate, 1)

fd, temp_name = tempfile.mkstemp(prefix=latias.name + ".", dir=latias.parent, text=True)
try:
    with os.fdopen(fd, "w", encoding="utf-8", newline="") as handle:
        handle.write(ltext)
        handle.flush()
        os.fsync(handle.fileno())
    os.replace(temp_name, latias)
except BaseException:
    try:
        os.unlink(temp_name)
    except FileNotFoundError:
        pass
    raise
