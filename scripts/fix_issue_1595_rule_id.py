from pathlib import Path

path = Path("src/trace_engine_v2/part_issue_1595_preserve_turo_over_quick_ball_override.inc")
text = path.read_text(encoding="utf-8")
old = '"R-QB-01; R-TURO-01; P-DCI-01; P-COMPRESS-01"'
new = '"R-QB-01; P-DCI-01; P-COMPRESS-01; P-CONNECTOR-01"'
if text.count(old) != 1:
    raise SystemExit(f"issue-1595 trace-ID anchor count: {text.count(old)}")
path.write_text(text.replace(old, new, 1), encoding="utf-8")
