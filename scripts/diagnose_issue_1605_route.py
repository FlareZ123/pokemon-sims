from pathlib import Path

path = Path("tests/issue_1605_arven_redundant_payload_tests.cpp")
source = path.read_text(encoding="utf-8")
source = source.replace("#include <algorithm>\n", "#include <algorithm>\n#include <iostream>\n", 1)
old = '''  static bool route(const Engine& engine) {
    return engine.issue_1605_arven_crobat_route_available();
  }
'''
new = '''  static bool route(const Engine& engine) {
    const auto& state = engine.state_;
    std::cerr
        << "supporter=" << engine.supporter_allowed()
        << " strict=" << (engine.scenario_.dci == DciProfile::StrictJit)
        << " go_second=" << !engine.scenario_.going_first
        << " turn=" << state.turn
        << " item_unlocked=" << !engine.item_locked()
        << " need_regi=" << engine.need_regi()
        << " bench=" << engine.bench_space()
        << " arven=" << engine.hand_count(Card::Arven)
        << " crobat=" << engine.hand_count(Card::CrobatV)
        << " fss=" << engine.hand_count(Card::ForestSealStone)
        << " vstar_unused=" << !state.vstar_power_used
        << " dark_unused=" << !state.dark_asset_used
        << " crobat_ability=" << engine.ability_available_for_pokemon(Card::CrobatV)
        << " hand=" << state.hand.size()
        << " regi_unseen=" << engine.might_be_unseen(Card::RegidragoV)
        << " qb_unseen=" << engine.might_be_unseen(Card::QuickBall)
        << " mt_unseen=" << engine.might_be_unseen(Card::MysteriousTreasure)
        << " redundant=" << engine.issue_1605_redundant_payload_cost().has_value()
        << '\\n';
    return engine.issue_1605_arven_crobat_route_available();
  }
'''
if source.count(old) != 1:
    raise SystemExit(f"diagnostic route anchor count: {source.count(old)}")
path.write_text(source.replace(old, new, 1), encoding="utf-8")
