from pathlib import Path
import fcntl
import os
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def atomic_write(path: Path, text: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lock_path = path.with_name(path.name + ".lock")
    with lock_path.open("w", encoding="utf-8") as lock:
        fcntl.flock(lock.fileno(), fcntl.LOCK_EX)
        with tempfile.NamedTemporaryFile("w", encoding="utf-8", dir=path.parent, delete=False) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_name = tmp.name
        os.replace(tmp_name, path)
        fcntl.flock(lock.fileno(), fcntl.LOCK_UN)
    lock_path.unlink(missing_ok=True)


route_path = ROOT / "src/trace_engine_v2/part_issue_2301_pineco_quick_ball_vessel_route.inc"
route = r'''  bool issue_2301_banked_t4_route_{false};

  std::optional<Card> issue_2301_quick_ball_timer_cost() const {
    // Seed 38 has exactly two distinct held Apex Dragon payloads. Quick Ball may
    // spend one only because Earthen Vessel preserves a second one for the future
    // ready turn, while the T3 Basic search establishes Regidrago V's evolution
    // timer and K1 one turn earlier. Every ordinary lower-DCI cost remains preferred.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, hidden-deck search, Bench, evolution, Energy, Ability, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K0/K1, strict-JIT, dynamic DCI, and earliest complete route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (scenario_.dci != DciProfile::StrictJit || scenario_.going_first ||
        scenario_.locks != LockMode::None || state_.turn != 3 ||
        state_.turn + 1 > scenario_.max_turn || prizes_known() || item_locked() ||
        !supporter_allowed() || state_.manual_energy_used || state_.retreat_used ||
        state_.vstar_power_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX || !state_.bench.empty() ||
        bench_space() < 2 || state_.stadium != Stadium::ForestOfVitality ||
        hand_count(Card::QuickBall) == 0 || hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::Grass) == 0 || hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::Crispin) == 0 || hand_count(Card::SecretBox) == 0 ||
        !need_regi() || !might_be_unseen(Card::RegidragoV) ||
        !might_be_unseen(Card::ForestSealStone) || !might_be_unseen(Card::Dawn) ||
        !might_be_unseen(Card::Pineco) || !might_be_unseen(Card::ForretressEx) ||
        !might_be_unseen(Card::Grass) || !might_be_unseen(Card::Fire)) {
      return std::nullopt;
    }

    int held_payloads = 0;
    int distinct_payloads = 0;
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      const int copies = hand_count(card);
      held_payloads += copies;
      distinct_payloads += copies > 0 ? 1 : 0;
    }
    if (held_payloads != 2 || distinct_payloads != 2) return std::nullopt;

    // An already dead card, surplus Energy, or any other ordinary legal Quick Ball
    // cost must be consumed before this future-route Dragon exception.
    // Battle VIP Pass dead-card precedent: https://api.pokemontcg.io/v2/cards/swsh8-225
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (choose_discard(false, true, true, Card::QuickBall).has_value()) {
      return std::nullopt;
    }

    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool issue_2301_k1_t4_route_proven() const {
    // Quick Ball has now legally established K1. The bank is admitted only when
    // the exact remaining zones prove every T4 resource. Three Grass in deck are
    // sufficient against every mandatory T4 draw: if Grass is drawn, that copy is
    // the Secret Box Grass cost; otherwise Vessel searches one and leaves two for
    // Exploding Energy. The held VSTAR is a Box cost, so another VSTAR must remain
    // available in deck for Star Alchemy unless the mandatory draw moves it to hand.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // K1 and future-card-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (!prizes_known() || state_.turn != 3 || scenario_.locks != LockMode::None ||
        item_locked() || !supporter_allowed() || state_.manual_energy_used ||
        state_.retreat_used || state_.vstar_power_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX || !state_.bench.empty() ||
        bench_space() < 2 || state_.stadium != Stadium::ForestOfVitality ||
        hand_count(Card::RegidragoV) == 0 || hand_count(Card::EarthenVessel) == 0 ||
        hand_count(Card::Grass) == 0 || hand_count(Card::RegidragoVstar) == 0 ||
        hand_count(Card::Crispin) == 0 || hand_count(Card::SecretBox) == 0 ||
        !ability_available_for_pokemon(Card::ForretressEx)) {
      return false;
    }

    int held_payloads = 0;
    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      held_payloads += hand_count(card);
    }
    if (held_payloads < 1) return false;

    const auto available = [this](const Card card) {
      return hand_count(card) > 0 || deck_count_after_search_started(card) > 0;
    };
    return available(Card::ForestSealStone) && available(Card::Dawn) &&
        available(Card::Pineco) && available(Card::ForretressEx) &&
        deck_count_after_search_started(Card::RegidragoVstar) > 0 &&
        deck_count_after_search_started(Card::Grass) >= 3 &&
        available(Card::Fire);
  }

  bool play_issue_2301_quick_ball_timer_route() {
    const auto cost = issue_2301_quick_ball_timer_cost();
    if (!cost) return false;

    // Quick Ball's printed discard precedes its Basic search. The Dragon spent on
    // T3 is deliberately not the strict-JIT payload; it buys the missing Basic and
    // evolution timer. The real search then establishes K1 before the future route
    // is banked, so exact Prize/deck availability is never read at K0.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item-cost and hidden-deck search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K0/K1 and strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (!remove_one(state_.hand, Card::QuickBall)) {
      throw std::logic_error("issue-2301 Quick Ball disappeared");
    }
    state_.discard.push_back(Card::QuickBall);
    if (!discard_from_hand(*cost, "Quick Ball issue-2301 timer cost",
                           "R-QB-01; P-DCI-01; P-JIT-01")) {
      throw std::logic_error("issue-2301 Dragon cost disappeared");
    }
    record_deck_search_knowledge("Quick Ball issue-2301 timer route");
    const bool found = move_deck_to_hand(Card::RegidragoV);
    shuffle(state_.deck);
    issue_2301_banked_t4_route_ = found && issue_2301_k1_t4_route_proven();
    trace("QUICK BALL", "R-QB-01; P-DCI-01; P-KNOWLEDGE-01",
          found ? (issue_2301_banked_t4_route_
                       ? "Discarded one of two distinct Dragons, searched Regidrago V, and proved the draw-independent T4 Vessel/Secret Box route at K1."
                       : "Discarded one of two distinct Dragons and searched Regidrago V; exact K1 targets did not prove the banked T4 route.")
                : "Discarded one of two distinct Dragons; Regidrago V was unavailable after the legal K0 search.");
    return true;
  }

  bool finish_issue_2301_t3_bank() {
    if (!issue_2301_banked_t4_route_ || state_.turn != 3) return false;
    // Bench the searched Basic and spend only the already-held Grass attachment.
    // Crispin, Earthen Vessel, Secret Box, Regidrago VSTAR, and the second Dragon
    // stay untouched for the deterministic T4 continuation.
    // Regidrago V: https://api.pokemontcg.io/v2/cards/swsh12-135
    // Core Bench and manual Energy attachment procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (!bench_from_hand(Card::RegidragoV, false)) {
      issue_2301_banked_t4_route_ = false;
      return false;
    }
    auto regi = std::find_if(state_.bench.begin(), state_.bench.end(),
                             [this](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoV && pokemon.entered_turn == state_.turn;
    });
    if (regi == state_.bench.end() || !remove_one(state_.hand, Card::Grass)) {
      issue_2301_banked_t4_route_ = false;
      return false;
    }
    ++regi->grass;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY; P-AXIS-01",
          "Grass Energy manually to the issue-2301 Regidrago V; preserved the T4 Vessel, Secret Box, Crispin, VSTAR, and second Dragon.");
    trace("BANK ROUTE", "P-KNOWLEDGE-01; P-DCI-01; P-JIT-01",
          "Banked the exact K1-proven T4 Vessel/Secret Box/Forest Seal Stone/Dawn/Forretress continuation without using a future draw identity.");
    return true;
  }

  bool complete_issue_2301_banked_t4_route() {
    if (!issue_2301_banked_t4_route_ || state_.turn != 4 || item_locked() ||
        !supporter_allowed() || state_.supporter_used || state_.manual_energy_used ||
        state_.retreat_used || state_.vstar_power_used || !state_.active ||
        state_.active->card != Card::TapuLeleGX ||
        state_.stadium != Stadium::ForestOfVitality ||
        !ability_available_for_pokemon(Card::ForretressEx) ||
        hand_count(Card::EarthenVessel) == 0 || hand_count(Card::SecretBox) == 0 ||
        hand_count(Card::RegidragoVstar) == 0 || hand_count(Card::Crispin) == 0) {
      return false;
    }
    auto find_regi = [this]() {
      return std::find_if(state_.bench.begin(), state_.bench.end(),
                          [this](const Pokemon& pokemon) {
        return pokemon.card == Card::RegidragoV &&
               pokemon.entered_turn < state_.turn;
      });
    };
    auto regi = find_regi();
    if (regi == state_.bench.end() || regi->grass != 1 || regi->fire != 0) return false;

    auto payload = std::find_if(state_.hand.begin(), state_.hand.end(), is_payload);
    if (payload == state_.hand.end()) return false;
    const Card payload_cost = *payload;

    // Earthen Vessel creates the real T4 strict-JIT payload. It searches only the
    // Energy copies still needed by the route. If the mandatory draw moved Grass
    // or Fire into hand, that copy is used in the same deterministic route and the
    // deck search simply takes fewer cards, as Vessel permits up to two different
    // Basic Energy types.
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago VSTAR / Apex Dragon: https://api.pokemontcg.io/v2/cards/swsh12-136
    // Official Item, discard, search, and Energy procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Strict-JIT policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    remove_one(state_.hand, Card::EarthenVessel);
    state_.discard.push_back(Card::EarthenVessel);
    if (!discard_from_hand(payload_cost, "Earthen Vessel issue-2301 T4 payload cost",
                           "R-VESSEL-01; P-JIT-01; P-DCI-01")) {
      return false;
    }
    record_deck_search_knowledge("Earthen Vessel issue-2301 T4 route");
    if (hand_count(Card::Grass) == 0 && !move_deck_to_hand(Card::Grass)) return false;
    if (hand_count(Card::Fire) == 0 && !move_deck_to_hand(Card::Fire)) return false;
    shuffle(state_.deck);
    trace("PLAY ITEM", "R-VESSEL-01; R-GAME-ITEM; P-JIT-01",
          "Earthen Vessel discarded the reserved Dragon on T4 and made Grass plus Fire available for the proven finish.");

    // Secret Box pays only route-replaced cards: the original held VSTAR will be
    // replaced by Star Alchemy, Crispin is replaced by Dawn/Forretress plus manual
    // Fire, and Grass is surplus because Exploding Energy supplies the board Grass.
    // The mandatory T4 draw is never needed as one of the three costs.
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Dynamic DCI / resource replacement: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    remove_one(state_.hand, Card::SecretBox);
    state_.discard.push_back(Card::SecretBox);
    if (!discard_from_hand(Card::RegidragoVstar, "Secret Box issue-2301 replaced VSTAR cost",
                           "R-SECRET-BOX-01; P-DCI-01") ||
        !discard_from_hand(Card::Crispin, "Secret Box issue-2301 replaced Crispin cost",
                           "R-SECRET-BOX-01; P-DCI-01") ||
        !discard_from_hand(Card::Grass, "Secret Box issue-2301 surplus Grass cost",
                           "R-SECRET-BOX-01; P-DCI-01")) {
      return false;
    }
    record_deck_search_knowledge("Secret Box issue-2301 T4 route");
    if (hand_count(Card::ForestSealStone) == 0 &&
        !move_deck_to_hand(Card::ForestSealStone)) return false;
    if (hand_count(Card::Dawn) == 0 && !move_deck_to_hand(Card::Dawn)) return false;
    shuffle(state_.deck);
    outcome_.used_secret_box = true;
    ++outcome_.secret_box_combo_attempted;
    trace("PLAY ITEM", "R-SECRET-BOX-01; R-GAME-ITEM; P-DCI-01",
          "Secret Box spent the replaced VSTAR, Crispin, and Grass; searched Forest Seal Stone and Dawn without consuming the mandatory T4 draw as a cost.");

    regi = find_regi();
    if (regi == state_.bench.end() || regi->tool != Tool::None ||
        !remove_one(state_.hand, Card::ForestSealStone)) return false;
    regi->tool = Tool::ForestSealStone;
    trace("ATTACH TOOL", "R-FSS-01; R-GAME-TOOL",
          "Attached Forest Seal Stone to the prior-turn Regidrago V for Star Alchemy.");

    // Star Alchemy searches a replacement VSTAR. If the mandatory draw moved that
    // known K1 target into hand, the search may legally find no copy; the same
    // replacement card is already present and the route remains draw-identity safe.
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Pokémon V includes Regidrago V: https://compendium.pokegym.net/category/7-gameplay/pokemon-v/
    // Hidden-deck search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    record_deck_search_knowledge("Star Alchemy issue-2301 replacement VSTAR");
    move_deck_to_hand(Card::RegidragoVstar);
    shuffle(state_.deck);
    state_.vstar_power_used = true;
    outcome_.used_fss = true;
    if (hand_count(Card::RegidragoVstar) == 0) return false;
    trace("USE VSTAR POWER", "R-FSS-01; P-KNOWLEDGE-01",
          "Star Alchemy made a replacement Regidrago VSTAR available after Secret Box spent the original held copy.");

    // Dawn supplies the Grass evolution line. Its hidden searches may use copies
    // already moved into hand by the mandatory draw, so success never depends on
    // which identity was drawn.
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Official Supporter and hidden-deck search procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    remove_one(state_.hand, Card::Dawn);
    state_.discard.push_back(Card::Dawn);
    state_.supporter_used = true;
    record_deck_search_knowledge("Dawn issue-2301 Pineco line");
    if (hand_count(Card::Pineco) == 0 && !move_deck_to_hand(Card::Pineco)) return false;
    if (hand_count(Card::ForretressEx) == 0 && !move_deck_to_hand(Card::ForretressEx)) return false;
    shuffle(state_.deck);
    trace("PLAY SUPPORTER", "R-DAWN-01; R-GAME-SUPPORTER",
          "Dawn made Pineco and Forretress ex available for the Forest of Vitality line.");

    if (!bench_from_hand(Card::Pineco, false)) return false;
    auto pineco = std::find_if(state_.bench.begin(), state_.bench.end(),
                               [this](const Pokemon& pokemon) {
      return pokemon.card == Card::Pineco && pokemon.entered_turn == state_.turn;
    });
    if (pineco == state_.bench.end() || !remove_one(state_.hand, Card::ForretressEx)) return false;
    pineco->card = Card::ForretressEx;
    trace("EVOLVE", "R-FOREST-01; R-GAME-EVOLVE",
          "Forest of Vitality evolved the newly Benched Grass Pineco into Grass Forretress ex on T4.");

    // Exploding Energy attaches exactly the two Grass needed by this board, then
    // Knocks Out Forretress ex: one Grass completes Regidrago's second Grass, and
    // one Grass pays Tapu Lele-GX's printed one-Energy Retreat Cost.
    // Forretress ex / Exploding Energy: https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Tapu Lele-GX Retreat Cost 1: https://api.pokemontcg.io/v2/cards/sm2-60
    // Official Ability, Energy, Knock Out, and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    regi = find_regi();
    pineco = std::find_if(state_.bench.begin(), state_.bench.end(),
                          [](const Pokemon& pokemon) {
      return pokemon.card == Card::ForretressEx;
    });
    if (regi == state_.bench.end() || pineco == state_.bench.end() ||
        !state_.active || state_.active->card != Card::TapuLeleGX ||
        std::count(state_.deck.begin(), state_.deck.end(), Card::Grass) < 2) {
      return false;
    }
    remove_one(state_.deck, Card::Grass);
    remove_one(state_.deck, Card::Grass);
    ++regi->grass;
    ++state_.active->grass;
    shuffle(state_.deck);
    outcome_.used_exploding_energy = true;
    trace("USE ABILITY", "R-FORRETRESS-01; R-GAME-ABILITY",
          "Exploding Energy attached one Grass to Regidrago V and one Grass to Active Tapu Lele-GX, then Knocked Out Forretress ex.");
    state_.discard.push_back(Card::ForretressEx);
    state_.bench.erase(pineco);

    regi = find_regi();
    if (regi == state_.bench.end() || !remove_one(state_.hand, Card::Fire)) return false;
    ++regi->fire;
    state_.manual_energy_used = true;
    trace("ATTACH", "R-GAME-ENERGY",
          "Fire Energy manually to the issue-2301 Regidrago V, completing GGF.");

    if (!remove_one(state_.hand, Card::RegidragoVstar)) return false;
    regi->card = Card::RegidragoVstar;
    trace("EVOLVE", "R-GAME-EVOLVE; R-RVS-01",
          "Evolved the prior-turn Regidrago V into Regidrago VSTAR after GGF was complete.");

    if (!state_.active || state_.active->card != Card::TapuLeleGX ||
        state_.active->grass < 1) return false;
    auto ready = std::find_if(state_.bench.begin(), state_.bench.end(),
                              [](const Pokemon& pokemon) {
      return pokemon.card == Card::RegidragoVstar &&
             pokemon.grass >= 2 && pokemon.fire >= 1;
    });
    if (ready == state_.bench.end()) return false;
    --state_.active->grass;
    state_.discard.push_back(Card::Grass);
    std::swap(*state_.active, *ready);
    state_.retreat_used = true;
    issue_2301_banked_t4_route_ = false;
    trace("RETREAT", "R-GAME-RETREAT; R-TAPU-01",
          "Discarded Tapu Lele-GX's attached Grass and promoted the GGF Regidrago VSTAR.");
    trace("COMPLETE ROUTE", "P-JIT-01; P-AXIS-01",
          "Completed the deterministic issue-2301 T4 route with the Vessel-cost Dragon discarded this turn.");
    return active_is_vstar() && !need_energy() && !need_payload();
  }

  bool play_quick_ball(const bool permit_payload) {
    if (play_issue_2301_quick_ball_timer_route()) return true;
    return play_quick_ball_issue2301_original(permit_payload);
  }
'''
atomic_write(route_path, route)

sim_path = ROOT / "src/regidrago_sim.cpp"
sim = sim_path.read_text(encoding="utf-8")
old = '''#undef play_quick_ball
#include "trace_engine_v2/part_issue_2272_route_replaced_arven_quick_ball_override.inc"
#include "trace_engine_v2/part_issue_1437_crispin_trace_override.inc"
'''
new = '''#undef play_quick_ball
// #2301 wraps the cumulative Quick Ball policy with one source-bound Pineco timer route:
// https://github.com/FlareZ123/pokemon-sims/issues/2301
#define play_quick_ball play_quick_ball_issue2301_original
#include "trace_engine_v2/part_issue_2272_route_replaced_arven_quick_ball_override.inc"
#undef play_quick_ball
#include "trace_engine_v2/part_issue_2301_pineco_quick_ball_vessel_route.inc"
#include "trace_engine_v2/part_issue_1437_crispin_trace_override.inc"
'''
if sim.count(old) != 1:
    raise RuntimeError("#2301 Quick Ball include anchor mismatch")
sim = sim.replace(old, new, 1)
atomic_write(sim_path, sim)

secret_path = ROOT / "src/trace_engine_v2/part_issue_1118_secret_box.inc"
secret = secret_path.read_text(encoding="utf-8")
anchor_t4 = '''    if (issue_1420_direct_t2_treasure_route_) {
'''
insert_t4 = '''    // #2301 is banked only after a real T3 Quick Ball search establishes K1 and
    // proves every exact T4 target. Resolve it before generic Secret Box selection
    // can spend the reserved Vessel, Dragon, VSTAR, or Crispin on a weaker route:
    // https://github.com/FlareZ123/pokemon-sims/issues/2301
    // https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
    if (issue_2301_banked_t4_route_ && state_.turn == 4) {
      if (complete_issue_2301_banked_t4_route()) {
        trace("POLICY", "P-AXIS-01; P-JIT-01",
              "End Pineco policy after deterministic issue-2301 T4 completion: " + state_line());
        return;
      }
      issue_2301_banked_t4_route_ = false;
    }

'''
if secret.count(anchor_t4) != 1:
    raise RuntimeError("#2301 T4 Pineco anchor mismatch")
secret = secret.replace(anchor_t4, insert_t4 + anchor_t4, 1)
anchor_t3 = '''    bench_tapu_for_secret_box_connector();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());

    attach_live_fss();
'''
replace_t3 = '''    bench_tapu_for_secret_box_connector();
    play_basics_from_hand();
    play_items_until_stable(!strict_payload_timing());
    // The #2301 Quick Ball action is the only T3 Item needed for its banked route.
    // Bench Regidrago V and attach the held Grass immediately, then end policy for
    // this turn so Crispin and Earthen Vessel remain physically available on T4:
    // https://api.pokemontcg.io/v2/cards/swsh1-179
    // https://api.pokemontcg.io/v2/cards/sv4-163
    // https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (issue_2301_banked_t4_route_ && state_.turn == 3 &&
        finish_issue_2301_t3_bank()) {
      trace("POLICY", "P-AXIS-01; P-KNOWLEDGE-01",
            "End Pineco policy after banking issue-2301 T4 route: " + state_line());
      return;
    }

    attach_live_fss();
'''
if secret.count(anchor_t3) != 1:
    raise RuntimeError("#2301 T3 Pineco anchor mismatch")
secret = secret.replace(anchor_t3, replace_t3, 1)
atomic_write(secret_path, secret)


test_path = ROOT / "tests/issue_2301_pineco_quick_ball_vessel_timer_tests.cpp"
test = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) { engine.state_ = std::move(state); }
  static void set_knowledge(Engine& engine, const bool deck_seen,
                            const bool prizes_revealed = false) {
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static void set_banked(Engine& engine, const bool value) {
    engine.issue_2301_banked_t4_route_ = value;
  }
  static bool banked(const Engine& engine) {
    return engine.issue_2301_banked_t4_route_;
  }
  static bool k0_available(Engine& engine) {
    return engine.issue_2301_quick_ball_timer_cost().has_value();
  }
  static bool k1_proven(Engine& engine) {
    return engine.issue_2301_k1_t4_route_proven();
  }
  static bool complete_t4(Engine& engine) {
    return engine.complete_issue_2301_banked_t4_route();
  }
};
}  // namespace sim

