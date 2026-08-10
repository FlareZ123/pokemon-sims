from __future__ import annotations

import base64
import gzip
import hashlib
import subprocess
from pathlib import Path

# Confirmed enhancement and source contract:
# https://github.com/FlareZ123/pokemon-sims/issues/1394
CHUNK_DIRECTORY = Path("scripts/issue_1394_patch_chunks")
EXPECTED_PATCH_SHA256 = "b99927a855bca1810e755aad6451dda270e532133ec06391074dfc20a28cda03"


def load_patch() -> bytes:
    chunks = sorted(CHUNK_DIRECTORY.glob("*.b64"))
    if len(chunks) != 5:
        raise RuntimeError(f"Expected 5 patch chunks, found {len(chunks)}")
    encoded = "".join(path.read_text(encoding="ascii").strip() for path in chunks)
    # Strict Base64 validation rejects malformed or incorrectly padded archives:
    # https://docs.python.org/3/library/base64.html#base64.b64decode
    patch = gzip.decompress(base64.b64decode(encoded, validate=True))
    # The archive reproduces the exact patch preserved by the successful #1394
    # current-main validation artifact rather than the abandoned bootstrap patch:
    # https://github.com/FlareZ123/pokemon-sims/actions/runs/30057886658
    if hashlib.sha256(patch).hexdigest() != EXPECTED_PATCH_SHA256:
        raise RuntimeError("Issue #1394 patch archive does not match validated artifact")
    return patch


def main() -> int:
    subprocess.run(
        ["git", "apply", "--3way", "--whitespace=nowarn", "-"],
        input=load_patch(),
        check=True,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
