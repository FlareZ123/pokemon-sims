from pathlib import Path

source = Path("src/trace_engine_v2/part_012_override.inc")
text = source.read_text(encoding="utf-8")
old = '''    const bool complementary_quick_ball_for_latias =
        need_vstar() && state_.active && state_.active->card != Card::RegidragoV &&
        state_.active->card != Card::RegidragoVstar && evolvable_benched_regi &&
        bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
        hand_count(Card::LatiasEx) == 0 && !in_play(Card::LatiasEx) &&
        might_be_unseen(Card::LatiasEx) && held_treasure_then_quick_ball_is_payable();

    const auto item_candidates_for_current_knowledge =
        [this, can_pay_search_cost, complementary_quick_ball_for_latias]() {
'''
new = '''    const auto complementary_quick_ball_for_latias = [this, evolvable_benched_regi,
                                                         &held_treasure_then_quick_ball_is_payable]() {
      return need_vstar() && state_.active && state_.active->card != Card::RegidragoV &&
             state_.active->card != Card::RegidragoVstar && evolvable_benched_regi &&
             bench_space() > 0 && ability_available_for_pokemon(Card::LatiasEx) &&
             hand_count(Card::LatiasEx) == 0 && !in_play(Card::LatiasEx) &&
             might_be_unseen(Card::LatiasEx) && held_treasure_then_quick_ball_is_payable();
    };

    const auto item_candidates_for_current_knowledge =
        [this, can_pay_search_cost, &complementary_quick_ball_for_latias]() {
'''
if old not in text:
    raise SystemExit("K0-captured complementary route anchor missing")
text = text.replace(old, new, 1)
text = text.replace(
    "if (complementary_quick_ball_for_latias) add_item_candidate(Card::QuickBall, true);",
    "if (complementary_quick_ball_for_latias()) add_item_candidate(Card::QuickBall, true);",
    1,
)
source.write_text(text, encoding="utf-8")
Path(__file__).unlink()
