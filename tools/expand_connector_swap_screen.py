from pathlib import Path

source = Path("tools/regidrago_connector_swap_screen.cpp")
output = Path("tools/regidrago_connector_swap_screen_full.cpp")
text = source.read_text(encoding="utf-8")

cut_anchor = """      sim::Card::ProfessorBurnet,     // Deck -> discard payload search: https://api.pokemontcg.io/v2/cards/swsh12tg-TG26\n"""
cut_extra = """      sim::Card::Oricorio,             // Vital Dance -> Basic Energy search: https://api.pokemontcg.io/v2/cards/sm2-55\n      sim::Card::BrilliantBlender,     // Deck -> discard payload search: https://api.pokemontcg.io/v2/cards/sv8-164\n"""
if cut_anchor not in text:
    raise SystemExit("connector cut anchor disappeared")
text = text.replace(cut_anchor, cut_anchor + cut_extra, 1)

add_anchor = """      sim::Card::BattleVipPass,\n"""
add_extra = """      sim::Card::Oricorio,\n      sim::Card::SecretBox,\n"""
if add_anchor not in text:
    raise SystemExit("connector add anchor disappeared")
text = text.replace(add_anchor, add_anchor + add_extra, 1)

output.write_text(text, encoding="utf-8")
