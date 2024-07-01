/************************************************************************************
 *
 * Copyright 1993,2001,2023 Craig Edwards <brain@ssod.org>
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
#include <dpp/misc-enum.h>
#include <dpp/once.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <malloc.h>
#include <rps/domain/command.h>
#include <rps/domain/embeds.h>
#include <rps/domain/game.h>
#include <rps/domain/game_manager.h>
#include <rps/domain/lang.h>
#include <rps/domain/listeners.h>

#include <rps/domain/commands/leave.h>
#include <rps/domain/commands/queue.h>
#include <rps/domain/commands/register.h>
#include <string>

namespace listeners {

std::vector<dpp::slashcommand> get_commands(dpp::cluster &bot) {
  return {
      register_command<queue_command>(bot),
      register_command<leave_command>(bot),
      register_command<register_command_t>(bot),
  };
}

std::string json_commands(dpp::cluster &bot) {
  json j = json::array();
  std::cerr << "Command List\n";
  auto v = get_commands(bot);
  std::cerr << "Total defined commands: " << v.size() << "\n\n";
  for (auto &s : v) {
    j.push_back(s.to_json(false));
  }
  return j.dump(1, '\t', false, json::error_handler_t::replace);
}

void on_ready(const dpp::ready_t &event) {
  dpp::cluster &bot = *event.from->creator;
  if (dpp::run_once<struct register_bot_commands>()) {
    if (bot.cluster_id == 0) {
      bot.global_bulk_command_create(get_commands(bot), [&bot](const auto &cc) {
        if (cc.is_error()) {
          bot.log(dpp::ll_error, cc.http_info.body);
        }
      });
    }

    auto set_presence = [&bot, event]() {
      bot.set_presence(dpp::presence(
          dpp::ps_online, dpp::at_game,
          fmt::format("{} lobbies created across {} servers!",
                      game::get_global_lobby_id(), event.guild_count)));
    };

    bot.start_timer([set_presence](dpp::timer t) { set_presence(); }, 240);
    bot.start_timer([&bot](dpp::timer t) { i18n::check_lang_reload(bot); }, 60);
    bot.start_timer(
        [](dpp::timer t) {
          /* Garbage collect free memory by consolidating free malloc() blocks
           */
          malloc_trim(0);
        },
        600);

    set_presence();
  }
}

void on_slashcommand(const dpp::slashcommand_t &event) {
  double start = dpp::utility::time_f();
  route_command(event);
  event.from->creator->log(
      dpp::ll_info,
      fmt::format("COMMAND: {} by {} ({} Guild: {}) Locale: {}, msecs: {:.02f}",
                  event.command.get_command_name(),
                  event.command.usr.format_username(), event.command.usr.id,
                  event.command.guild_id, event.command.locale,
                  (dpp::utility::time_f() - start) * 1000));
}

void on_buttonclick(const dpp::button_click_t &event) {
  event.reply();

  /* Create a copy of the original message's components and disable the button
   */
  std::vector<dpp::component> components = event.command.msg.components;
  for (auto &row : components) {
    for (auto &button : row.components) {
      button.disabled = true;
    }
  }

  /* Edit the original message to update the components */
  dpp::message m = event.command.msg;
  m.components = components;

  event.edit_response(m);

  /* Instance of game bans */
  if (event.custom_id.starts_with("ban")) {
    unsigned int player_lobby_id =
        game::find_player_lobby_id(event.command.get_issuing_user().id);
    if (player_lobby_id == 0) {
      return;
    }

    if (event.custom_id.ends_with('3')) {
      game::set_player_ban(player_lobby_id, event.command.get_issuing_user().id,
                           3);
      /* Spawn worker so sync methods don't block main event loop */
      std::thread worker(game_manager::send_ban_message, player_lobby_id);
      worker.detach();
      return;
    }

    if (event.custom_id.ends_with('4')) {
      game::set_player_ban(player_lobby_id, event.command.get_issuing_user().id,
                           4);
      /* Spawn worker so sync methods don't block main event loop */
      std::thread worker(game_manager::send_ban_message, player_lobby_id);
      worker.detach();
      return;
    }

    if (event.custom_id.ends_with('5')) {
      game::set_player_ban(player_lobby_id, event.command.get_issuing_user().id,
                           5);
      /* Spawn worker so sync methods don't block main event loop */
      std::thread worker(game_manager::send_ban_message, player_lobby_id);
      worker.detach();
      return;
    }
    return;
  }

  /* Instance of game choice */
  if (event.custom_id == "Rock" || event.custom_id == "Paper" ||
      event.custom_id == "Scissors") {
    unsigned int player_lobby_id =
        game::find_player_lobby_id(event.command.get_issuing_user().id);
    if (player_lobby_id == 0) {
      return;
    }

    /* Spawn worker so sync methods don't block main event loop */
    std::thread worker(game_manager::handle_choice, std::ref(event));
    worker.detach();
    return;
  }
}
} // namespace listeners