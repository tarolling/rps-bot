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

#include <dpp/misc-enum.h>
#include <rps/game.h>

namespace game {
/**
 * @brief Tracks global lobby ID
 */
unsigned int global_lobby_id;

/**
 * @brief Important that only one client reads/writes to game state!
 */
std::shared_mutex game_mutex;

/**
 * @brief Collection of pending lobbies
 */
std::list<struct rps_lobby> lobby_queue;

/**
 * @brief Creating D++ cluster, used for logging
 */
dpp::cluster *creator{nullptr};

void init(dpp::cluster &bot) {
  creator = &bot;
  creator->log(dpp::ll_info, "Game state initialized");
}

std::optional<rps_lobby> find_player_lobby(const dpp::user &player) {
  if (lobby_queue.empty()) {
    creator->log(dpp::ll_debug, "Player not in any lobbies");
    return std::nullopt;
  }

  auto it = std::find_if(lobby_queue.begin(), lobby_queue.end(),
                         [&player](const rps_lobby &lobby) {
                           return std::any_of(
                               lobby.players.begin(), lobby.players.end(),
                               [&player](const dpp::user &player_cpy) {
                                 return player_cpy.id == player.id;
                               });
                         });

  /* If a game is found, return it */
  if (it != lobby_queue.end()) {
    creator->log(dpp::ll_debug, "Player in a lobby");
    return *it;
  }

  creator->log(dpp::ll_debug, "Player not in any lobbies");
  return std::nullopt;
}

std::optional<std::reference_wrapper<rps_lobby>> find_open_lobby() {
  auto it = std::find_if(
      lobby_queue.begin(), lobby_queue.end(),
      [=](const rps_lobby &lobby) { return lobby.players.size() < 2; });

  if (it != lobby_queue.end()) {
    return *it;
  }
  return std::nullopt;
}

unsigned int get_global_lobby_id() {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  return global_lobby_id;
}

void increment_global_lobby_id() {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  ++global_lobby_id;
}

void decrement_global_lobby_id() {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  --global_lobby_id;
}
void add_lobby_to_queue(const struct rps_lobby &lobby) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  lobby_queue.push_back(lobby);
}
void remove_lobby_from_queue(const struct rps_lobby &lobby) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto it = lobby_queue.begin(); it != lobby_queue.end(); ++it) {
    if (it->id == lobby.id) {
      lobby_queue.erase(it);
      return;
    }
  }
}
void add_player_to_lobby(const struct rps_lobby &lobby,
                         const dpp::user &player);
void remove_player_from_lobby(const struct rps_lobby &lobby,
                              const dpp::user &player);
} // namespace game