from pathlib import Path

src = Path('src/trace_engine_v2/part_010_fss_override.inc')
text = src.read_text()
old = '''        hand_count(Card::Crispin) == 0 || hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::TateLiza) == 0 ||
'''
new = '''        hand_count(Card::Crispin) == 0 ||
        deck_count_after_search_started(Card::Crispin) == 0 ||
        hand_count(Card::QuickBall) == 0 || hand_count(Card::TateLiza) == 0 ||
'''
if old not in text:
    raise SystemExit('production route gate not found')
text = text.replace(old, new, 1)
old_comment = '''    // On the projected next turn the established Regidrago V can evolve, the held
    // Fire attachment completes GGF, and Professor Burnet supplies the JIT payload.
'''
new_comment = '''    // This override is only an arbitration against the historical duplicate-Crispin
    // search. If no second Crispin is searchable, the ordinary selector may retain
    // Star Alchemy or choose another stronger route instead of spending it on Grass.
    // On the projected next turn the established Regidrago V can evolve, the held
    // Fire attachment completes GGF, and Professor Burnet supplies the JIT payload.
'''
if old_comment not in text:
    raise SystemExit('production route comment not found')
text = text.replace(old_comment, new_comment, 1)
src.write_text(text)

test = Path('tests/issue_2891_fss_seed23_state_generic_tests.cpp')
text = test.read_text()
old = '''  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::LatiasEx, sim::Card::MegaDragonite,
'''
new = '''  state.deck = {sim::Card::Grass, sim::Card::Grass, sim::Card::Fire,
                sim::Card::Crispin, sim::Card::LatiasEx, sim::Card::MegaDragonite,
'''
if old not in text:
    raise SystemExit('test route deck not found')
text = text.replace(old, new, 1)
marker = '''  expect_missing_hand_card(sim::Card::Crispin, 289115,
                           "The route requires held Crispin.");
'''
insert = marker + '''
  sim::State no_duplicate_crispin_state = route_state(2, 2);
  remove_card(no_duplicate_crispin_state.deck, sim::Card::Crispin);
  std::mt19937_64 duplicate_crispin_rng{2891151};
  sim::Engine no_duplicate_crispin = make_engine(
      strict_t3, duplicate_crispin_rng, std::move(no_duplicate_crispin_state));
  expect(!sim::EngineTestAccess::fss_grass_route(no_duplicate_crispin),
         "The Grass override must not replace an ordinary Star Alchemy hold when no duplicate Crispin is searchable.");
'''
if marker not in text:
    raise SystemExit('test Crispin marker not found')
text = text.replace(marker, insert, 1)
test.write_text(text)
