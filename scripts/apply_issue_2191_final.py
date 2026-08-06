from pathlib import Path

path = Path("src/trace_engine_v2/part_pokemon_communication.inc")
source = path.read_text(encoding="utf-8")
old = """    const bool mysterious_target = is_dragon_or_psychic(returned) ||
        std::any_of(recipe_.begin(), recipe_.end(), [this](const auto& entry) {
          return is_dragon_or_psychic(entry.first) &&
                 might_be_unseen(entry.first);
        });
    const bool quick_target = is_basic(returned) ||
        std::any_of(recipe_.begin(), recipe_.end(), [this](const auto& entry) {
          return is_basic(entry.first) && might_be_unseen(entry.first);
        });
"""
new = """    // At K1, the exchange above has already constructed the exact post-search
    // deck. Later search Items must inspect that physical deck, including off-recipe
    // Appletun and Mawile-GX targets. True K0 retains fixed-list plausibility:
    // https://api.pokemontcg.io/v2/cards/sv8-140
    // https://api.pokemontcg.io/v2/cards/sm11-141
    // https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // https://github.com/FlareZ123/pokemon-sims/issues/2191
    const bool mysterious_target = is_dragon_or_psychic(returned) ||
        (prizes_known()
             ? std::any_of(state_.deck.begin(), state_.deck.end(),
                           is_dragon_or_psychic)
             : std::any_of(recipe_.begin(), recipe_.end(),
                           [this](const auto& entry) {
                             return is_dragon_or_psychic(entry.first) &&
                                    might_be_unseen(entry.first);
                           }));
    const bool quick_target = is_basic(returned) ||
        (prizes_known()
             ? std::any_of(state_.deck.begin(), state_.deck.end(), is_basic)
             : std::any_of(recipe_.begin(), recipe_.end(),
                           [this](const auto& entry) {
                             return is_basic(entry.first) &&
                                    might_be_unseen(entry.first);
                           }));
"""
if source.count(old) != 1:
    raise SystemExit("issue-2191 anchor count was not exactly one")
path.write_text(source.replace(old, new, 1), encoding="utf-8")