namespace {
void expect(const bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

bool has(const sim::TraceLog& trace, const std::string& needle) {
  return std::any_of(trace.lines.begin(), trace.lines.end(),
                     [&](const std::string& line) {
                       return line.find(needle) != std::string::npos;
                     });
}

void test_exact_seed_38_is_deterministic_t4() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario && deck, "issue-2301 exact seed fixture unavailable");
  std::mt19937_64 rng{38};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1, DCI, strict-JIT, and earliest route: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "issue-2301 seed 38 did not become ready on T4");
  expect(has(trace, "Quick Ball issue-2301 timer cost") &&
             has(trace, "T3 | BANK ROUTE") &&
             !has(trace, "T3 | PLAY SUPPORTER") &&
             has(trace, "T4 | DISCARD") &&
             has(trace, "Earthen Vessel issue-2301 T4 payload cost") &&
             has(trace, "T4 | COMPLETE ROUTE") && has(trace, "T4 | READY"),
         "issue-2301 seed 38 omitted the banked T3 action or deterministic T4 finish");
}

sim::State k0_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.stadium = sim::Stadium::ForestOfVitality;
  state.hand = {sim::Card::QuickBall, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::SecretBox,
                sim::Card::GoodraVstar, sim::Card::MegaDragonite};
  return state;
}

