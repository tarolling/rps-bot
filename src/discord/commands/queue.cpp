/************************************************************************************
 *
 * Copyright 2024 tarolling
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

#include <dpp/appcommand.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <rps/commands/queue.h>
#include <rps/embeds.h>
#include <rps/game.h>

dpp::slashcommand queue_command::register_command(dpp::cluster &bot) {
  return dpp::slashcommand("queue", "Enter into the RPS queue", bot.me.id)
      .set_dm_permission(true)
      .add_option(dpp::command_option(dpp::co_integer, "minutes",
                                      "Minutes to stay in queue"));
}

void queue_command::route(const dpp::slashcommand_t &event) {
  dpp::cluster *bot = event.from->creator;

  unsigned int player_lobby_id =
      game::find_player_lobby_id(event.command.usr.id);
  if (player_lobby_id != 0) {
    /* Game found */
    event.reply(dpp::message("You are already in a lobby.")
                    .set_flags(dpp::m_ephemeral));
    return;
  }

  /* No player game found, finding open game */
  unsigned int open_lobby_id = game::find_open_lobby_id();

  if (open_lobby_id == 0) {
    open_lobby_id = game::create_lobby();
  }

  game::add_player_to_lobby(open_lobby_id, event);
  unsigned int player_count = game::get_num_players(open_lobby_id);

  /* Send confirmation embed */
  event.reply(embeds::queue(event.command.usr, player_count));

  if (player_count == 2) {
    bot->log(dpp::ll_debug, fmt::format("Lobby {} started!", open_lobby_id));
    game::send_game_messages(open_lobby_id);
  }
}