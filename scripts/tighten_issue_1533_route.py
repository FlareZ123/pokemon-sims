from pathlib import Path

path = Path("src/trace_engine_v2/part_014c_latias_bench_override.inc")
source = path.read_text(encoding="utf-8")
old = """        if (!strict_payload_timing() || item_locked() || target == nullptr ||
            target->card != Card::RegidragoVstar || target->grass < 2 ||
            target->fire < 1 || !need_active_vstar() || !need_payload() ||
            state_.retreat_used || !state_.active ||
            state_.active->card != Card::TapuLeleGX || bench_space() == 0 ||
            !ability_available_for_pokemon(Card::LatiasEx) || in_play(Card::LatiasEx) ||
            hand_count(Card::LatiasEx) > 0 || !might_be_unseen(Card::LatiasEx) ||
            hand_count(Card::MysteriousTreasure) == 0 ||
            hand_count(Card::BrilliantBlender) == 0 || !payload_might_be_in_deck()) {
"""
new = """        if (!strict_payload_timing() || item_locked() || !deck_seen_ ||
            state_.ace_spec_used || target == nullptr ||
            target->card != Card::RegidragoVstar || target->grass < 2 ||
            target->fire < 1 || !need_active_vstar() || !need_payload() ||
            state_.retreat_used || !state_.active ||
            state_.active->card != Card::TapuLeleGX || bench_space() == 0 ||
            !ability_available_for_pokemon(Card::LatiasEx) || in_play(Card::LatiasEx) ||
            hand_count(Card::LatiasEx) > 0 ||
            deck_count_after_search_started(Card::LatiasEx) == 0 ||
            hand_count(Card::MysteriousTreasure) == 0 ||
            hand_count(Card::BrilliantBlender) == 0 ||
            !std::any_of(state_.deck.begin(), state_.deck.end(), is_payload)) {
"""
if source.count(old) != 1:
    raise SystemExit(f"issue-1533 K1 guard anchor count: {source.count(old)}")
path.write_text(source.replace(old, new, 1), encoding="utf-8")
