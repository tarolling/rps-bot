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
  size_t player_count = 0;
  struct game::rps_lobby match;
  auto player_lobby = game::find_player_lobby(event.command.usr);
  if (player_lobby) {
    /* Game found */
    event.reply("You are already in a lobby.");
    return;
  }

  /* No player game found, finding open game */
  auto open_lobby = game::find_open_lobby();
  if (!open_lobby) {
    struct game::rps_lobby lobby;
    game::increment_global_lobby_id();
    lobby.id = game::get_global_lobby_id();
    lobby.players.push_back(event.command.usr);
    game::add_lobby_to_queue(lobby);
    player_count = 1;
  } else {
    game::rps_lobby &found_lobby = open_lobby.value().get();
    found_lobby.id = game::get_global_lobby_id();
    found_lobby.players.push_back(event.command.usr);
    match = found_lobby;
    player_count = 2;
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
}