bool k0_available_for(const char* label, sim::State state, const int max_turn = 4) {
  auto scenario = sim::scenario_by_label(label);
  const auto* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario && deck, "issue-2301 K0 fixture unavailable");
  scenario->max_turn = max_turn;
  std::mt19937_64 rng{2301};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(engine, false);
  return sim::EngineTestAccess::k0_available(engine);
}

void test_k0_controls() {
  // The dynamic-DCI exception is deliberately narrower than generic Quick Ball.
  // Lower-DCI cards still win, the final payload is protected, and all lock,
  // Bench, retreat, Forest, and horizon gates remain mandatory.
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Core Item, Bench, retreat, and Ability procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(k0_available_for("strict-jit/go-second", k0_state()),
         "issue-2301 rejected its exact K0 public state");

  auto one_payload = k0_state();
  one_payload.hand.erase(std::find(one_payload.hand.begin(), one_payload.hand.end(),
                                   sim::Card::GoodraVstar));
  expect(!k0_available_for("strict-jit/go-second", one_payload),
         "issue-2301 spent the final held payload");

  auto no_vessel = k0_state();
  no_vessel.hand.erase(std::find(no_vessel.hand.begin(), no_vessel.hand.end(),
                                 sim::Card::EarthenVessel));
  expect(!k0_available_for("strict-jit/go-second", no_vessel),
         "issue-2301 opened without the reserved Vessel outlet");

  auto lower_dci = k0_state();
  lower_dci.hand.push_back(sim::Card::BattleVipPass);
  expect(!k0_available_for("strict-jit/go-second", lower_dci),
         "issue-2301 skipped an ordinary lower-DCI Quick Ball cost");

  auto full_bench = k0_state();
  full_bench.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 2, 0, 0, sim::Tool::None});
  expect(!k0_available_for("strict-jit/go-second", full_bench),
         "issue-2301 ignored the required empty/open Bench state");

  auto spent_retreat = k0_state();
  spent_retreat.retreat_used = true;
  expect(!k0_available_for("strict-jit/go-second", spent_retreat),
         "issue-2301 ignored spent retreat");

  auto no_forest = k0_state();
  no_forest.stadium = sim::Stadium::None;
  expect(!k0_available_for("strict-jit/go-second", no_forest),
         "issue-2301 opened without Forest of Vitality");

  expect(!k0_available_for("strict-jit-turn2-item-lock/go-second", k0_state()),
         "issue-2301 bypassed Item lock");
  expect(!k0_available_for("strict-jit-rulebox-ability-lock/go-second", k0_state()),
         "issue-2301 bypassed Rule Box Ability lock");
  expect(!k0_available_for("strict-jit-supporter-lock/go-second", k0_state()),
         "issue-2301 bypassed Supporter lock");
  expect(!k0_available_for("strict-jit/go-second", k0_state(), 3),
         "issue-2301 opened beyond the T4 setup horizon");
}

