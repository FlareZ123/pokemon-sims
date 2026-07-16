// Strict JIT controls when policy may commit a protected Dragon payload. It does
// not change the game-state fact that a Dragon already in discard remains usable
// by Apex Dragon on later turns.
//
// This fragment is composed inside Engine after part_006.inc.

  bool strict_payload_timing_for_readiness() const {
    // part_004.inc's legacy payload_ready() branches on this compatibility hook.
    // Always select its persistent-discard branch; strict timing is enforced by
    // payload_window_open() and can_play_payload_this_turn() instead.
    return false;
  }

  bool strict_jit_attacker_energy_complete(const Pokemon& pokemon) const {
    return (pokemon.card == Card::RegidragoV ||
            pokemon.card == Card::RegidragoVstar) &&
           pokemon.grass >= 2 && pokemon.fire >= 1;
  }

  bool strict_jit_attacker_can_be_active_this_turn(
      const Pokemon& pokemon) const {
    if (state_.active && &pokemon == &*state_.active) return true;
    if (!state_.active) return false;

    // Latias ex makes a Basic Active's Retreat Cost zero. Tate & Liza is the
    // modeled Supporter switch route. Either route can promote the selected
    // Benched attacker after its payload is committed.
    const bool latias_route = is_basic(state_.active->card) &&
        in_play(Card::LatiasEx) &&
        ability_available_for_pokemon(Card::LatiasEx) &&
        !state_.retreat_used;
    const bool tate_route = supporter_allowed() &&
        hand_count(Card::TateLiza) > 0;
    return latias_route || tate_route;
  }

  int strict_jit_ultra_ball_policy_cost_count(
      const bool permit_payload) const {
    int count = 0;

    if (permit_payload) {
      for (const Card card : {Card::MegaDragonite, Card::Dragapult,
                              Card::GoodraVstar, Card::DialgaGX}) {
        count += hand_count(card);
      }
    }

    count += hand_count(Card::Dipplin);
    if (in_play_count(Card::RegidragoV) >= 2) {
      count += hand_count(Card::RegidragoV);
    }
    if (has_vstar()) count += hand_count(Card::RegidragoVstar);

    count += std::max(0, hand_count(Card::Grass) - (grass_needed() + 1));
    count += std::max(0, hand_count(Card::Fire) - (fire_needed() + 1));

    if (scenario_.dci != DciProfile::StrictJit) {
      for (const Card card : {Card::MawileGX, Card::Guzma,
                              Card::Channeler, Card::FieldBlower,
                              Card::ChaoticSwell, Card::PathToPeak,
                              Card::TeamYellsCheer, Card::RoseannesBackup,
                              Card::Lusamine, Card::ProfessorTuro,
                              Card::Powerglass}) {
        count += hand_count(card);
      }
    }

    const bool known_no_basic_prize = prizes_known() &&
        std::none_of(state_.prizes.begin(), state_.prizes.end(), is_basic);
    if (known_no_basic_prize) count += hand_count(Card::HisuianHeavyBall);

    // One Ultra Ball is reserved as the card being played. Duplicate search
    // cards and baseline Supporters are policy-safe only while another copy
    // remains, matching choose_discard().
    count += std::max(0, hand_count(Card::UltraBall) - 1);
    count += std::max(0, hand_count(Card::MysteriousTreasure) - 1);
    count += std::max(0, hand_count(Card::EarthenVessel) - 1);
    count += std::max(0, hand_count(Card::QuickBall) - 1);
    count += std::max(0, hand_count(Card::Arven) - 1);
    count += std::max(0, hand_count(Card::Crispin) - 1);
    count += std::max(0, hand_count(Card::Gladion) - 1);
    return count;
  }

  bool strict_jit_direct_vstar_access_secured() const {
    if (hand_count(Card::RegidragoVstar) > 0) return true;

    // An attached live Forest Seal Stone can search the missing VSTAR. Its
    // granted Tool Ability remains available through the modeled Path lock.
    if (!state_.vstar_power_used) {
      const auto has_live_stone = [](const Pokemon& pokemon) {
        return is_pokemon_v(pokemon.card) &&
               pokemon.tool == Tool::ForestSealStone;
      };
      if ((state_.active && has_live_stone(*state_.active)) ||
          std::any_of(state_.bench.begin(), state_.bench.end(),
                      has_live_stone)) {
        return true;
      }
    }

    if (item_locked() || !might_be_unseen(Card::RegidragoVstar)) {
      return false;
    }
    if (hand_count(Card::EvolutionIncense) > 0) return true;

    // Mysterious Treasure can use the protected payload itself as the legal
    // discard cost and then search Regidrago VSTAR in the same turn.
    if (hand_count(Card::MysteriousTreasure) > 0 &&
        choose_discard(true, true, true,
                       Card::MysteriousTreasure).has_value()) {
      return true;
    }

    // Ultra Ball supplies the same deterministic search only when two distinct
    // policy-admissible costs remain after reserving the Ultra Ball being played.
    return hand_count(Card::UltraBall) > 0 &&
           strict_jit_ultra_ball_policy_cost_count(true) >= 2;
  }

  bool payload_commitment_secured_this_turn() const {
    if (!strict_payload_timing()) return true;
    if (state_.turn < 2) return false;

    const auto line_is_secured = [this](const Pokemon& pokemon) {
      if (!strict_jit_attacker_energy_complete(pokemon) ||
          !strict_jit_attacker_can_be_active_this_turn(pokemon)) {
        return false;
      }

      if (pokemon.card == Card::RegidragoVstar) return true;
      if (pokemon.card != Card::RegidragoV ||
          pokemon.entered_turn >= state_.turn) {
        return false;
      }
      return strict_jit_direct_vstar_access_secured();
    };

    if (state_.active && line_is_secured(*state_.active)) return true;
    return std::any_of(state_.bench.begin(), state_.bench.end(),
                       line_is_secured);
  }

  bool payload_window_open() const {
    return !strict_payload_timing() ||
           payload_commitment_secured_this_turn();
  }

  void setup() {
    setup_same_turn_legacy();
    if (trace_ == nullptr || !trace_->enabled) return;

    for (std::string& line : trace_->lines) {
      if (line.starts_with("Policy rule:")) {
        line = "Policy rule: strict and matchup-flex JIT defer a protected "
               "payload until the current turn has a secured ready route; a "
               "payload already in discard remains ready on later turns.";
        break;
      }
    }
  }
