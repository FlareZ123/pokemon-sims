from __future__ import annotations

import base64
import gzip
import subprocess
from pathlib import Path

# Confirmed enhancement and source contract:
# https://github.com/FlareZ123/pokemon-sims/issues/1394
CHUNK_DIRECTORY = Path("scripts/issue_1394_patch_chunks")


def load_patch() -> bytes:
    chunks = sorted(CHUNK_DIRECTORY.glob("*.b64"))
    if len(chunks) != 5:
        raise RuntimeError(f"Expected 5 patch chunks, found {len(chunks)}")
    encoded = "".join(path.read_text(encoding="ascii").strip() for path in chunks)
    return gzip.decompress(base64.b64decode(encoded))


def main() -> int:
    subprocess.run(
        ["git", "apply", "--3way", "--whitespace=nowarn", "-"],
        input=load_patch(),
        check=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
