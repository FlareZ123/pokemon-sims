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
        with tempfile.NamedTemporaryFile(
            "w", encoding="utf-8", dir=path.parent, delete=False
        ) as tmp:
            tmp.write(text)
            tmp.flush()
            os.fsync(tmp.fileno())
            tmp_name = tmp.name
        os.replace(tmp_name, path)
        fcntl.flock(lock.fileno(), fcntl.LOCK_UN)
    lock_path.unlink(missing_ok=True)


source_path = ROOT / "src/trace_engine_v2/part_issue_2272_route_replaced_arven_quick_ball_override.inc"
source = source_path.read_text(encoding="utf-8")
anchor = "  bool play_quick_ball(const bool permit_payload) {\n"
helper = r'''  std::optional<Card> issue_2301_quick_ball_vessel_timer_cost() const {
    // This exception is deliberately narrow. On the Pineco strict-JIT T3 timer
    // state, one of exactly two distinct held payloads may pay Quick Ball only
    // when a held Earthen Vessel preserves the second Dragon for the projected
    // T4 same-turn payload event and the rest of the connector route is publicly
    // plausible at K0 or proven at K1. Lower-DCI costs keep priority.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
    // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
    // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
    // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
    // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
    // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
    // Tapu Lele-GX: https://api.pokemontcg.io/v2/cards/sm2-60
    // Official Item, discard, search, Bench, evolution, Ability, attachment, KO and retreat procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
    // K0/K1, strict-JIT, dynamic DCI and earliest-route policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
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
        !need_regi()) {
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

    // Any ordinary cost with lower DCI must be consumed first. The Dragon fallback
    // exists only when the generic selector would otherwise pass the turn.
    // Battle VIP Pass dead-card DCI precedent: https://api.pokemontcg.io/v2/cards/swsh8-225
    // Dynamic DCI: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (choose_discard(false, true, true, Card::QuickBall).has_value()) {
      return std::nullopt;
    }

    // At K0 these checks use only fixed-list plausibility. The Quick Ball search
    // itself establishes K1 before any later route decision can inspect exact
    // deck/Prize identities, so no hidden-zone oracle is introduced here.
    // K0/K1 specification: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states
    // Future-card oracle prohibition: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
    // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
    if (!might_be_unseen(Card::RegidragoV) ||
        !might_be_unseen(Card::ForestSealStone) || !might_be_unseen(Card::Dawn) ||
        !might_be_unseen(Card::Pineco) || !might_be_unseen(Card::ForretressEx) ||
        !might_be_unseen(Card::Grass) || !might_be_unseen(Card::Fire)) {
      return std::nullopt;
    }

    for (const Card card : {Card::Appletun, Card::MegaDragonite,
                            Card::Dragapult, Card::GoodraVstar,
                            Card::DialgaGX}) {
      if (hand_count(card) > 0) return card;
    }
    return std::nullopt;
  }

  bool play_issue_2301_quick_ball_vessel_timer_route() {
    const auto cost = issue_2301_quick_ball_vessel_timer_cost();
    if (!cost) return false;

    // Quick Ball's printed cost happens before its hidden-deck search. Spending
    // one Dragon establishes the Basic/evolution timer while Earthen Vessel keeps
    // the second Dragon available for the actual T4 strict-JIT discard event.
    // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
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
    record_deck_search_knowledge("Quick Ball issue-2301 Regidrago timer route");
    const bool found = move_deck_to_hand(Card::RegidragoV);
    shuffle(state_.deck);
    trace("QUICK BALL", "R-QB-01; P-DCI-01; P-CONNECTOR-01",
          found ? "Discarded one of two distinct Dragons and searched Regidrago V; reserved the second Dragon for Earthen Vessel."
                : "Discarded one of two distinct Dragons; Regidrago V was unavailable after the legal K0 search.");
    return true;
  }

'''
if helper not in source:
    if source.count(anchor) != 1:
        raise RuntimeError("#2301 Quick Ball anchor mismatch")
    source = source.replace(anchor, helper + anchor, 1)
