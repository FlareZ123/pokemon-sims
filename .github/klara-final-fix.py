from pathlib import Path


def replace_once(path: str, old: str, new: str) -> None:
    file = Path(path)
    text = file.read_text()
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{path}: expected one replacement, found {count}")
    file.write_text(text.replace(old, new, 1))


# Preserve Klara's wrapper position while retaining every newer main override.
path = Path("src/regidrago_sim.cpp")
text = path.read_text()
if "part_klara_recovery_override.inc" not in text:
    old = '''#include "trace_engine_v2/part_roseanne_multimode_override.inc"
#define choose_supporter choose_supporter_issue1070_original
#include "trace_engine_v2/part_turo_oricorio_override.inc"
#undef choose_supporter
#define choose_supporter choose_supporter_issue1209_original
#include "trace_engine_v2/part_issue_1070_tate_after_vstar_search_override.inc"
'''
    new = '''#include "trace_engine_v2/part_roseanne_multimode_override.inc"
#define choose_supporter choose_supporter_klara_original
#include "trace_engine_v2/part_turo_oricorio_override.inc"
#undef choose_supporter
#define choose_supporter choose_supporter_issue1070_original
#include "trace_engine_v2/part_klara_recovery_override.inc"
#undef choose_supporter
#define choose_supporter choose_supporter_issue1209_original
#include "trace_engine_v2/part_issue_1070_tate_after_vstar_search_override.inc"
'''
    if text.count(old) != 1:
        raise SystemExit("src/regidrago_sim.cpp: current-main Supporter wrapper block not found")
    path.write_text(text.replace(old, new, 1))

path = Path("src/trace_engine_v2/part_007.inc")
text = path.read_text()
if "Card::RoseannesBackup, Card::Klara, Card::ProfessorTuro" not in text:
    replace_once(
        str(path),
        "Card::RoseannesBackup, Card::ProfessorTuro",
        "Card::RoseannesBackup, Card::Klara, Card::ProfessorTuro",
    )

# Merge source registries additively rather than choosing either side.
saved = Path("/tmp/klara-rules-traceability.md").read_text().splitlines()
path = Path("docs/RULES_TRACEABILITY.md")
text = path.read_text()
if "`R-KLARA-01`" not in text:
    row = next(line for line in saved if "`R-KLARA-01`" in line)
    marker = next(line for line in text.splitlines() if "`R-LUSAMINE-01`" in line)
    path.write_text(text.replace(marker, row + "\n" + marker, 1))

saved = Path("/tmp/klara-rule-sources.md").read_text().splitlines()
path = Path("docs/RULE_SOURCES.md")
text = path.read_text()
additions = [
    next(line for line in saved if line.startswith("| Klara |")),
    next(line for line in saved if line.startswith("| Eri,")),
    next(line for line in saved if line.startswith("| Roseanne's Backup,")),
]
if additions[0] not in text:
    marker = next(line for line in text.splitlines() if line.startswith("| Roseanne's Backup |"))
    path.write_text(text.replace(marker, "\n".join(additions) + "\n" + marker, 1))

replace_once(
    "src/trace_engine_v2/part_000.inc",
    "#include <map>\n#include <numeric>",
    "#include <map>\n#include <memory>\n#include <numeric>",
)

replace_once(
    "src/trace_engine_v2/part_003.inc",
    '''  Engine(const Scenario& scenario, const DeckRecipe& recipe, std::mt19937_64& rng, TraceLog* trace = nullptr)
      : scenario_(scenario), recipe_(recipe), rng_(rng), trace_(trace) {}
''',
    '''  Engine(const Scenario& scenario, const DeckRecipe& recipe, std::mt19937_64& rng, TraceLog* trace = nullptr)
      : scenario_(scenario), recipe_(recipe), rng_(rng), trace_(trace) {}

  Engine(const Scenario& scenario, DeckRecipe&& recipe, std::mt19937_64& rng,
         TraceLog* trace = nullptr)
      : scenario_(scenario),
        owned_recipe_(std::make_shared<DeckRecipe>(std::move(recipe))),
        recipe_(*owned_recipe_),
        rng_(rng),
        trace_(trace) {
    // Synthetic fixtures may pass a temporary recipe. Own that rvalue so later
    // route projections and recipe inventory checks cannot retain a dangling
    // reference. Shared ownership also keeps default Engine copies safe:
    // https://eel.is/c++draft/class.temporary
    // https://eel.is/c++draft/util.smartptr.shared
    // Sanitizer evidence: https://github.com/FlareZ123/pokemon-sims/pull/1774
  }
''',
)

replace_once(
    "src/trace_engine_v2/part_003.inc",
    '''  const Scenario& scenario_;
  const DeckRecipe& recipe_;
''',
    '''  const Scenario& scenario_;
  std::shared_ptr<const DeckRecipe> owned_recipe_;
  const DeckRecipe& recipe_;
''',
)

