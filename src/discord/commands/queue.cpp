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
#include <rps/game.h>

dpp::slashcommand queue_command::register_command(dpp::cluster &bot) {
  return dpp::slashcommand("queue", "Enter into the RPS queue", bot.me.id)
      .set_dm_permission(true)
      .add_option(dpp::command_option(dpp::co_integer, "minutes",
                                      "Minutes to stay in queue"));
}

void queue_command::route(const dpp::slashcommand_t &event) {
  dpp::cluster *bot = event.from->creator;
  unsigned long player_count = 0;
  unsigned int player_lobby_id =
      game::find_player_lobby_id(event.command.usr.id);
  if (player_lobby_id != 0) {
    /* Game found */
    event.reply("You are already in a lobby.");
    return;
  }

  /* No player game found, finding open game */
  unsigned int open_lobby_id = game::find_open_lobby_id();

  if (open_lobby_id == 0) {
    open_lobby_id = game::create_lobby();
    game::add_player_to_lobby(open_lobby_id, event);
    player_count = game::get_num_players(open_lobby_id);
  } else {
    game::add_player_to_lobby(open_lobby_id, event);
    bot->log(dpp::ll_debug, fmt::format("player count: {}",
                                        game::get_num_players(open_lobby_id)));
    player_count = game::get_num_players(open_lobby_id);
  }

  /* Send confirmation embed */
  dpp::message confirmation;
  std::string adjustment = (player_count == 1) ? " is" : "s are";
  confirmation.add_embed(
      dpp::embed()
          .set_title(
              fmt::format("{} player{} in the queue", player_count, adjustment))
          .add_field("Want to join?",
                     "Type `/queue` or `!queue` to join this lobby!")
          .set_description(fmt::format("**{}** has joined.",
                                       event.command.usr.format_username()))
          .set_thumbnail(event.command.usr.get_avatar_url(1024))
          .set_footer(dpp::embed_footer()
                          .set_text("Powered By RPS Bot")
                          .set_icon("https://i.imgur.com/R19P703.png")));
  event.reply(confirmation);

  if (player_count == 2) {
    game::send_game_messages(open_lobby_id);
  }
}