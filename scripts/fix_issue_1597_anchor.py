from pathlib import Path

path = Path("scripts/apply_issue_1597_patch.py")
source = path.read_text(encoding="utf-8")
old = '''turn_anchor = dedent(
    \'\'\'\\
      void run_turn() {
        if (secret_box_combo_enabled()) {
          run_secret_box_turn();
          return;
        }
        trace("POLICY", "P-AXIS-01", "Start: " + state_line());
        if (complete_late_steven_vstar_vessel_continuation()) return;
    \'\'\'
)
'''
new = '''turn_anchor = (
    "  void run_turn() {\\n"
    "    if (secret_box_combo_enabled()) {\\n"
    "      run_secret_box_turn();\\n"
    "      return;\\n"
    "    }\\n"
    "    trace(\\"POLICY\\", \\"P-AXIS-01\\", \\"Start: \\" + state_line());\\n"
    "    if (complete_late_steven_vstar_vessel_continuation()) return;\\n"
)
'''
if source.count(old) != 1:
    raise SystemExit(f"issue-1597 script anchor count: {source.count(old)}")
path.write_text(source.replace(old, new, 1), encoding="utf-8")