replace_once(
    "src/trace_engine_v2/part_klara_recovery_override.inc",
    '''  void choose_supporter() {
    if (!supporter_allowed() || !setup_axis_missing()) return;
    if (play_klara_recovery()) return;
    choose_supporter_klara_original();
  }
''',
    '''  void choose_supporter() {
    if (!supporter_allowed()) return;
    if (setup_axis_missing() && play_klara_recovery()) return;

    // Klara's setup-only admission must not hide existing Supporter routes that
    // repair the Active position after another Regidrago is already complete,
    // including Professor Turo plus Oricorio Energy compression:
    // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145
    // Professor Turo's Scenario: https://api.pokemontcg.io/v2/cards/sv4-171
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Official Supporter procedure: https://www.pokemon.com/us/pokemon-tcg/rules
    // Sanitizer regression: https://github.com/FlareZ123/pokemon-sims/pull/1774
    choose_supporter_klara_original();
  }
''',
)

replace_once(
    "src/trace_engine_v2/part_issue_1209_treasure_tapu_crispin_override.inc",
    '''        hand_count(Card::MysteriousTreasure) >= 2 &&
        hand_count(Card::RoseannesBackup) > 0 &&
        hand_count(Card::ForestSealStone) > 0 &&
''',
    '''        hand_count(Card::MysteriousTreasure) >= 2 &&
        (hand_count(Card::RoseannesBackup) > 0 ||
         hand_count(Card::Klara) > 0) &&
        hand_count(Card::ForestSealStone) > 0 &&
''',
)

replace_once(
    "src/trace_engine_v2/part_issue_1209_treasure_tapu_crispin_override.inc",
    '''    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
''',
    '''    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Klara, the registered shell's second printed discard cost: https://api.pokemontcg.io/v2/cards/swsh6-145
    // Roseanne's Backup, retained legacy recipe support: https://api.pokemontcg.io/v2/cards/swsh9-148
    // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145
''',
)

path = Path("tests/issue_1236_vessel_before_steven_tests.cpp")
text = path.read_text()
substitutions = [
    ("sim::Card::LatiasEx, sim::Card::RoseannesBackup,", "sim::Card::LatiasEx, sim::Card::Klara,"),
    ("with Roseanne's Backup and searches Regidrago VSTAR.", "with Klara and searches Regidrago VSTAR."),
    ("// Roseanne's Backup: https://api.pokemontcg.io/v2/cards/swsh9-148", "// Klara: https://api.pokemontcg.io/v2/cards/swsh6-145"),
    (
        "sim::State no_roseanne = route_state();\n  erase_one(no_roseanne.hand, sim::Card::RoseannesBackup);\n  blocked(no_roseanne, scenario(), 1236004,",
        "sim::State no_supporter_cost = route_state();\n  erase_one(no_supporter_cost.hand, sim::Card::Klara);\n  blocked(no_supporter_cost, scenario(), 1236004,",
    ),
]
for old, new in substitutions:
    if text.count(old) != 1:
        raise SystemExit(f"tests/issue_1236_vessel_before_steven_tests.cpp: replacement count for {old!r} was {text.count(old)}")
    text = text.replace(old, new, 1)
path.write_text(text)

path = Path("tests/issue_1109_late_steven_blender_tests.cpp")
text = path.read_text()
old = '''  const bool used_steven_by_t3 =
      trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-STEVEN-01") ||
      trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-STEVEN-01");
  expect(used_steven_by_t3, "Seed 143 must play Steven by T3.");
'''
new = '''  const bool used_steven_by_t3 =
      trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-STEVEN-01") ||
      trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-STEVEN-01");
  const bool used_klara_by_t3 =
      trace_contains(trace, "T2 | PLAY SUPPORTER | rules: R-KLARA-01") ||
      trace_contains(trace, "T3 | PLAY SUPPORTER | rules: R-KLARA-01");
  expect(used_steven_by_t3 || used_klara_by_t3,
         "Seed 143 must use Steven or the earlier legal Klara route by T3.");
'''
if text.count(old) != 1:
    raise SystemExit("tests/issue_1109_late_steven_blender_tests.cpp: integration assertion not found exactly once")
text = text.replace(old, new, 1)
text = text.replace(
    '''  // The complete Steven route must outrank spending Gladion on a redundant Basic.
  // A later policy may legally discover an earlier Steven plus Burnet continuation,
  // so this integration regression preserves the route and its original T4 deadline
  // while the exact-state test above continues to cover held Blender admission:
''',
    '''  // The complete route must outrank spending Gladion on a redundant Basic.
  // The registered shell may now use Klara for an earlier legal T3 completion, while
  // the exact-state test above continues to cover held-Blender Steven admission:
''',
    1,
)
text = text.replace(
    "  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145\n",
    "  // Steven's Resolve: https://api.pokemontcg.io/v2/cards/sm7-145\n  // Klara: https://api.pokemontcg.io/v2/cards/swsh6-145\n",
    1,
)
path.write_text(text)
