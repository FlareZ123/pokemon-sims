from pathlib import Path

path = Path("src/trace_engine_v2/part_014c_latias_bench_override.inc")
source = path.read_text(encoding="utf-8")
old = '''    trace("POLICY", "P-AXIS-01", "Start: " + state_line());

if (complete_late_steven_vstar_vessel_continuation()) return;
play_chaotic_swell();
play_field_blower();
if (complete_issue_1533_blender_treasure_latias_route()) {
  trace("POLICY", "P-AXIS-01", "End: " + state_line());
  return;
}
'''
new = '''    trace("POLICY", "P-AXIS-01", "Start: " + state_line());
    if (complete_late_steven_vstar_vessel_continuation()) return;
    play_chaotic_swell();
    play_field_blower();
    if (complete_issue_1533_blender_treasure_latias_route()) {
      trace("POLICY", "P-AXIS-01", "End: " + state_line());
      return;
    }
'''
if source.count(old) != 1:
    raise SystemExit(f"issue 1533 indentation anchor count: {source.count(old)}")
path.write_text(source.replace(old, new, 1), encoding="utf-8")
