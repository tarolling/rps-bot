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
#include <rps/domain/commands/leave.h>
#include <rps/domain/embeds.h>
#include <rps/domain/game.h>
#include <rps/domain/game_manager.h>

using namespace i18n;

dpp::slashcommand leave_command::register_command(dpp::cluster &bot) {
  return tr(dpp::slashcommand("c_leave", "d_leave", bot.me.id)
                .set_dm_permission(true));
}

void leave_command::route(const dpp::slashcommand_t &event) {
  unsigned int player_lobby_id =
      game::find_player_lobby_id(event.command.usr.id);
  if (player_lobby_id == 0) {
    /* Lobby not found */
    event.reply(
        dpp::message("You are not in a lobby.").set_flags(dpp::m_ephemeral));
    return;
  }

  if (game::get_num_players(player_lobby_id) == 2) {
    /* Match found */
    event.reply(dpp::message("You are already in a match.")
                    .set_flags(dpp::m_ephemeral));
    return;
  }

  /* Delete game */
  game_manager::handle_leave(event, player_lobby_id);

  /* Send confirmation embed */
  event.reply(embeds::leave(event, event.command.usr));
}