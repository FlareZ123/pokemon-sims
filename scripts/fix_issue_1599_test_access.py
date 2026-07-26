from pathlib import Path

path = Path("tests/issue_1599_quick_ball_tapu_crispin_tests.cpp")
text = path.read_text(encoding="utf-8")
anchor = "#include <utility>\n\nnamespace {\n"
replacement = (
    "#include <utility>\n\n"
    "namespace sim {\n"
    "struct EngineTestAccess {};\n"
    "}  // namespace sim\n\n"
    "namespace {\n"
)
if text.count(anchor) != 1:
    raise SystemExit(f"issue-1599 EngineTestAccess anchor count: {text.count(anchor)}")
path.write_text(text.replace(anchor, replacement, 1), encoding="utf-8")
