import os
from pathlib import Path

path = Path(
    "src/trace_engine_v2/part_issue_1516_quick_ball_tapu_crispin_override.inc"
)
text = path.read_text(encoding="utf-8")
old = r'''    // Quick Ball pays one discard and searches one Basic Pokemon. The projection
    // executes that exact final selector before suppressing the real play. When it
    // selects Tapu Lele-GX and the established duplicate-Crispin guard sees held
    // Gladion covering a known no-control payload Prize, the searched Supporter
    // adds no earlier setup axis and every physical resource should be preserved:
'''
new = r'''    const bool existing_no_marginal_route =
        projected.wonder_tag_duplicate_held_crispin_has_no_marginal_route();
    const int crispin_before = projected.hand_count(Card::Crispin);
    const bool benched_tapu = projected.bench_tapu_if_useful();
    const bool selected_duplicate_crispin = benched_tapu &&
        projected.hand_count(Card::Crispin) > crispin_before;

    // Quick Ball pays one discard and searches one Basic Pokemon. The projection
    // executes that exact final selector and then the actual Wonder Tag selector
    // before suppressing the real play. When it selects a duplicate held Crispin
    // while held Gladion covers a known no-control payload Prize, the searched
    // Supporter adds no earlier setup axis and every physical resource is preserved:
'''
if text.count(old) != 1:
    raise SystemExit("Expected one issue-1516 projection comment anchor")
text = text.replace(old, new, 1)
old_return = "    return projected.wonder_tag_duplicate_held_crispin_has_no_marginal_route();\n"
new_return = (
    "    return selected_duplicate_crispin &&\n"
    "        (existing_no_marginal_route || known_prized_payload);\n"
)
if text.count(old_return) != 1:
    raise SystemExit("Expected one issue-1516 projection return")
text = text.replace(old_return, new_return, 1)
temporary = path.with_suffix(path.suffix + ".tmp")
temporary.write_text(text, encoding="utf-8")
os.replace(temporary, path)
