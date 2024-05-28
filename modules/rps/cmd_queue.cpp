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

#include "commands.h"
#include "rps.h"
#include "state.h"
#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <fmt/format.h>
#include <mutex>
#include <sporks/modules.h>
#include <sporks/statusfield.h>
#include <sporks/stringops.h>
#include <string>
#include <utility>

using json = nlohmann::json;

command_queue_t::command_queue_t(class RPSModule *_creator,
                                 const std::string &_base_command, bool adm,
                                 const std::string &descr,
                                 std::vector<dpp::command_option> options)
    : command_t(_creator, _base_command, adm, descr, std::move(options)) {}

void command_queue_t::call(const in_cmd &cmd, std::stringstream &tokens,
                           guild_settings_t &settings,
                           const std::string &username, bool is_moderator,
                           dpp::channel *c, dpp::user *user) {
  size_t player_count = 0;

  {
    std::lock_guard<std::mutex> states_lock(creator->states_mutex);
    auto player_game = creator->state.find_player_game(*user);
    if (player_game) {
      /* Game found */
      creator->SimpleEmbed(cmd.interaction_token, cmd.command_id, settings, "",
                           "You are already in a game.", cmd.channel_id, "", "",
                           "");
      return;
    }

    /* No player game found, finding open game */
    auto open_game = creator->state.find_open_game();
    rps_game game;

    if (!open_game) {
      game = creator->state.games.emplace_back(game);
      game.players.push_back(user->id);
      player_count = 1;
    } else {
      game = open_game.value();
      game.players.push_back(user->id);
      player_count = 2;
    }
  }

  /* Send confirmation embed */
  std::string adjustment = (player_count == 1) ? " is" : "s are";
  std::vector<field_t> fields = {
      {"Want to join?", "Type `/queue` or `!queue` to join this lobby!",
       false}};
  creator->EmbedWithFields(
      cmd.interaction_token, cmd.command_id, settings,
      fmt::format("{} player{} in the queue", player_count, adjustment), fields,
      cmd.channel_id, "", "", user->get_avatar_url(1024),
      fmt::format("**{}** has joined.", username));
}