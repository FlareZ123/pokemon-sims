from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = root / "src/trace_engine_v2/part_issue_1516_quick_ball_tapu_crispin_override.inc"
text = source.read_text(encoding="utf-8")
old = """        hand_count(Card::QuickBall) == 0 || hand_count(Card::Fire) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::MegaDragonite) == 0 || payload_ready()) {
      return false;
    }

    const bool prior_turn_gg_regidrago = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          return pokemon.card == Card::RegidragoV &&
                 pokemon.entered_turn < state_.turn &&
                 pokemon.grass >= 2 && pokemon.fire == 0;
        });
    if (!prior_turn_gg_regidrago) return false;
"""
new = """        hand_count(Card::QuickBall) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::MegaDragonite) == 0 || payload_ready()) {
      return false;
    }

    const bool prior_turn_regidrago_one_basic_from_apex = std::any_of(
        state_.bench.begin(), state_.bench.end(), [this](const Pokemon& pokemon) {
          if (pokemon.card != Card::RegidragoV ||
              pokemon.entered_turn >= state_.turn) return false;
          for (const Card basic : {Card::Grass, Card::Fire}) {
            if (hand_count(basic) == 0) continue;
            Pokemon projected = pokemon;
            if (attach_energy_card(projected, basic) &&
                pays_apex_energy_cost(projected)) {
              // Double Dragon Energy: https://www.pokemon.com/us/pokemon-tcg/pokemon-cards/series/xy6/97/
              // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
              // Energy attachment rule: https://www.pokemon.com/us/pokemon-tcg/rules
              // DDE semantic contract: https://github.com/FlareZ123/pokemon-sims/issues/2238
              // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2438
              return true;
            }
          }
          return false;
        });
    if (!prior_turn_regidrago_one_basic_from_apex) return false;
"""
if text.count(old) != 1:
    raise RuntimeError(f"expected one target block, found {text.count(old)}")
source.write_text(text.replace(old, new, 1), encoding="utf-8")
