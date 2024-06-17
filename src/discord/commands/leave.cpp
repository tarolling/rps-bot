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
#include <rps/commands/leave.h>
#include <rps/game.h>

dpp::slashcommand leave_command::register_command(dpp::cluster &bot) {
  return dpp::slashcommand("leave", "Leave the RPS queue", bot.me.id)
      .set_dm_permission(true);
}

void leave_command::route(const dpp::slashcommand_t &event) {
  dpp::cluster *bot = event.from->creator;

  auto player_lobby_id = game::find_player_lobby_id(event.command.usr);
  if (player_lobby_id == 0) {
    /* Lobby not found */
    event.reply("You are not in a lobby.");
    return;
  }

  if (game::get_num_players(player_lobby_id) == 2) {
    /* Match found */
    event.reply("You are already in a match.");
    return;
  }

  /* Delete game */
  game::remove_lobby_from_queue(player_lobby_id, false);

  /* Send confirmation embed */
  dpp::message confirmation;
  confirmation.add_embed(
      dpp::embed()
          .set_title("0 players are in the queue")
          .set_description(fmt::format("**{}** has left.",
                                       event.command.usr.format_username()))
          .set_thumbnail(event.command.usr.get_avatar_url(1024))
          .set_footer(dpp::embed_footer()
                          .set_text("Powered By RPS Bot")
                          .set_icon("https://i.imgur.com/R19P703.png")));
  event.reply(confirmation);
}