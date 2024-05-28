/************************************************************************************
 *
 * Copyright 2004 Craig Edwards <support@brainbox.cc>
 *
 * Core based on Sporks, the Learning Discord Bot, Craig Edwards (c) 2019.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ************************************************************************************/

#include "state.h"
#include "rps.h"
#include "time.h"
#include <algorithm>
#include <dpp/dpp.h>
#include <dpp/misc-enum.h>
#include <dpp/snowflake.h>
#include <fmt/format.h>
#include <optional>
#include <sporks/modules.h>
#include <sporks/statusfield.h>
#include <sporks/stringops.h>
#include <string>
#include <unistd.h>

state_t::state_t()
    : next_tick(time(nullptr)), creator(nullptr), terminating(false){};

state_t::state_t(RPSModule *_creator)
    : next_tick(time(nullptr)), creator(_creator), terminating(false) {
  creator->GetBot()->core->log(dpp::ll_debug,
                               fmt::format("state_t::state_t()"));
}

state_t::~state_t() {
  terminating = true;
  /* XXX: These are safety values, so that if we access a deleted state at any
   * point, it crashes sooner and can be identified easily in the debugger */
  creator = nullptr;
}

/* Games are a finite state machine, where the tick() function is called
 * periodically on each state_t object. The next_tick value indicates when the
 * tick() method should next be called, if the terminating flag is set then the
 * object is removed from the list of current games. Each cluster only stores a
 * game list for itself.
 */
void state_t::tick() {}

std::optional<rps_game> state_t::find_player_game(dpp::user &user) {
  if (games.empty()) {
    creator->GetBot()->core->log(dpp::ll_debug, "Player not in any queues");
    return std::nullopt;
  }

  creator->GetBot()->core->log(
      dpp::ll_debug, fmt::format("FPG - Games Count: {}", games.size()));
  for (const rps_game &temp_game : games) {
    creator->GetBot()->core->log(
        dpp::ll_debug,
        fmt::format("FPG - Player Count: {}", temp_game.players.size()));
    // for (const dpp::user &player : temp_game.players) {
    //   creator->GetBot()->core->log(
    //       dpp::ll_debug, fmt::format("FPG - Player ID: {}", player.id));
    // }
  }

  auto it =
      std::find_if(games.begin(), games.end(), [&user](const rps_game &game) {
        return std::any_of(game.players.begin(), game.players.end(),
                           [&user](const dpp::snowflake &player) {
                             return player == user.id;
                           });
      });

  /* If a game is found, return it */
  if (it != games.end()) {
    creator->GetBot()->core->log(dpp::ll_debug, "Player in a queue");
    return *it;
  }

  creator->GetBot()->core->log(dpp::ll_debug, "Player not in any queues");
  return std::nullopt;
}

std::optional<rps_game> state_t::find_open_game() {
  auto it = std::find_if(games.begin(), games.end(), [=](const rps_game &game) {
    return game.players.size() < 2;
  });

  if (it != games.end()) {
    return *it;
  }
  return std::nullopt;
}