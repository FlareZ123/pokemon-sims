from pathlib import Path

WORKFLOW = Path(".github/workflows/professors-letter-validation.yml")
ISSUE_URL = "https://github.com/FlareZ123/pokemon-sims/issues/3008"


def executable_lines(text: str) -> list[str]:
    return [
        line.strip()
        for line in text.splitlines()
        if line.strip() and not line.lstrip().startswith("#")
    ]


def main() -> None:
    text = WORKFLOW.read_text(encoding="utf-8")
    lines = executable_lines(text)
    scheduled_100k = [line for line in lines if "100000 20260705" in line]

    # The confirmed defect was two independent 100k experiment loops in one CI
    # job. The workflow must now schedule exactly one matched-seed 100k pass.
    # Confirmed overcomputation bug: https://github.com/FlareZ123/pokemon-sims/issues/3008
    assert len(scheduled_100k) == 1, (
        f"expected one 100k experiment command for {ISSUE_URL}, got "
        f"{len(scheduled_100k)}: {scheduled_100k}"
    )
    assert "professors-letter-analysis --analyze-real-games" in scheduled_100k[0]
    assert not any(
        "professors-letter-swap-matrix 100000" in line for line in lines
    ), "standalone 100k checkpoint experiment was reintroduced"

    # The single command owns all three evidence families, so CI cannot silently
    # drop the checkpoint or branch artifacts while removing the duplicate loop.
    # Evidence contract: https://github.com/FlareZ123/pokemon-sims/issues/3008
    command_index = lines.index(scheduled_100k[0])
    command_window = " ".join(lines[command_index : command_index + 5])
    for output in (
        "professors-letter-real-games.csv",
        "professors-letter-branches.csv",
        "professors-letter-swap-matrix.csv",
    ):
        assert output in command_window, f"single-pass command does not emit {output}"

    print("Professor's Letter single-pass CI contract is satisfied")


if __name__ == "__main__":
    main()
