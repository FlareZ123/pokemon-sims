from pathlib import Path

path = Path("src/trace_engine_v2/part_014c_latias_bench_override.inc")
text = path.read_text(encoding="utf-8")
old = '''  const int crispin_before = hand_count(Card::Crispin);
  if (!bench_from_hand(Card::TapuLeleGX, true) ||
      hand_count(Card::Crispin) <= crispin_before) {
    throw std::logic_error("Issue-1599 Wonder Tag did not obtain Crispin");
  }
'''
new = '''  if (!bench_from_hand(Card::TapuLeleGX, false)) {
    throw std::logic_error("Issue-1599 Tapu Lele-GX Bench action failed");
  }
  record_deck_search_knowledge("Wonder Tag issue-1599 Crispin route");
  if (!move_deck_to_hand(Card::Crispin)) {
    throw std::logic_error("Issue-1599 Crispin target disappeared");
  }
  shuffle(state_.deck);
  trace("WONDER TAG", "R-TAPU-01; R-CRISPIN-01; P-COMPRESS-01",
        "Searched and revealed Crispin for the missing Fire Energy axis.");
'''
if text.count(old) != 1:
    raise SystemExit(f"issue-1599 direct Wonder Tag anchor count: {text.count(old)}")
path.write_text(text.replace(old, new, 1), encoding="utf-8")
