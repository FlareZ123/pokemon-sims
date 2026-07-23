from __future__ import annotations

import base64
import gzip
import subprocess
from pathlib import Path

# Confirmed route-planning bug and exact source contract:
# https://github.com/FlareZ123/pokemon-sims/issues/1403
# The branch-only push workflow applies and removes this helper after validation.
CHUNK_DIRECTORY = Path("scripts/issue_1403_patch_chunks")


def main() -> int:
    chunks = sorted(CHUNK_DIRECTORY.glob("*.b64"))
    if len(chunks) != 2:
        raise RuntimeError(f"Expected 2 patch chunks, found {len(chunks)}")
    encoded = "".join(path.read_text(encoding="ascii").strip() for path in chunks)
    patch = gzip.decompress(base64.b64decode(encoded, validate=True))
    subprocess.run(
        ["git", "apply", "--3way", "--whitespace=error-all", "-"],
        input=patch,
        check=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
