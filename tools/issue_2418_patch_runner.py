#!/usr/bin/env python3
from __future__ import annotations

import issue_2418_patch as patch

_original_replace_once = patch.replace_once


def replace_once(text: str, old: str, new: str, label: str) -> str:
    if label == "record returned DDE in discard":
        count = text.count(old)
        if count != 2:
            raise RuntimeError(
                f"{label}: expected Active and Oricorio attachment blocks, found {count}"
            )
        # The Active-promotion block appears first. Oricorio cannot legally carry DDE,
        # so only the first matching Turo cleanup block receives the DDE transition.
        return text.replace(old, new, 1)
    return _original_replace_once(text, old, new, label)


patch.replace_once = replace_once
raise SystemExit(patch.main())
