from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
path = ROOT / "src/trace_engine_v2/part_007.inc"
text = path.read_text(encoding="utf-8")
anchor = "  Card choose_supporter_after_search_started() const {\n"
helper = r'''  bool issue_2291_wonder_tag_arven_fss_vessel_route() const {
    if (scenario_.dci != DciProfile::StrictJit || !scenario_.going_first ||
        state_.turn != 4 || !prizes_known() || !supporter_allowed() ||
        item_locked() || state_.manual_energy_used || state_.vstar_power_used ||
        !state_.active || state_.active->card != Card::RegidragoV ||
        state_.active->entered_turn >= state_.turn ||
        state_.active->grass != 1 || state_.active->fire != 1 ||
        state_.active->tool != Tool::None ||
        deck_count_after_search_started(Card::Arven) == 0 ||
        deck_count_after_search_started(Card::ForestSealStone) == 0 ||
        deck_count_after_search_started(Card::EarthenVessel) == 0 ||
        deck_count_after_search_started(Card::RegidragoVstar) == 0 ||
        deck_count_after_search_started(Card::Grass) == 0 ||
        !in_play(Card::TapuLeleGX) ||
        !ability_available_for_pokemon(Card::TapuLeleGX) ||
        crispin_can_advance_energy_axis() ||
        !std::any_of(state_.hand.begin(), state_.hand.end(), is_payload)) {
      return false;
    }

    // Wonder Tag should search Arven when K1 proves Crispin cannot produce its
    // two-different-Energy attachment, while Arven compresses the missing axes:
    // FSS -> Star Alchemy -> VSTAR, and Vessel -> Grass with a held Dragon paying
    // the Vessel discard cost and becoming the same-turn strict-JIT payload.
    // Tapu Lele-GX / Wonder Tag: https://api.pokemontcg.io/v2/cards/sm2-60
    // Crispin: https://api.pokemontcg.io/v2/cards/sv7-133
    // Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
    // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago V/VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Core Supporter, Tool, VSTAR Power, Item, discard, search, attachment and evolution procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K1, strict-JIT, DCI/AMR, Supporter contention and earliest-ready policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2291
    return true;
  }

'''
if helper not in text:
    if text.count(anchor) != 1:
        raise RuntimeError("#2291 selector anchor mismatch")
    text = text.replace(anchor, helper + anchor, 1)
old = """  Card choose_supporter_after_search_started() const {\n    if (issue_1797_wonder_tag_steven_route_available()) {\n"""
new = """  Card choose_supporter_after_search_started() const {\n    if (issue_2291_wonder_tag_arven_fss_vessel_route()) {\n      // Arven completes the live K1 FSS/Vessel route when Crispin is energy-dead.\n      // Confirmed selector bug: https://github.com/FlareZ123/pokemon-sims/issues/2291\n      return Card::Arven;\n    }\n    if (issue_1797_wonder_tag_steven_route_available()) {\n"""
if new not in text:
    if text.count(old) != 1:
        raise RuntimeError("#2291 selector insertion mismatch")
    text = text.replace(old, new, 1)
path.write_text(text, encoding="utf-8")

cmake = ROOT / "CMakeLists.txt"
c = cmake.read_text(encoding="utf-8")
if "trace_issue_2291_strict_first_seed275" not in c:
    c += r'''

# Wonder Tag -> Arven -> FSS/Vessel is the deterministic strict-JIT T4 finish
# when K1 proves Crispin cannot produce its two-different-Energy attachment.
# Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
# Arven: https://api.pokemontcg.io/v2/cards/sv1-166
# Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
# Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
# Crispin ruling: https://compendium.pokegym.net/category/5-trainers/crispin/
# Core rules: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
# Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2291
add_test(NAME trace_issue_2291_strict_first_seed275
  COMMAND regidrago_sim --simulate-this --scenario strict-jit/go-first --seed 275 --require-ready-by 4)
'''
    cmake.write_text(c, encoding="utf-8")
