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

#include <fmt/format.h>
#include <rps/game.h>
#include <rps/game_manager.h>
#include <thread>

namespace game_manager {
auto play_game(const dpp::slashcommand_t &event,
               const struct game::rps_lobby &game) {}

void handle_game(const dpp::slashcommand_t &event,
                 const struct game::rps_lobby &game) {
  dpp::cluster *bot = event.from->creator;
  if (game.players.size() != 2) {
    bot->log(dpp::ll_error, fmt::format("ERROR - Player vector size not 2"));
    return;
  }

  // std::thread p1(play_game, std::ref(bot), std::ref(game.players.front()));
  // std::thread p2(play_game, std::ref(bot), std::ref(game.players.back()));

  // p1.join();
  // p2.join();
}
} // namespace game_manager