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

#include <cstdint>
#include <dpp/appcommand.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/timer.h>
#include <rps/domain/commands/leave.h>
#include <rps/domain/commands/queue.h>
#include <rps/domain/embeds.h>
#include <rps/domain/game.h>
#include <variant>

using namespace i18n;

dpp::slashcommand queue_command::register_command(dpp::cluster &bot) {
  return tr(dpp::slashcommand("c_queue", "d_queue", bot.me.id)
                .set_dm_permission(true)
                .add_option(dpp::command_option(dpp::co_integer, "co_queue",
                                                "cod_queue")
                                .set_min_value(1)
                                .set_max_value(60)));
}

void queue_command::route(const dpp::slashcommand_t &event) {
  dpp::cluster *bot = event.from->creator;

  unsigned int player_lobby_id =
      game::find_player_lobby_id(event.command.usr.id);
  if (player_lobby_id != 0) {
    /* Game found */
    event.reply(dpp::message(tr("R_PLAYER_ALREADY_IN_LOBBY", event))
                    .set_flags(dpp::m_ephemeral));
    return;
  }

  /* No player game found, finding open game */
  unsigned int open_lobby_id = game::find_open_lobby_id();

  if (open_lobby_id == 0) {
    open_lobby_id = game::create_lobby();
  }

  game::add_player_to_lobby(open_lobby_id, event);

  long queue_time = 0;
  if (std::holds_alternative<std::monostate>(
          event.get_parameter(tr("CO_QUEUE", event)))) {
    queue_time = config::get("default_queue_time");
  } else {
    queue_time =
        std::get<std::int64_t>(event.get_parameter(tr("CO_QUEUE", event)));
  }

  game::start_queue_timer(
      event.command.usr.id,
      event.from->creator->start_timer(
          [=](unsigned long t) {
            game::remove_lobby_from_queue(open_lobby_id, false);
            event.from->creator->message_create(
                embeds::leave(event, event.command.usr)
                    .set_channel_id(event.command.channel_id));
            event.from->creator->stop_timer(t);
          },
          60 * queue_time));

  const unsigned int player_count = game::get_num_players(open_lobby_id);

  /* Send confirmation embed */
  event.reply(embeds::queue(event, event.command.usr, player_count));

  if (player_count == 2) {
    bot->log(dpp::ll_debug, fmt::format("Lobby {} started!", open_lobby_id));
    std::thread worker(game::send_game_messages, open_lobby_id);
    worker.detach();
  }
}