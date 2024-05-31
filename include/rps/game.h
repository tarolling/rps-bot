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

#pragma once

#include <dpp/dpp.h>
#include <dpp/user.h>

namespace game {
struct rps_lobby {
  unsigned int id;
  std::vector<dpp::user> players;
};

/**
 * @brief Initialize global game state
 *
 * @param bot creating D++ cluster
 */
void init(dpp::cluster &bot);

/**
 * @brief Finds a lobby that the player is in, if it exists
 *
 * @param user
 * @return std::optional<rps_lobby>
 */
std::optional<rps_lobby> find_player_lobby(const dpp::user &player);

/**
 * @brief Finds an open lobby avilable to join
 *
 * @return std::optional<std::reference_wrapper<rps_lobby>>
 */
std::optional<std::reference_wrapper<rps_lobby>> find_open_lobby();

unsigned int get_global_lobby_id();
void increment_global_lobby_id();
void decrement_global_lobby_id();
void add_lobby_to_queue(const struct rps_lobby &lobby);
void remove_lobby_from_queue(const struct rps_lobby &lobby);
void add_player_to_lobby(const struct rps_lobby &lobby,
                         const dpp::user &player);
void remove_player_from_lobby(const struct rps_lobby &lobby,
                              const dpp::user &player);
} // namespace game
