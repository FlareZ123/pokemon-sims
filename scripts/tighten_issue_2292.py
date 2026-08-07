from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
path = ROOT / "src/trace_engine_v2/part_012.inc"
text = path.read_text(encoding="utf-8")
old = """  bool issue_2292_gladion_final_prize_energy_finish(const Card energy) const {\n    if (!prizes_known() || !supporter_allowed() || state_.manual_energy_used ||\n"""
new = """  bool issue_2292_gladion_final_prize_energy_finish(const Card energy) const {\n    if (scenario_.dci != DciProfile::StrictJit || !prizes_known() ||\n        !supporter_allowed() || state_.manual_energy_used ||\n"""
if new not in text:
    if text.count(old) != 1:
        raise RuntimeError("expected exactly one #2292 helper anchor")
    text = text.replace(old, new, 1)
path.write_text(text, encoding="utf-8")