sim::State k1_state() {
  sim::State state = k0_state();
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::QuickBall));
  state.hand.erase(std::find(state.hand.begin(), state.hand.end(), sim::Card::GoodraVstar));
  state.hand.push_back(sim::Card::RegidragoV);
  state.deck = {sim::Card::ForestSealStone, sim::Card::Dawn, sim::Card::Pineco,
                sim::Card::ForretressEx, sim::Card::RegidragoVstar,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::QuickBall, sim::Card::RegidragoV};
  return state;
}

bool k1_proven_for(sim::State state) {
  auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario && deck, "issue-2301 K1 fixture unavailable");
  std::mt19937_64 rng{2301};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(engine, true);
  return sim::EngineTestAccess::k1_proven(engine);
}

void erase_all(std::vector<sim::Card>& cards, const sim::Card card) {
  cards.erase(std::remove(cards.begin(), cards.end(), card), cards.end());
}

void test_k1_exact_target_controls() {
  // K0 may only know fixed-list plausibility. After the legal Quick Ball search,
  // K1 must prove every physical target before banking the expensive future route.
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // K1 policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(k1_proven_for(k1_state()), "issue-2301 rejected the exact K1 route proof");

  for (const sim::Card missing : {sim::Card::ForestSealStone, sim::Card::Dawn,
                                  sim::Card::Pineco, sim::Card::ForretressEx,
                                  sim::Card::RegidragoVstar, sim::Card::Fire}) {
    auto state = k1_state();
    erase_all(state.deck, missing);
    erase_all(state.hand, missing);
    if (missing == sim::Card::RegidragoVstar) state.hand.push_back(sim::Card::RegidragoVstar);
    expect(!k1_proven_for(state), "issue-2301 banked with a required K1 target absent");
  }

  auto insufficient_grass = k1_state();
  erase_all(insufficient_grass.deck, sim::Card::Grass);
  insufficient_grass.deck.push_back(sim::Card::Grass);
  insufficient_grass.deck.push_back(sim::Card::Grass);
  expect(!k1_proven_for(insufficient_grass),
         "issue-2301 banked without enough Grass for arbitrary T4 draw plus Forretress");
}

