from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
path = ROOT / "src/trace_engine_v2/part_007.inc"
text = path.read_text(encoding="utf-8")
anchor = "  Card choose_supporter_after_search_started() const {\n"
helper = r'''  bool issue_2293_wonder_tag_prized_latias_gladion_route() const {
    if (scenario_.dci != DciProfile::StrictJit || !scenario_.going_first ||
        state_.turn != 4 || !prizes_known() || !supporter_allowed() ||
        item_locked() || state_.manual_energy_used || state_.retreat_used ||
        !state_.active || state_.active->card != Card::Oricorio ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::LatiasEx) ||
        prize_count_after_reveal(Card::LatiasEx) == 0 ||
        prize_count_after_reveal(Card::HisuianHeavyBall) == 0 ||
        deck_count_after_search_started(Card::Gladion) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 || hand_count(Card::Fire) == 0 ||
        hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::MysteriousTreasure) < 2 ||
        deck_count_after_search_started(Card::DialgaGX) == 0) {
      return false;
    }

    const bool prior_turn_gg_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pokemon.grass >= 2 &&
              pokemon.fire == 0 && pokemon.tool == Tool::None;
        });
    if (!prior_turn_gg_regi) return false;

    // Wonder Tag may search Gladion, whose known Prize exchange recovers Latias ex.
    // Held Fire and Regidrago VSTAR complete GGF/evolution while Quick Ball plus two
    // Mysterious Treasure copies provide the already-public Dialga-GX JIT payload
    // chain, leaving Active position as the only Supporter-dependent axis:
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Oricorio: https://api.pokemontcg.io/v2/cards/sm2-55
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
    // Dialga-GX: https://api.pokemontcg.io/v2/cards/sm5-100
    // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Prize, Item, attachment, evolution, Ability and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, DCI/AMR, Supporter contention and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Opposite live-Crispin boundary: https://github.com/FlareZ123/pokemon-sims/issues/1870
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2293
    return true;
  }

'''
if helper not in text:
    if text.count(anchor) != 1:
        raise RuntimeError("#2293 supporter selector anchor mismatch")
    text = text.replace(anchor, helper + anchor, 1)

old = """  Card choose_supporter_after_search_started() const {\n    if (issue_1797_wonder_tag_steven_route_available()) {\n"""
new = """  Card choose_supporter_after_search_started() const {\n    if (issue_2293_wonder_tag_prized_latias_gladion_route()) {\n      // The known prized Latias is the one missing active-position connector;\n      // Gladion preserves the held Fire/evolution/payload channels this turn.\n      // Confirmed selector bug: https://github.com/FlareZ123/pokemon-sims/issues/2293\n      return Card::Gladion;\n    }\n    if (issue_1797_wonder_tag_steven_route_available()) {\n"""
if new not in text:
    if text.count(old) != 1:
        raise RuntimeError("#2293 selector insertion anchor mismatch")
    text = text.replace(old, new, 1)
path.write_text(text, encoding="utf-8")

cmake = ROOT / "CMakeLists.txt"
c = cmake.read_text(encoding="utf-8")
if "trace_issue_2293_strict_first_seed291" not in c:
    c += r'''

# K1 Wonder Tag must bridge through Gladion to the known prized Latias ex when
# the held Fire/evolution/payload chain already completes every other T4 axis.
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
# Gladion: https://api.pokemontcg.io/v2/cards/sm4-95
# Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
# Regidrago VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-136
# Core rules: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2293
add_test(NAME trace_issue_2293_strict_first_seed291
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 291 --require-ready-by 4)
'''
    cmake.write_text(c, encoding="utf-8")
