from pathlib import Path
import sys

# Direct execution starts Python from tests/, so add the tracked repository root
# before importing the workflow selector:
# https://docs.python.org/3/library/sys_path_init.html
# https://github.com/FlareZ123/pokemon-sims/issues/2152
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from scripts.select_added_prize_k1_tests import select_added_prize_k1_tests


def test_ordinary_pull_request_has_no_focused_tests() -> None:
    # A general pull_request workflow may receive source, documentation, audit, or
    # unrelated test paths. The focused Prize-K1 step must skip those paths:
    # https://docs.github.com/en/actions/using-workflows/events-that-trigger-workflows#pull_request
    # https://github.com/FlareZ123/pokemon-sims/issues/2152
    paths = [
        "src/trace_engine_v2/part_example.inc",
        "docs/REPORT.md",
        "tests/issue_2152_workflow_contract_tests.cpp",
    ]
    assert select_added_prize_k1_tests(paths) == []


def test_intended_prize_k1_tests_are_selected() -> None:
    # The temporary workflow retains its original focused naming contract for
    # newly added issue-specific Prize-K1 C++ regressions:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/k1-stacked-ci.yml
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
