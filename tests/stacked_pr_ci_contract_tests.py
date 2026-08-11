from pathlib import Path
import sys


# Direct execution starts Python from tests/, so add the tracked repository root
# before importing the focused regression selector:
# https://docs.python.org/3/library/sys_path_init.html
# https://github.com/FlareZ123/pokemon-sims/issues/2152
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.select_added_prize_k1_tests import select_added_prize_k1_tests


def test_ordinary_pull_request_has_no_focused_tests() -> None:
    paths = [
        "src/trace_engine_v2/part_example.inc",
        "docs/REPORT.md",
        "tests/issue_2152_workflow_contract_tests.cpp",
    ]
    assert select_added_prize_k1_tests(paths) == []


def test_intended_prize_k1_tests_are_selected() -> None:
    # Stacked PR validation retains the focused Prize-K1 naming contract without
    # running the old duplicate full CI workflow on every main-targeting PR:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/stacked-pr-validation.yml
    # https://github.com/FlareZ123/pokemon-sims/issues/2152
    paths = [
        "tests/issue_2103_prize_k1_wonder_tag_arven_tests.cpp",
        "tests/issue_999_prize_k1_control.cpp",
        "tests/issue_x_prize_k1_invalid.cpp",
        "tests/issue_999_other.cpp",
    ]
    assert select_added_prize_k1_tests(paths) == [
        "tests/issue_2103_prize_k1_wonder_tag_arven_tests.cpp",
        "tests/issue_999_prize_k1_control.cpp",
    ]


if __name__ == "__main__":
    test_ordinary_pull_request_has_no_focused_tests()
    test_intended_prize_k1_tests_are_selected()
