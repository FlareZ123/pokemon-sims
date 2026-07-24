import os
from pathlib import Path

for path in (
    Path("src/trace_engine_v2/part_issue_1516_quick_ball_tapu_crispin_override.inc"),
    Path("tests/issue_1516_quick_ball_tapu_crispin_tests.cpp"),
):
    text = path.read_text(encoding="utf-8")
    if "https://api.pokemontcg.io/v2/cards/cel25c-60_A" not in text:
        raise SystemExit(f"Expected the Classic Collection Tapu URL in {path}")
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(
        text.replace(
            "https://api.pokemontcg.io/v2/cards/cel25c-60_A",
            "https://api.pokemontcg.io/v2/cards/sm2-60",
        ),
        encoding="utf-8",
    )
    os.replace(temporary, path)
