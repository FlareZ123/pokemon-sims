from __future__ import annotations

from pathlib import Path


def main() -> int:
    path = Path("audit/issue_1343_arven_held_payload_probe.cpp")
    text = path.read_text(encoding="utf-8")
    old = """  state.hand = {sim::Card::Arven, sim::Card::UltraBall,
                sim::Card::MegaDragonite, sim::Card::Grass};"""
    new = """  state.hand = {sim::Card::Arven, sim::Card::UltraBall,
                sim::Card::MegaDragonite, sim::Card::RegidragoVstar};"""
    if old not in text:
        return 0
    if text.count(old) != 1:
        raise RuntimeError("Unexpected payable Ultra Ball probe shape")
    text = text.replace(old, new, 1)
    old_comment = """  // Ultra Ball can pay the Dragon plus one other legal cost and search a Pokémon:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/sv1-166"""
    new_comment = """  // Ultra Ball can pay the Dragon plus a redundant held VSTAR and search a
  // Pokémon. The Active VSTAR makes the extra copy policy-legal to discard:
  // https://api.pokemontcg.io/v2/cards/swsh12pt5-146
  // https://api.pokemontcg.io/v2/cards/me2pt5-152
  // https://api.pokemontcg.io/v2/cards/swsh12-136
  // https://api.pokemontcg.io/v2/cards/sv1-166"""
    if text.count(old_comment) != 1:
        raise RuntimeError("Unexpected payable Ultra Ball comment shape")
    path.write_text(text.replace(old_comment, new_comment, 1), encoding="utf-8")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
