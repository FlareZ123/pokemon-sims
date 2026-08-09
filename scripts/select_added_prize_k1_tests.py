from __future__ import annotations

import re
import sys
from collections.abc import Iterable


# The temporary stacked workflow owns only newly added issue-specific Prize-K1
# regressions. Ordinary PR paths must produce an empty selection rather than a
# false workflow failure:
# https://github.com/FlareZ123/pokemon-sims/blob/main/.github/workflows/k1-stacked-ci.yml
# https://github.com/FlareZ123/pokemon-sims/issues/2152
_PRIZE_K1_TEST = re.compile(r"^tests/issue_[0-9]+_prize_k1_.*\.cpp$")


def is_prize_k1_test(path: str) -> bool:
    return _PRIZE_K1_TEST.fullmatch(path) is not None


def select_added_prize_k1_tests(paths: Iterable[str]) -> list[str]:
    return [path for raw in paths if (path := raw.strip()) and is_prize_k1_test(path)]


def main() -> int:
    for path in select_added_prize_k1_tests(sys.stdin):
        print(path)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
