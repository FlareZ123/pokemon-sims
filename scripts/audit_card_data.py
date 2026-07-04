"""Extract and validate the exact supplied deck card facts from pokemon-tcg-data.

This tool accepts either the upstream repository directory or its ZIP archive. Output is written
through an exclusive lock and atomic replacement so concurrent reproductions cannot leave a
partial JSON artifact.
"""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import tempfile
import zipfile
from contextlib import contextmanager
from typing import Any, Iterator

REQUESTED: dict[str, tuple[str, int]] = {
    "Regidrago V": ("swsh12-135", 4),
    "Regidrago VSTAR": ("swsh12-136", 3),
    "Dragapult ex": ("sv6-130", 2),
    "Mega Dragonite ex": ("me2pt5-152", 2),
    "Dialga-GX": ("sm5-100", 1),
    "Hisuian Goodra VSTAR": ("swsh11-136", 1),
    "Tapu Lele-GX": ("cel25c-60_A", 2),
    "Latias ex": ("sv8-76", 1),
    "Mawile-GX": ("sm11-141", 1),
    "Oricorio GRI 55": ("sm2-55", 1),
    "Dipplin TWM 127": ("sv6-127", 1),
    "Brilliant Blender": ("sv8-164", 1),
    "Mysterious Treasure": ("sm6-113", 4),
    "Quick Ball": ("swsh1-179", 3),
    "Earthen Vessel": ("sv4-163", 2),
    "Arven": ("sv1-166", 2),
    "Crispin": ("sv7-133", 2),
    "Professor Burnet": ("swsh12tg-TG26", 1),
    "Serena": ("swsh12-164", 1),
    "Tate & Liza": ("sm7-148", 1),
    "Steven's Resolve": ("sm7-145", 1),
    "Guzma": ("sm3-115", 1),
    "Channeler": ("sm11-190", 1),
    "Gladion": ("sm4-95", 2),
    "Lusamine": ("sm4-96", 1),
    "Team Yell's Cheer": ("swsh9-149", 1),
    "Roseanne's Backup": ("swsh9-148", 1),
    "Professor Turo's Scenario": ("sv4-171", 1),
    "Forest Seal Stone": ("swsh12-156", 1),
    "Powerglass": ("sv6pt5-63", 1),
    "Field Blower": ("sm2-125", 1),
    "Chaotic Swell": ("sm12-187", 1),
    "Path to the Peak": ("swsh6-148", 1),
    "Hisuian Heavy Ball": ("swsh10-146", 1),
    "Grass Energy": ("energy-grass", 6),
    "Fire Energy": ("energy-fire", 3),
}


@contextmanager
def exclusive_lock(lock_path: Path) -> Iterator[None]:
    lock_path.parent.mkdir(parents=True, exist_ok=True)
    with lock_path.open("a+b") as lock_file:
        if os.name == "nt":
            import msvcrt

            lock_file.seek(0)
            lock_file.write(b"0")
            lock_file.flush()
            lock_file.seek(0)
            msvcrt.locking(lock_file.fileno(), msvcrt.LK_LOCK, 1)
            try:
                yield
            finally:
                lock_file.seek(0)
                msvcrt.locking(lock_file.fileno(), msvcrt.LK_UNLCK, 1)
        else:
            import fcntl

            fcntl.flock(lock_file.fileno(), fcntl.LOCK_EX)
            try:
                yield
            finally:
                fcntl.flock(lock_file.fileno(), fcntl.LOCK_UN)


def atomic_json_write(destination: Path, payload: object) -> None:
    content = json.dumps(payload, indent=2, ensure_ascii=False, sort_keys=True) + "\n"
    with exclusive_lock(destination.with_suffix(destination.suffix + ".lock")):
        with tempfile.NamedTemporaryFile(
            mode="w", encoding="utf-8", dir=destination.parent, prefix=destination.name + ".", suffix=".tmp", delete=False
        ) as temp_file:
            temp_file.write(content)
            temp_file.flush()
            os.fsync(temp_file.fileno())
            temp_path = Path(temp_file.name)
        try:
            os.replace(temp_path, destination)
        finally:
            if temp_path.exists():
                temp_path.unlink()


def read_all_cards(source: Path) -> list[dict[str, Any]]:
    cards: list[dict[str, Any]] = []
    if source.suffix.lower() == ".zip":
        with zipfile.ZipFile(source) as archive:
            for member in archive.namelist():
                if "/cards/en/" not in member or not member.endswith(".json"):
                    continue
                cards.extend(item for item in json.loads(archive.read(member)) if isinstance(item, dict) and "id" in item)
        return cards
    for file_path in source.glob("cards/en/*.json"):
        cards.extend(item for item in json.loads(file_path.read_text(encoding="utf-8")) if isinstance(item, dict) and "id" in item)
    return cards


def card_summary(card: dict[str, Any], copies: int) -> dict[str, Any]:
    return {
        "copies": copies,
        "id": card["id"],
        "name": card["name"],
        "supertype": card.get("supertype"),
        "subtypes": card.get("subtypes", []),
        "types": card.get("types", []),
        "hp": card.get("hp"),
        "evolvesFrom": card.get("evolvesFrom"),
        "rules": card.get("rules", []),
        "abilities": card.get("abilities", []),
        "attacks": card.get("attacks", []),
        "legalities": card.get("legalities", {}),
    }


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path, help="pokemon-tcg-data repository directory or ZIP")
    parser.add_argument("--out", type=Path, default=Path("data/card_audit.json"))
    args = parser.parse_args()

    by_id = {card["id"]: card for card in read_all_cards(args.source)}
    missing = [f"{name} ({card_id})" for name, (card_id, _) in REQUESTED.items() if card_id not in by_id and not card_id.startswith("energy-")]
    if missing:
        raise SystemExit("Missing requested card IDs: " + ", ".join(missing))

    records: dict[str, Any] = {}
    for name, (card_id, copies) in REQUESTED.items():
        if card_id.startswith("energy-"):
            records[name] = {"copies": copies, "id": card_id, "name": name, "category": "Basic Energy"}
        else:
            records[name] = card_summary(by_id[card_id], copies)

    category_totals = {"pokemon": 19, "trainers": 32, "energy": 9}
    payload = {
        "source": str(args.source),
        "deck_total": sum(copies for _, copies in REQUESTED.values()),
        "category_totals": category_totals,
        "records": records,
        "notes": [
            "The source corpus records paper legality. It does not encode the current Pokemon TCG Live client card pool.",
            "Live availability remains an explicit input assumption verified by the user-provided Live list, not a claim inferred from paper Expanded legality.",
        ],
    }
    if payload["deck_total"] != 60:
        raise SystemExit(f"Deck total is {payload['deck_total']}, expected 60")
    atomic_json_write(args.out, payload)
    print(f"Validated {payload['deck_total']} cards across {len(records)} named entries: {args.out}")


if __name__ == "__main__":
    main()