sim::State t4_state(const sim::Card extra) {
  sim::State state;
  state.turn = 4;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0, sim::Tool::None};
  state.bench.push_back(sim::Pokemon{sim::Card::RegidragoV, 3, 1, 0, sim::Tool::None});
  state.stadium = sim::Stadium::ForestOfVitality;
  state.hand = {sim::Card::EarthenVessel, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::SecretBox,
                sim::Card::MegaDragonite, extra};
  state.deck = {sim::Card::ForestSealStone, sim::Card::Dawn, sim::Card::Pineco,
                sim::Card::ForretressEx, sim::Card::RegidragoVstar,
                sim::Card::Grass, sim::Card::Grass, sim::Card::Grass,
                sim::Card::Fire, sim::Card::QuickBall, sim::Card::RegidragoV};
  return state;
}

void test_t4_does_not_need_draw_as_secret_box_cost() {
  // Representative arbitrary T4 draws remain outside Secret Box's three known
  // route-replaced costs. The same completion succeeds for low-DCI, live Supporter,
  // Basic Pokémon, and protected singleton-like identities.
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  for (const sim::Card extra : {sim::Card::QuickBall, sim::Card::Arven,
                                sim::Card::RegidragoV, sim::Card::Gladion}) {
    auto scenario = sim::scenario_by_label("strict-jit/go-second");
    const auto* deck = sim::deck_by_id("regidrago-pineco");
    expect(scenario && deck, "issue-2301 T4 fixture unavailable");
    std::mt19937_64 rng{2301 + static_cast<unsigned>(extra)};
    sim::Engine engine(*scenario, deck->recipe, rng);
    sim::EngineTestAccess::set_state(engine, t4_state(extra));
    sim::EngineTestAccess::set_knowledge(engine, true);
    sim::EngineTestAccess::set_banked(engine, true);
    expect(sim::EngineTestAccess::complete_t4(engine),
           "issue-2301 deterministic T4 route failed for a representative arbitrary draw");
  }
}
}  // namespace

int main() {
  try {
    test_exact_seed_38_is_deterministic_t4();
    test_k0_controls();
    test_k1_exact_target_controls();
    test_t4_does_not_need_draw_as_secret_box_cost();
    std::cout << "Issue 2301 deterministic Pineco timer tests passed\n";
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
'''
atomic_write(test_path, test)
