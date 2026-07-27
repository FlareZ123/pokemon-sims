from pathlib import Path

patch_path = Path("scripts/apply_issue_1605_patch.py")
patch_source = patch_path.read_text(encoding="utf-8")
old = '''treasure_clear_anchor = (
    "    remove_one(state_.hand, Card::MysteriousTreasure);\\n"
    "    state_.discard.push_back(Card::MysteriousTreasure);\\n"
)
treasure_clear_insert = (
    "    remove_one(state_.hand, Card::MysteriousTreasure);\\n"
    "    issue_1605_arven_redundant_payload_route_ = false;\\n"
    "    state_.discard.push_back(Card::MysteriousTreasure);\\n"
)
'''
new = '''treasure_clear_anchor = (
    "    remove_one(state_.hand, Card::MysteriousTreasure);\\n"
    "    issue_1598_bank_prized_treasure_ = false;\\n"
    "    state_.discard.push_back(Card::MysteriousTreasure);\\n"
)
treasure_clear_insert = (
    "    remove_one(state_.hand, Card::MysteriousTreasure);\\n"
    "    issue_1598_bank_prized_treasure_ = false;\\n"
    "    issue_1605_arven_redundant_payload_route_ = false;\\n"
    "    state_.discard.push_back(Card::MysteriousTreasure);\\n"
)
'''
if patch_source.count(old) != 1:
    raise SystemExit(f"issue-1605 post-1598 adapter count: {patch_source.count(old)}")
patch_path.write_text(patch_source.replace(old, new, 1), encoding="utf-8")

activate_path = Path("scripts/activate_issue_1605_arven_route.py")
activate_source = activate_path.read_text(encoding="utf-8")
old_name = '"Arven searched " + card_name(*found_item) +\n'
new_name = '"Arven searched " + std::string(name(*found_item)) +\n'
if activate_source.count(old_name) != 1:
    raise SystemExit(f"issue-1605 card-name adapter count: {activate_source.count(old_name)}")
activate_path.write_text(activate_source.replace(old_name, new_name, 1), encoding="utf-8")
