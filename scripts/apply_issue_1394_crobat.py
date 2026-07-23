from __future__ import annotations

import base64
import gzip
import subprocess
from pathlib import Path

# Confirmed enhancement and source contract:
# https://github.com/FlareZ123/pokemon-sims/issues/1394
CHUNK_DIRECTORY = Path("scripts/issue_1394_patch_chunks")


def main() -> int:
    encoded = "".join(
        path.read_text(encoding="ascii").strip()
        for path in sorted(CHUNK_DIRECTORY.glob("*.b64"))
    )
    patch = gzip.decompress(base64.b64decode(encoded))
    subprocess.run(
        ["git", "apply", "--3way", "--whitespace=nowarn", "-"],
        input=patch,
        check=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
