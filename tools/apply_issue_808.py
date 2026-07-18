from __future__ import annotations

import os
from pathlib import Path


LOCK_PATH = Path(".issue-808-apply.lock")


def atomic_replace(path: Path, text: str) -> None:
    temporary = path.with_name(f".{path.name}.issue-808.tmp")
    temporary.write_text(text, encoding="utf-8", newline="\n")
    os.replace(temporary, path)


def replace_once(path: Path, old: str, new: str) -> None:
    text = path.read_text(encoding="utf-8")
    if new in text:
        return
    if old not in text:
        raise SystemExit(f"issue 808 anchor missing in {path}")
    atomic_replace(path, text.replace(old, new, 1))


def main() -> None:
    lock_fd = os.open(LOCK_PATH, os.O_CREAT | os.O_EXCL | os.O_WRONLY)
    try:
        os.write(lock_fd, str(os.getpid()).encode("ascii"))

        source = Path("src/trace_engine_v2/part_pokemon_communication.inc")
        old_preflight = '''    const std::vector<Card> original_hand = state_.hand;
    const auto restore = [this, &original_hand]() { state_.hand = original_hand; };
    if (!communication_is_held) state_.hand.push_back(Card::PokemonCommunication);
    if (!remove_one(state_.hand, Card::PokemonCommunication) ||
        !remove_one(state_.hand, returned)) {
      restore();
      return false;
    }
    state_.hand.push_back(fetched);

    // Pokémon Communication returns one Pokémon before the later search. That
    // returned card is a guaranteed target for a subsequent search Item only when it
    // belongs to that Item's printed target class:
'''
        new_preflight = '''    const std::vector<Card> original_hand = state_.hand;
    const std::vector<Card> original_deck = state_.deck;
    const auto restore = [this, &original_hand, &original_deck]() {
      state_.hand = original_hand;
      state_.deck = original_deck;
    };
    if (!communication_is_held) state_.hand.push_back(Card::PokemonCommunication);
    if (!remove_one(state_.hand, Card::PokemonCommunication) ||
        !remove_one(state_.hand, returned)) {
      restore();
      return false;
    }
    state_.hand.push_back(fetched);
    if (prizes_known()) {
      if (!remove_one(state_.deck, fetched)) {
        restore();
        return false;
      }
      state_.deck.push_back(returned);
    }

    // At K1 the searched Pokémon leaves the known deck before the later Item, while
    // the returned Pokémon enters it. At K0, fixed-list accounting derives the same
    // public transition from the exact post-Communication hand without inspecting
    // the hidden deck. Evaluate the later target class against that state:
'''
        replace_once(source, old_preflight, new_preflight)

        old_sources = '''    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
'''
        new_sources = '''    // Ultra Ball: https://api.pokemontcg.io/v2/cards/swsh12pt5-146
    // Earthen Vessel: https://api.pokemontcg.io/v2/cards/sv4-163
    // Known no-effect Trainer ruling: https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
    // Hidden-information policy: https://github.com/FlareZ123/pokemon-sims/blob/main/docs/MODEL_ASSUMPTIONS.md#hidden-information-policy
    // Regression: https://github.com/FlareZ123/pokemon-sims/issues/808
'''
        replace_once(source, old_sources, new_sources)

        tests = Path("tests/pokemon_communication_tests.cpp")
        test_text = tests.read_text(encoding="utf-8")
        negative_tests = r'''void test_payload_route_rejects_fetched_only_mysterious_treasure_target() {
  Fixture fixture("communication-treasure-fetched-only-target");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::MawileGX,
                sim::Card::MysteriousTreasure};
  state.deck = {sim::Card::MegaDragonite};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Mega Dragonite ex leaves the known deck during Pokémon Communication. Mawile-GX
  // then becomes the only deck card, so Mysterious Treasure would have no Psychic or
  // Dragon target after discarding the fetched payload. Reject the one-shot bridge
  // before spending it:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/sm6-113
  // https://compendium.pokegym.net/category/5-trainers/trainers-in-general/#:~:text=No%2C%20you%20cannot%20play%20a%20Trainer%20when%20it%20is%20known%20that%20it%20will%20have%20no%20effect.
  // https://github.com/FlareZ123/pokemon-sims/issues/808
  if (sim::EngineTestAccess::play_pokemon_communication(fixture.engine, true) ||
      sim::EngineTestAccess::state(fixture.engine).hand != state.hand ||
      sim::EngineTestAccess::state(fixture.engine).deck != state.deck) {
    throw std::runtime_error(
        "Communication must reject the known targetless Treasure continuation.");
  }
}

void test_payload_route_rejects_fetched_only_quick_ball_target() {
  Fixture fixture("communication-quick-ball-fetched-only-target");
  sim::State state;
  state.turn = 2;
  state.active = sim::Pokemon{sim::Card::RegidragoVstar, 1, 2, 1,
                              sim::Tool::None};
  state.hand = {sim::Card::PokemonCommunication, sim::Card::Dipplin,
                sim::Card::QuickBall};
  state.deck = {sim::Card::DialgaGX};
  sim::EngineTestAccess::set_state(fixture.engine, state);
  sim::EngineTestAccess::set_deck_seen(fixture.engine);

  // Dialga-GX leaves the known deck during Pokémon Communication. The returned
  // Stage 1 Dipplin cannot satisfy Quick Ball's Basic target requirement after the
  // fetched Dialga-GX pays the discard cost:
  // https://api.pokemontcg.io/v2/cards/sm9-152
  // https://api.pokemontcg.io/v2/cards/swsh1-179
  // https://api.pokemontcg.io/v2/cards/sm5-100
  // https://api.pokemontcg.io/v2/cards/sv6-127
  // https://github.com/FlareZ123/pokemon-sims/issues/808
  if (sim::EngineTestAccess::play_pokemon_communication(fixture.engine, true) ||
      sim::EngineTestAccess::state(fixture.engine).hand != state.hand ||
      sim::EngineTestAccess::state(fixture.engine).deck != state.deck) {
    throw std::runtime_error(
        "Communication must reject the known targetless Quick Ball continuation.");
  }
}

'''
        if "void test_payload_route_rejects_fetched_only_mysterious_treasure_target()" not in test_text:
            anchor = "void test_payload_route_accepts_a_different_mysterious_treasure_target() {\n"
            if anchor not in test_text:
                raise SystemExit("issue 808 test insertion anchor missing")
            test_text = test_text.replace(anchor, negative_tests + anchor, 1)

        old_main = '''  test_payload_route_accepts_a_different_mysterious_treasure_target();
  test_payload_route_accepts_a_different_quick_ball_target();
'''
        new_main = '''  test_payload_route_rejects_fetched_only_mysterious_treasure_target();
  test_payload_route_rejects_fetched_only_quick_ball_target();
  test_payload_route_accepts_a_different_mysterious_treasure_target();
  test_payload_route_accepts_a_different_quick_ball_target();
'''
        if new_main not in test_text:
            if old_main not in test_text:
                raise SystemExit("issue 808 test main anchor missing")
            test_text = test_text.replace(old_main, new_main, 1)
        atomic_replace(tests, test_text)
    finally:
        os.close(lock_fd)
        LOCK_PATH.unlink(missing_ok=True)


if __name__ == "__main__":
    main()
