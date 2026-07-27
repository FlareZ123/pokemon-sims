from pathlib import Path

path = Path("tests/issue_1118_multi_deck_secret_box_tests.cpp")
source = path.read_text(encoding="utf-8")
anchor = '''      if (!sim::EngineTestAccess::play_secret_box(payable.engine)) {
        throw std::runtime_error(
            "The issue-1619 complete route did not admit three costs.");
      }
      const sim::State& after = sim::EngineTestAccess::state(payable.engine);
      if (std::count(after.discard.begin(), after.discard.end(),
                     sim::Card::RegidragoVstar) != 1 ||
          !contains(after.discard, sim::Card::ForestOfVitality) ||
          !contains(after.discard, sim::Card::RegidragoV) ||
          std::count(after.hand.begin(), after.hand.end(),
                     sim::Card::RegidragoVstar) != 1 ||
          !contains(after.hand, sim::Card::EarthenVessel) ||
          contains(after.hand, sim::Card::MysteriousTreasure)) {
        throw std::runtime_error(
            "The issue-1619 route did not preserve one VSTAR and select the direct Vessel continuation.");
      }

'''
replacement = '''      sim::EngineTestAccess::run_secret_box_turn(payable.engine);
      const sim::State& after = sim::EngineTestAccess::state(payable.engine);
      const sim::TrialOutcome& outcome =
          sim::EngineTestAccess::outcome(payable.engine);
      // The complete turn must pay Secret Box with only the replaced copies, search
      // Vessel plus Dawn, discard the Dawn-searched Dragon to Vessel, attach Fire,
      // evolve the established Regidrago V, and finish GGF with a current-turn payload:
      // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
      // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
      // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
      // Dragapult ex: https://api.pokemontcg.io/v2/cards/sv6-130
      // Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-2
      // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
      // Core rules: https://www.pokemon.com/us/pokemon-tcg/rules
      // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1619
      if (!after.active || after.active->card != sim::Card::RegidragoVstar ||
          after.active->grass < 2 || after.active->fire < 1 ||
          !contains(after.discard, sim::Card::Dragapult) ||
          std::count(after.discard.begin(), after.discard.end(),
                     sim::Card::RegidragoVstar) != 1 ||
          !contains(after.discard, sim::Card::ForestOfVitality) ||
          !contains(after.discard, sim::Card::RegidragoV) ||
          !outcome.used_secret_box || !outcome.used_exploding_energy) {
        throw std::runtime_error(
            "The issue-1619 Secret Box-Dawn-Vessel route did not complete on T2.");
      }

'''
if source.count(anchor) != 1:
    raise SystemExit(f"issue-1619 full-route test anchor count: {source.count(anchor)}")
path.write_text(source.replace(anchor, replacement, 1), encoding="utf-8")
