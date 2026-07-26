from pathlib import Path

path = Path("scripts/apply_issue_1605_patch.py")
source = path.read_text(encoding="utf-8")
old = '''cost_anchor = dedent(
    \'\'\'\\
        const bool can_pay_search_cost =
            is_legal_cost_available(can_play_payload_this_turn(), true) ||
            final_energy_vessel_dead_role_cost;
    \'\'\'
)
'''
new = '''cost_anchor = (
    "    const bool can_pay_search_cost =\\n"
    "        is_legal_cost_available(can_play_payload_this_turn(), true) ||\\n"
    "        final_energy_vessel_dead_role_cost;\\n"
)
'''
if source.count(old) != 1:
    raise SystemExit(f"issue-1605 script anchor count: {source.count(old)}")
path.write_text(source.replace(old, new, 1), encoding="utf-8")
