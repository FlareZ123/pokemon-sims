from __future__ import annotations

import tempfile
from pathlib import Path

from scripts.extract_deck_matrix import extract_deck_rows


# The regression preserves the canonical shell file as an exact row subset of
# the single paired aggregate instead of launching a second simulator experiment:
# https://github.com/FlareZ123/pokemon-sims/blob/main/README.md#generate-the-paired-two-deck-matrices
# https://github.com/FlareZ123/pokemon-sims/issues/2724
def main() -> int:
    paired = (
        'deck,scenario,trials,ready_by_t2_pct\n'
        '"regidrago-shell","strict-jit/go-first",100000,12.19\n'
        '"regidrago-pineco","strict-jit/go-first",100000,19.762\n'
        '"regidrago-shell","strict-jit/go-second",100000,29.812\n'
        '"regidrago-pineco","strict-jit/go-second",100000,48.305\n'
    )
    expected = (
        'deck,scenario,trials,ready_by_t2_pct\n'
        '"regidrago-shell","strict-jit/go-first",100000,12.19\n'
        '"regidrago-shell","strict-jit/go-second",100000,29.812\n'
    )

    with tempfile.TemporaryDirectory() as directory:
        root = Path(directory)
        source = root / "paired.csv"
        output = root / "shell.csv"
        source.write_text(paired, encoding="utf-8", newline="")

        extract_deck_rows(source, output, "regidrago-shell")
        assert output.read_text(encoding="utf-8") == expected
        assert not Path(f"{output}.lock").exists()

        try:
            extract_deck_rows(source, output, "missing-deck")
        except ValueError as error:
            assert "deck not found" in str(error)
        else:
            raise AssertionError("missing deck must fail")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
