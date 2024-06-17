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

#include <dpp/coro/task.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <fmt/format.h>
#include <rps/game.h>
#include <rps/game_manager.h>
#include <thread>

namespace game_manager {
void play_game(game::rps_lobby &game, game::player_info &player_info) {
  dpp::cluster *bot = player_info.init_interaction.from->creator;
  dpp::message msg;
  msg.set_content(
      fmt::format("Lobby #{} - Game {}", game::get_global_lobby_id(), 1));
  msg.add_component(
      dpp::component()
          .add_component(dpp::component()
                             .set_type(dpp::component_type::cot_button)
                             .set_label("Rock")
                             .set_id("Rock")
                             .set_style(dpp::component_style::cos_primary))
          .add_component(dpp::component()
                             .set_type(dpp::component_type::cot_button)
                             .set_label("Paper")
                             .set_id("Paper")
                             .set_style(dpp::component_style::cos_primary))
          .add_component(dpp::component()
                             .set_type(dpp::component_type::cot_button)
                             .set_label("Scissors")
                             .set_id("Scissors")
                             .set_style(dpp::component_style::cos_primary)));

  bot->direct_message_create(player_info.player.id, msg);

  std::unique_lock<std::shared_mutex> lock(player_info.mtx);
  if (player_info.cv.wait_for(lock, std::chrono::seconds(30), [&player_info] {
        return !game::get_player_choice(player_info.player).empty();
      })) {
    // Player made a choice
    bot->log(dpp::ll_debug, fmt::format("{} selected {}.",
                                        player_info.player.format_username(),
                                        player_info.choice));
    bot->direct_message_create(
        player_info.player.id,
        fmt::format("You selected {}.", player_info.choice));
  } else {
    // Timeout
    bot->log(dpp::ll_debug, fmt::format("{} did not make a choice in time.",
                                        player_info.player.format_username()));
    bot->direct_message_create(player_info.player.id,
                               dpp::message("wow u didn't respond"));
    player_info.choice = "Timeout"; // Handle timeout case
  }
}

void handle_game(game::rps_lobby &game) {
  dpp::cluster *bot = game.players.back()->init_interaction.from->creator;
  if (game.players.size() != 2) {
    bot->log(dpp::ll_error,
             fmt::format("ERROR - Player vector size not 2 - was {}",
                         game.players.size()));
    return;
  }

  auto &player_one_info = *game.players.front();
  auto &player_two_info = *game.players.back();

  // TODO: Make constant for first-to # of games
  while (game::get_player_score(game.id, player_one_info.player) < 4 &&
         game::get_player_score(game.id, player_two_info.player) < 4) {
    game::reset_choices(game.id);
    std::thread p1(play_game, std::ref(game), std::ref(player_one_info));
    std::thread p2(play_game, std::ref(game), std::ref(player_two_info));

    p1.join();
    p2.join();
  }

  game::remove_lobby_from_queue(game.id, true);
}
} // namespace game_manager