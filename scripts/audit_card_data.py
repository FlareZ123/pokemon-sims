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
    "Erika's Invitation": ("sv3pt5-160", 1),
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

REPO_ROOT = Path(__file__).resolve().parents[1]
DECKLIST_PATH = REPO_ROOT / "data" / "decklist.json"
DECKLIST_NAME_ALIASES = {
    "Dialga-GX (Dragon)": "Dialga-GX",
    "Latias ex (Skyliner)": "Latias ex",
    "Oricorio GRI 55 (Vital Dance)": "Oricorio GRI 55",
}
SUPERTYPE_TO_CATEGORY = {"Pokémon": "pokemon", "Trainer": "trainers"}


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


def canonical_category_contract() -> tuple[dict[str, str], dict[str, int]]:
    # The repository decklist owns the expected group and count for each requested
    # entry. The audit must compare corpus supertypes with that specification:
    # https://github.com/FlareZ123/pokemon-sims/blob/main/data/decklist.json
    # https://github.com/FlareZ123/pokemon-sims/issues/691
    decklist = json.loads(DECKLIST_PATH.read_text(encoding="utf-8"))
    expected_by_name: dict[str, str] = {}
    expected_totals = {"pokemon": 0, "trainers": 0, "energy": 0}
    for category in expected_totals:
        entries = decklist.get(category)
        if not isinstance(entries, dict):
            raise SystemExit(f"Decklist category {category!r} must be an object")
        for deck_name, copies in entries.items():
            requested_name = DECKLIST_NAME_ALIASES.get(deck_name, deck_name)
            if requested_name not in REQUESTED:
                raise SystemExit(f"Decklist entry is not present in REQUESTED: {deck_name}")
            if requested_name in expected_by_name:
                raise SystemExit(f"Decklist entry appears more than once: {requested_name}")
            if REQUESTED[requested_name][1] != copies:
                raise SystemExit(
                    f"Decklist copies for {deck_name} are {copies}, expected {REQUESTED[requested_name][1]}"
                )
            expected_by_name[requested_name] = category
            expected_totals[category] += copies

    missing_groups = sorted(set(REQUESTED) - set(expected_by_name))
    if missing_groups:
        raise SystemExit("REQUESTED entries missing from decklist groups: " + ", ".join(missing_groups))
    if sum(expected_totals.values()) != decklist.get("total"):
        raise SystemExit("Decklist category totals do not match its declared total")
    return expected_by_name, expected_totals


def build_payload(source: Path, by_id: dict[str, dict[str, Any]]) -> dict[str, Any]:
    missing = [
        f"{name} ({card_id})"
        for name, (card_id, _) in REQUESTED.items()
        if card_id not in by_id and not card_id.startswith("energy-")
    ]
    if missing:
        raise SystemExit("Missing requested card IDs: " + ", ".join(missing))

    expected_by_name, expected_totals = canonical_category_contract()
    cards: dict[str, dict[str, Any]] = {}
    observed_totals = {"pokemon": 0, "trainers": 0, "energy": 0}
    for name, (card_id, copies) in REQUESTED.items():
        if card_id.startswith("energy-"):
            cards[name] = {"copies": copies, "id": card_id, "name": name, "supertype": "Energy"}
            observed_totals["energy"] += copies
            continue
        summary = card_summary(by_id[card_id], copies)
        cards[name] = summary
        supertype = summary.get("supertype")
        category = SUPERTYPE_TO_CATEGORY.get(supertype)
        if category is None:
            raise SystemExit(f"Unsupported supertype for {name}: {supertype!r}")
        expected_category = expected_by_name[name]
        if category != expected_category:
            raise SystemExit(
                f"Unexpected supertype for {name}: {supertype!r} maps to {category!r}, expected {expected_category!r}"
            )
        observed_totals[category] += copies

    if observed_totals != expected_totals:
        raise SystemExit(
            f"Derived category totals {observed_totals!r} do not match decklist totals {expected_totals!r}"
        )
    return {"source": str(source), "category_totals": observed_totals, "cards": cards}


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("source", type=Path, help="pokemon-tcg-data checkout directory or ZIP archive")
    parser.add_argument("--out", type=Path, default=Path("data/card_audit.json"))
    args = parser.parse_args()

    all_cards = read_all_cards(args.source)
    by_id = {card["id"]: card for card in all_cards}
    payload = build_payload(args.source, by_id)
    args.out.parent.mkdir(parents=True, exist_ok=True)
    atomic_json_write(args.out, payload)
    print(json.dumps({"output": str(args.out), "cards": len(payload["cards"]), "category_totals": payload["category_totals"]}))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
