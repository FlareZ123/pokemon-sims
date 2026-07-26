import os
import subprocess
from pathlib import Path


SOURCE_BRANCH = "origin/fix/issue-1516-qb-tapu-crispin-v2"


def atomic_write(path: Path, content: str) -> None:
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(content, encoding="utf-8")
    os.replace(temporary, path)


def source_file(path: str) -> str:
    return subprocess.check_output(
        ["git", "show", f"{SOURCE_BRANCH}:{path}"],
        text=True,
        encoding="utf-8",
    )


sim_path = Path("src/regidrago_sim.cpp")
sim_source = sim_path.read_text(encoding="utf-8")
old_include = '#include "trace_engine_v2/part_issue_1476_redundant_burnet_route_override.inc"'
new_include = (
    '#define play_quick_ball play_quick_ball_issue1516_original\n'
    '#include "trace_engine_v2/part_issue_1476_redundant_burnet_route_override.inc"\n'
    '#undef play_quick_ball\n'
    '#include "trace_engine_v2/part_issue_1516_quick_ball_tapu_crispin_override.inc"'
)
if old_include in sim_source:
    if sim_source.count(old_include) != 1:
        raise SystemExit("Expected one issue-1476 include anchor")
    atomic_write(sim_path, sim_source.replace(old_include, new_include, 1))
elif new_include not in sim_source:
    raise SystemExit("Issue-1516 include composition is neither original nor applied")

base_path = Path(
    "src/trace_engine_v2/part_issue_991_wonder_tag_burnet_legacy_star_override_base.inc"
)
base = base_path.read_text(encoding="utf-8")
old_axis = '''    const bool held_gladion_has_known_axis = hand_count(Card::Gladion) > 0 &&
        ((need_regi() && bench_space() > 0 &&
          prize_count_after_reveal(Card::RegidragoV) > 0) ||
         (need_vstar() && prize_count_after_reveal(Card::RegidragoVstar) > 0) ||
         prize_count_after_reveal(Card::Grass) > 0 ||
         prize_count_after_reveal(Card::Fire) > 0);
'''
new_axis = '''    constexpr std::array<Card, 5> kAcceptedPayloads{
        Card::Dragapult, Card::MegaDragonite, Card::DialgaGX,
        Card::GoodraVstar, Card::Appletun};
    const bool known_prized_payload_axis =
        scenario_.dci == DciProfile::NoDiscardControl &&
        std::any_of(kAcceptedPayloads.begin(), kAcceptedPayloads.end(),
                    [this](const Card card) {
                      return prize_count_after_reveal(card) > 0;
                    });
    const bool held_gladion_has_known_axis = hand_count(Card::Gladion) > 0 &&
        ((need_regi() && bench_space() > 0 &&
          prize_count_after_reveal(Card::RegidragoV) > 0) ||
         (need_vstar() && prize_count_after_reveal(Card::RegidragoVstar) > 0) ||
         prize_count_after_reveal(Card::Grass) > 0 ||
         prize_count_after_reveal(Card::Fire) > 0 ||
         known_prized_payload_axis);
'''
if old_axis in base:
    if base.count(old_axis) != 1:
        raise SystemExit("Expected one duplicate-Crispin Gladion-axis block")
    base = base.replace(old_axis, new_axis, 1)
elif new_axis not in base:
    raise SystemExit("Issue-1516 Gladion-axis block is neither original nor applied")

old_sources = '''    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // One Supporter per turn: https://www.pokemon.com/us/pokemon-tcg/rules
    // Route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
'''
new_sources = '''    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Accepted Apex Dragon payloads: https://api.pokemontcg.io/v2/cards/sv6-130 https://api.pokemontcg.io/v2/cards/me2pt5-152 https://api.pokemontcg.io/v2/cards/sm5-100 https://api.pokemontcg.io/v2/cards/swsh11-136 https://api.pokemontcg.io/v2/cards/sv8-140
    // One Supporter per turn: https://www.pokemon.com/us/pokemon-tcg/rules
    // No-control payload banking and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed payload-axis extension: https://github.com/FlareZ123/pokemon-sims/issues/1516
'''
if old_sources in base:
    if base.count(old_sources) != 1:
        raise SystemExit("Expected one duplicate-Crispin source block")
    base = base.replace(old_sources, new_sources, 1)
elif new_sources not in base:
    raise SystemExit("Issue-1516 source block is neither original nor applied")
atomic_write(base_path, base)

copied_paths = (
    "src/trace_engine_v2/part_issue_1516_quick_ball_tapu_crispin_override.inc",
    "tests/issue_1516_quick_ball_tapu_crispin_tests.cpp",
)
for copied_path in copied_paths:
    atomic_write(Path(copied_path), source_file(copied_path))

required_urls = (
    "https://api.pokemontcg.io/v2/cards/swsh1-179",
    "https://api.pokemontcg.io/v2/cards/sm2-60",
    "https://api.pokemontcg.io/v2/cards/sv7-133",
    "https://api.pokemontcg.io/v2/cards/sm4-95",
    "https://www.pokemon.com/us/pokemon-tcg/rules",
    "https://github.com/FlareZ123/pokemon-sims/issues/1516",
)
combined = base + "\n" + source_file(copied_paths[0]) + "\n" + source_file(copied_paths[1])
missing = [url for url in required_urls if url not in combined]
if missing:
    raise SystemExit(f"Missing direct source URLs: {missing}")