old = '''  bool play_quick_ball(const bool permit_payload) {
    if (play_issue_2199_quick_ball_latias_payload_route()) return true;
    if (play_issue_2272_route_replaced_arven_quick_ball()) return true;
    return play_quick_ball_issue2272_original(permit_payload);
  }
'''
new = '''  bool play_quick_ball(const bool permit_payload) {
    if (play_issue_2199_quick_ball_latias_payload_route()) return true;
    if (play_issue_2272_route_replaced_arven_quick_ball()) return true;
    if (play_issue_2301_quick_ball_vessel_timer_route()) return true;
    return play_quick_ball_issue2272_original(permit_payload);
  }
'''
if new not in source:
    if source.count(old) != 1:
        raise RuntimeError("#2301 Quick Ball wrapper mismatch")
    source = source.replace(old, new, 1)
atomic_write(source_path, source)

test_path = ROOT / "tests/issue_2301_pineco_quick_ball_vessel_timer_tests.cpp"
test = r'''#define REGIDRAGO_SIM_NO_MAIN
#include "../src/regidrago_sim.cpp"

#include <algorithm>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>

namespace sim {
struct EngineTestAccess {
  static void set_state(Engine& engine, State state) {
    engine.state_ = std::move(state);
  }
  static void set_knowledge(Engine& engine, const bool deck_seen,
                            const bool prizes_revealed) {
    engine.deck_seen_ = deck_seen;
    engine.prizes_revealed_ = prizes_revealed;
  }
  static bool issue_2301_available(Engine& engine) {
    return engine.issue_2301_quick_ball_vessel_timer_cost().has_value();
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

void test_exact_seed_38_reaches_t4() {
  const auto scenario = sim::scenario_by_label("strict-jit/go-second");
  const auto* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario && deck, "issue-2301 exact-seed fixture unavailable");

  std::mt19937_64 rng{38};
  sim::TraceLog trace{true, {}};
  sim::Engine engine(*scenario, deck->recipe, rng, &trace);
  const auto outcome = engine.run();

  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // Regidrago V / VSTAR: https://api.pokemontcg.io/v2/cards/swsh12-135 https://api.pokemontcg.io/v2/cards/swsh12-136
  // Secret Box: https://api.pokemontcg.io/v2/cards/sv6-163
  // Forest Seal Stone: https://api.pokemontcg.io/v2/cards/swsh12-156
  // Dawn: https://api.pokemontcg.io/v2/cards/me2-87
  // Pineco / Forretress ex: https://api.pokemontcg.io/v2/cards/sv4pt5-1 https://api.pokemontcg.io/v2/cards/sv4pt5-2
  // Forest of Vitality: https://api.pokemontcg.io/v2/cards/me1-117
  // Official procedure: https://www.pokemon.com/static-assets/content-assets/cms2/pdf/trading-card-game/rulebook/par_rulebook_en.pdf
  // K0/K1, strict-JIT, DCI, and route priority: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#dcijit-treatment https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#dci-implementation https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#decision-priorities
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(outcome.first_ready_turn == 4 && !outcome.setup_failed,
         "issue-2301 seed 38 did not reach strict-JIT readiness on T4");
  expect(has(trace, "Quick Ball issue-2301 timer cost") &&
             has(trace, "T3 | BENCH") && has(trace, "Regidrago V from hand") &&
             has(trace, "T3 | ATTACH") && has(trace, "T4 | READY"),
         "issue-2301 seed 38 omitted the T3 timer route or T4 finish");
}

sim::State route_state() {
  sim::State state;
  state.turn = 3;
  state.active = sim::Pokemon{sim::Card::TapuLeleGX, 1, 0, 0,
                              sim::Tool::None};
  state.stadium = sim::Stadium::ForestOfVitality;
  state.hand = {sim::Card::QuickBall, sim::Card::EarthenVessel,
                sim::Card::Grass, sim::Card::RegidragoVstar,
                sim::Card::Crispin, sim::Card::SecretBox,
                sim::Card::GoodraVstar, sim::Card::MegaDragonite};
  state.deck = {sim::Card::RegidragoV, sim::Card::ForestSealStone,
                sim::Card::Dawn, sim::Card::Pineco, sim::Card::ForretressEx,
                sim::Card::Grass, sim::Card::Fire};
  return state;
}

bool available_for(const char* label, sim::State state,
                   const bool deck_seen = false, const int max_turn = 4) {
  auto scenario = sim::scenario_by_label(label);
  const auto* deck = sim::deck_by_id("regidrago-pineco");
  expect(scenario && deck, "issue-2301 control fixture unavailable");
  scenario->max_turn = max_turn;
  std::mt19937_64 rng{2301};
  sim::Engine engine(*scenario, deck->recipe, rng);
  sim::EngineTestAccess::set_state(engine, std::move(state));
  sim::EngineTestAccess::set_knowledge(engine, deck_seen, false);
  return sim::EngineTestAccess::issue_2301_available(engine);
}

void test_public_route_controls() {
  // The exception must never spend the last held payload, bypass a lower-DCI
  // Quick Ball cost, cross a modeled lock, or rely on a horizon that cannot reach T4.
  // Battle VIP Pass: https://api.pokemontcg.io/v2/cards/swsh8-225
  // Quick Ball: https://api.pokemontcg.io/v2/cards/swsh1-179
  // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
  // K0/K1 and no-oracle policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/POLICY_DECISIONS.md#knowledge-states https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#policy-versus-future-card-oracle
  // Confirmed bug: https://github.com/FlareZ123/pokemon-sims/issues/2301
  expect(available_for("strict-jit/go-second", route_state()),
         "issue-2301 rejected the exact public K0 route shape");

  auto one_payload = route_state();
  one_payload.hand.erase(std::find(one_payload.hand.begin(), one_payload.hand.end(),
                                   sim::Card::GoodraVstar));
  expect(!available_for("strict-jit/go-second", one_payload),
         "issue-2301 spent the final held payload");

  auto no_vessel = route_state();
  no_vessel.hand.erase(std::find(no_vessel.hand.begin(), no_vessel.hand.end(),
                                 sim::Card::EarthenVessel));
  expect(!available_for("strict-jit/go-second", no_vessel),
         "issue-2301 opened without the reserved Vessel outlet");

  auto lower_dci = route_state();
  lower_dci.hand.push_back(sim::Card::BattleVipPass);
  expect(!available_for("strict-jit/go-second", lower_dci),
         "issue-2301 skipped an ordinary lower-DCI Quick Ball cost");

  expect(!available_for("strict-jit-turn2-item-lock/go-second", route_state()),
         "issue-2301 bypassed Item lock");
  expect(!available_for("strict-jit-rulebox-ability-lock/go-second", route_state()),
         "issue-2301 bypassed Rule Box Ability lock");
  expect(!available_for("strict-jit-supporter-lock/go-second", route_state()),
         "issue-2301 bypassed Supporter lock");
  expect(!available_for("strict-jit/go-second", route_state(), false, 3),
         "issue-2301 opened beyond the setup horizon");

  auto no_forest = route_state();
  no_forest.stadium = sim::Stadium::None;
  expect(!available_for("strict-jit/go-second", no_forest),
         "issue-2301 opened without Forest of Vitality");

  auto no_regi_k1 = route_state();
  no_regi_k1.deck.erase(std::find(no_regi_k1.deck.begin(), no_regi_k1.deck.end(),
                                  sim::Card::RegidragoV));
  expect(!available_for("strict-jit/go-second", no_regi_k1, true),
         "issue-2301 used an absent K1 Regidrago V target");
}
}  // namespace

int main() {
  test_exact_seed_38_reaches_t4();
  test_public_route_controls();
  return 0;
}
'''
atomic_write(test_path, test)
