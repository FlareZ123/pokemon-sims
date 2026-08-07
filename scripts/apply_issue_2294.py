from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
path = ROOT / "src/trace_engine_v2/part_012.inc"
text = path.read_text(encoding="utf-8")
anchor = "  bool play_serena(const bool allow_zero_draw_payload_completion = false) {\n"
helper = r'''  bool issue_2294_arven_latias_blender_beats_serena() const {
    if (scenario_.dci != DciProfile::MatchupFlexJit || !scenario_.going_first ||
        state_.turn != 4 || !prizes_known() || !supporter_allowed() ||
        item_locked() || state_.manual_energy_used || state_.retreat_used ||
        !state_.active || state_.active->card != Card::TapuLeleGX ||
        bench_space() == 0 || !ability_available_for_pokemon(Card::LatiasEx) ||
        hand_count(Card::Arven) == 0 || hand_count(Card::Serena) == 0 ||
        hand_count(Card::BrilliantBlender) == 0 || hand_count(Card::Fire) == 0 ||
        hand_count(Card::RegidragoVstar) < 2 ||
        deck_count_after_search_started(Card::QuickBall) == 0 ||
        deck_count_after_search_started(Card::LatiasEx) == 0 ||
        !payload_might_be_in_deck()) {
      return false;
    }

    const bool prior_turn_gg_regi = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
              pokemon.entered_turn < state_.turn && pokemon.grass >= 2 &&
              pokemon.fire == 0;
        });
    if (!prior_turn_gg_regi) return false;

    // Arven uniquely solves the Active-position axis this turn: it finds Quick Ball,
    // the second held VSTAR becomes route-surplus after evolution and pays that Item,
    // Quick Ball finds Latias ex, and Skyliner supplies the free promotion. Held
    // Brilliant Blender independently supplies the JIT payload, so Serena would spend
    // the only Supporter action on an axis that is already covered:
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Latias ex / Skyliner: https://api.pokemontcg.io/v2/cards/sv8-76
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
    // Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
    // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Item, discard, Bench, evolution, Ability and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, matchup-flex JIT, dynamic DCI/AMR, Supporter contention and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2294
    return true;
  }

'''
if helper not in text:
    if text.count(anchor) != 1:
        raise RuntimeError("#2294 Serena anchor mismatch")
    text = text.replace(anchor, helper + anchor, 1)
old = """  bool play_serena(const bool allow_zero_draw_payload_completion = false) {\n    if (!supporter_allowed() || hand_count(Card::Serena) == 0) return false;\n"""
new = """  bool play_serena(const bool allow_zero_draw_payload_completion = false) {\n    if (issue_2294_arven_latias_blender_beats_serena()) {\n      // Preserve the Supporter for the complete same-turn Arven mobility route.\n      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2294\n      return false;\n    }\n    if (!supporter_allowed() || hand_count(Card::Serena) == 0) return false;\n"""
if new not in text:
    if text.count(old) != 1:
        raise RuntimeError("#2294 Serena preemption insertion mismatch")
    text = text.replace(old, new, 1)
path.write_text(text, encoding="utf-8")

cmake = ROOT / "CMakeLists.txt"
c = cmake.read_text(encoding="utf-8")
if "trace_issue_2294_matchup_first_seed8" not in c:
    c += r'''

# Held Arven -> Quick Ball -> Latias plus held Blender is a deterministic T4
# matchup-flex-JIT finish and must keep Serena from spending the Supporter first.
# Arven: https://api.pokemontcg.io/v2/cards/sv1-166
# Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
# Latias ex: https://api.pokemontcg.io/v2/cards/sv8-76
# Brilliant Blender: https://api.pokemontcg.io/v2/cards/sv8-164
# Serena: https://api.pokemontcg.io/v2/cards/swsh12-164
# Core rules: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2294
add_test(NAME trace_issue_2294_matchup_first_seed8
  COMMAND regidrago_sim --simulate-this --scenario matchup-flex-jit/go-first --seed 8 --require-ready-by 4)
'''
    cmake.write_text(c, encoding="utf-8")
