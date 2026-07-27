from pathlib import Path

path = Path("scripts/apply_issue_1533_current_main.py")
source = path.read_text(encoding="utf-8")

old_anchor = 'call_anchor = "    play_chaotic_swell();\\n    play_field_blower();\\n"\n'
new_anchor = (
    'call_anchor = "    if (complete_late_steven_vstar_vessel_continuation()) return;\\n'
    '    play_chaotic_swell();\\n    play_field_blower();\\n"\n'
)
old_text = """call_text = dedent(
    r'''
    play_chaotic_swell();
"""
new_text = """call_text = dedent(
    r'''
    if (complete_late_steven_vstar_vessel_continuation()) return;
    play_chaotic_swell();
"""

if source.count(old_anchor) != 1:
    raise SystemExit(f"issue 1533 materializer anchor-definition count: {source.count(old_anchor)}")
if source.count(old_text) != 1:
    raise SystemExit(f"issue 1533 materializer call-text count: {source.count(old_text)}")
source = source.replace(old_anchor, new_anchor, 1).replace(old_text, new_text, 1)
path.write_text(source, encoding="utf-8")
