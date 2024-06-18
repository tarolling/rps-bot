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
#include "rps/game.h"
#include <dpp/misc-enum.h>
#include <dpp/once.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <malloc.h>
#include <mutex>
#include <rps/command.h>
#include <rps/listeners.h>

#include <rps/commands/leave.h>
#include <rps/commands/queue.h>
#include <string>

namespace listeners {

std::vector<dpp::slashcommand> get_commands(dpp::cluster &bot) {
  return {
      register_command<queue_command>(bot),
      register_command<leave_command>(bot),
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

    auto set_presence = [&bot]() {
      bot.set_presence(dpp::presence(dpp::ps_online, dpp::at_game,
                                     "Is RPS all luck, or pure skill?"));
    };

    bot.start_timer([&bot, set_presence](dpp::timer t) { set_presence(); },
                    240);
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
  /* Instance of game */
  if (event.custom_id == "Rock" || event.custom_id == "Paper" ||
      event.custom_id == "Scissors") {
    event.reply();

    /* Spawn worker so sync methods don't block main event loop */
    std::thread worker(game::handle_game, std::ref(event));
    worker.detach();
  }
}
} // namespace listeners