from __future__ import annotations

import re
from pathlib import Path


ROOTS = (Path("src"), Path("tests"), Path("scripts"))
SUFFIXES = {".cpp", ".h", ".hpp", ".inc", ".py", ".cmake"}
LEGACY_PAYLOADS = {
    "Card::MegaDragonite",
    "Card::Dragapult",
    "Card::GoodraVstar",
    "Card::DialgaGX",
}
REGISTERED_PINECO_CARDS = {
    "Card::Pineco",
    "Card::ForretressEx",
    "Card::Appletun",
}
CARD_LIST_RE = re.compile(r"\{(?P<body>[^{}]{0,1800}Card::[^{}]{0,1800})\}", re.DOTALL)
CARD_TOKEN_RE = re.compile(r"Card::[A-Za-z0-9_]+")
MARKER_RE = re.compile(r"\b(?:TODO|FIXME|HACK|XXX)\b")


def line_number(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def source_files() -> list[Path]:
    files: list[Path] = []
    for root in ROOTS:
        if not root.exists():
            continue
        files.extend(
            path
            for path in root.rglob("*")
            if path.is_file() and path.suffix.lower() in SUFFIXES
        )
    files.append(Path("CMakeLists.txt"))
    return sorted(set(files))


def main() -> None:
    suspicious: list[str] = []
    inventory: list[str] = []
    markers: list[str] = []

    for path in source_files():
        text = path.read_text(encoding="utf-8")

        for match in MARKER_RE.finditer(text):
            markers.append(f"{path}:{line_number(text, match.start())}: {match.group(0)}")

        for match in CARD_LIST_RE.finditer(text):
            tokens = sorted(set(CARD_TOKEN_RE.findall(match.group("body"))))
            if len(tokens) < 3:
                continue

            line = line_number(text, match.start())
            inventory.append(f"{path}:{line}: {', '.join(tokens)}")

            token_set = set(tokens)
            legacy_hits = token_set & LEGACY_PAYLOADS
            if len(legacy_hits) >= 2 and "Card::Appletun" not in token_set:
                suspicious.append(
                    f"{path}:{line}: legacy payload list omits Card::Appletun: "
                    f"{', '.join(tokens)}"
                )

            nearby_start = max(0, match.start() - 700)
            nearby_end = min(len(text), match.end() + 700)
            nearby = text[nearby_start:nearby_end]
            if (
                ("EvolutionIncense" in nearby or "Evolution Incense" in nearby)
                and legacy_hits
                and "Card::Appletun" not in token_set
            ):
                suspicious.append(
                    f"{path}:{line}: Evolution Incense-adjacent list omits Appletun: "
                    f"{', '.join(tokens)}"
                )

            if (
                "pokemon_communication" in nearby.lower()
                and token_set & REGISTERED_PINECO_CARDS
                and not REGISTERED_PINECO_CARDS.issubset(token_set)
            ):
                suspicious.append(
                    f"{path}:{line}: Pokémon Communication-adjacent Pineco card list is partial: "
                    f"{', '.join(tokens)}"
                )

    print("STATIC CARD-ROUTE SCAN")
    print("Repository specification: https://github.com/FlareZ123/pokemon-sims/blob/main/README.md")
    print("Policy specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md")
    print("Appletun: https://api.pokemontcg.io/v2/cards/sv8-140")
    print("Evolution Incense: https://api.pokemontcg.io/v2/cards/swsh1-163")
    print("Pokémon Communication: https://api.pokemontcg.io/v2/cards/sm9-152")
    print()

    print(f"SUSPICIOUS ({len(suspicious)})")
    for entry in sorted(set(suspicious)):
        print(entry)
    print()

    print(f"UNRESOLVED MARKERS ({len(markers)})")
    for entry in sorted(set(markers)):
        print(entry)
    print()

    print(f"CARD LIST INVENTORY ({len(inventory)})")
    for entry in inventory:
        print(entry)


if __name__ == "__main__":
    main()
