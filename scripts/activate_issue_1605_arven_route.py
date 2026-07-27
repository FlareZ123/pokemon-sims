from pathlib import Path
from textwrap import dedent

path = Path("src/trace_engine_v2/part_012_arven_fss_blender_contention_override.inc")
source = path.read_text(encoding="utf-8")
anchor = "  bool play_arven() {\n"
insert = dedent(
    '''\
      bool play_issue_1605_arven_redundant_payload_route() {
        if (!issue_1605_arven_crobat_route_available()) return false;

        // This decision is made from the public K0 hand, board, lock state, and fixed
        // deck recipe. Arven then performs the real legal deck inspection. Prefer a
        // physical Quick Ball, then Mysterious Treasure, and voluntarily fail the Tool
        // search because Forest Seal Stone is already held. The route flag is enabled
        // only when the inspected deck actually supplies the one-discard Item:
        // Arven: https://api.pokemontcg.io/v2/cards/sv1-166
        // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
        // Mysterious Treasure: https://api.pokemontcg.io/v2/cards/sm6-113
        // Crobat V / Dark Asset: https://api.pokemontcg.io/v2/cards/swsh3-104
        // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
        // Mega Dragonite ex: https://api.pokemontcg.io/v2/cards/me2pt5-152
        // Hisuian Goodra VSTAR: https://api.pokemontcg.io/v2/cards/swsh11-136
        // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
        // Core Supporter, Item, discard, search, Bench, Tool, and Ability procedure: https://www.pokemon.com/us/pokemon-tcg/rules
        // Typed searches may voluntarily fail: https://compendium.pokegym.net/compendium-bw.html#trainers-in-general
        // K0/K1, dynamic DCI, UDP, and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
        // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/1605
        remove_one(state_.hand, Card::Arven);
        state_.discard.push_back(Card::Arven);
        state_.supporter_used = true;
        record_deck_search_knowledge("Arven issue-1605 route");

        std::optional<Card> found_item;
        if (deck_count_after_search_started(Card::QuickBall) > 0 &&
            move_deck_to_hand(Card::QuickBall)) {
          found_item = Card::QuickBall;
        } else if (deck_count_after_search_started(Card::MysteriousTreasure) > 0 &&
                   move_deck_to_hand(Card::MysteriousTreasure)) {
          found_item = Card::MysteriousTreasure;
        }
        issue_1605_arven_redundant_payload_route_ = found_item.has_value();
        shuffle(state_.deck);
        trace("PLAY SUPPORTER", "R-ARVEN-01; R-GAME-SUPPORTER; P-DCI-01; P-CONNECTOR-01",
              found_item
                  ? "Arven searched " + card_name(*found_item) +
                        " and preserved the held Forest Seal Stone for the Crobat continuation."
                  : "Arven legally inspected the deck, but no one-discard Regidrago search Item remained.");
        return true;
      }

      bool play_arven() {
        if (play_issue_1605_arven_redundant_payload_route()) return true;
    '''
)
if source.count(anchor) != 1:
    raise SystemExit(f"issue-1605 active Arven anchor count: {source.count(anchor)}")
path.write_text(source.replace(anchor, insert, 1), encoding="utf-8")
