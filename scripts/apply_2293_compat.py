from pathlib import Path

part7 = Path('src/trace_engine_v2/part_007.inc')
text = part7.read_text()
old = '''    const bool current_turn_payload = std::any_of(
        state_.discarded_this_turn.begin(), state_.discarded_this_turn.end(), is_payload);
    const bool blender_payload = hand_count(Card::BrilliantBlender) > 0 &&
        payload_might_be_in_deck() && can_play_payload_this_turn();

    const bool treasure_has_target = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_dragon_or_psychic);
    const bool quick_ball_has_target = std::any_of(
        state_.deck.begin(), state_.deck.end(), is_basic);
    const bool held_payload = std::any_of(
        state_.hand.begin(), state_.hand.end(), is_payload);
    const bool direct_item_payload = held_payload &&
        ((hand_count(Card::MysteriousTreasure) > 0 && treasure_has_target) ||
         (hand_count(Card::QuickBall) > 0 && quick_ball_has_target));

'''
if old not in text:
    raise SystemExit('part_007 unused projection block not found')
part7.write_text(text.replace(old, '', 1))

part14 = Path('src/trace_engine_v2/part_014c_latias_bench_override.inc')
text = part14.read_text()
old = '''    if (!retreat_to_benched_vstar_with_latias()) {
      throw std::logic_error("late Skyliner promotion disappeared");
    }
    return true;
'''
new = '''    if (!retreat_to_benched_vstar_with_latias()) {
      throw std::logic_error("late Skyliner promotion disappeared");
    }
    // This helper performs the same completed held-Latias mobility transition as
    // the established late-promotion path, so retain its semantic trace contract:
    // https://github.com/FlareZ123/pokemon-sims/issues/1675
    // https://github.com/FlareZ123/pokemon-sims/issues/2293
    trace("POLICY", "P-AXIS-01",
          "Completed the deadline Latias ex promotion route.");
    return true;
'''
if old not in text:
    raise SystemExit('part_014c late mobility return block not found')
part14.write_text(text.replace(old, new, 1))
