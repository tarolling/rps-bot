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

#include <dpp/appcommand.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/snowflake.h>
#include <dpp/timer.h>
#include <dpp/user.h>
#include <memory>

namespace game {
struct player_info {
  dpp::user player;
  dpp::slashcommand_t init_interaction;
  dpp::timer queue_timer{};
  std::string choice;
  unsigned int score{0};

  player_info(const dpp::user &p, dpp::slashcommand_t i)
      : player(p), init_interaction(std::move(i)) {}
};

struct rps_lobby {
  unsigned int id{0};
  unsigned int game_number{1};
  dpp::timer game_timer{};
  std::vector<std::shared_ptr<player_info>> players;
};

/**
 * @brief Finds a lobby that the player is in, if it exists
 *
 * @param user
 * @return unsigned int lobby id
 */
unsigned int find_player_lobby_id(const dpp::snowflake player_id);

/**
 * @brief Finds an open lobby avilable to join
 *
 * @return unsigned int lobby id
 */
unsigned int find_open_lobby_id();

/**
 * @brief Get the global lobby id object
 *
 * @return unsigned int
 */
unsigned int get_global_lobby_id();

/**
 * @brief Removes the lobby from the queue
 *
 * @param lobby_id
 * @param game_over
 */
void remove_lobby_from_queue(const unsigned int lobby_id, const bool game_over);

/**
 * @brief Create a lobby object
 *
 * @return unsigned int
 */
unsigned int create_lobby();

/**
 * @brief Add a player to the lobby
 *
 * @param lobby_id
 * @param event
 */
void add_player_to_lobby(const unsigned int lobby_id,
                         const dpp::slashcommand_t &event);

/**
 * @brief Set the player choice object
 *
 * @param player_id
 * @param choice
 */
void set_player_choice(const dpp::snowflake player_id,
                       const std::string &choice);

/**
 * @brief Get the player choice object
 *
 * @param player_id
 * @return std::string
 */
std::string get_player_choice(const dpp::snowflake player_id);

/**
 * @brief Get the num players
 *
 * @param lobby_id
 * @return unsigned int
 */
unsigned int get_num_players(const unsigned int lobby_id);

/**
 * @brief Get the lobby object
 *
 * @param lobby_id
 * @return rps_lobby
 */
rps_lobby get_lobby(const unsigned int lobby_id);

/**
 * @brief Get the player info object
 *
 * @param lobby_id
 * @param index
 * @return std::shared_ptr<player_info>
 */
std::shared_ptr<player_info> get_player_info(const unsigned int lobby_id,
                                             const unsigned int index);

/**
 * @brief Get the player ID
 *
 * @param lobby_id
 * @param player_index
 * @return dpp::snowflake
 */
dpp::snowflake get_player_id(const unsigned int lobby_id,
                             const unsigned int player_index);

/**
 * @brief Reset player choices
 *
 * @param lobby_id
 */
void reset_choices(const unsigned int lobby_id);

/**
 * @brief Increment player score
 *
 * @param lobby_id
 * @param player_num
 */
void increment_player_score(const unsigned int lobby_id,
                            const unsigned int player_num);

/**
 * @brief Get the current game number
 *
 * @param lobby_id
 * @return unsigned int
 */
unsigned int get_game_num(const unsigned int lobby_id);

/**
 * @brief Increment game number
 *
 * @param lobby_id
 */
void increment_game_num(const unsigned int lobby_id);

/**
 * @brief Check if both players have responded
 *
 * @param lobby_id
 * @return true
 * @return false
 */
bool check_both_responses(const unsigned int lobby_id);

/**
 * @brief Determine game winner
 *
 * @param lobby_id
 * @return std::string
 */
std::string determine_winner(const unsigned int lobby_id);

/**
 * @brief Helper for determine_winner
 *
 * @param player_one_choice
 * @param player_two_choice
 * @return std::string
 */
std::string calculate_winner(const std::string &player_one_choice,
                             const std::string &player_two_choice);

/**
 * @brief Get the player name
 *
 * @param lobby_id
 * @param index
 * @return std::string
 */
std::string get_player_name(const unsigned int lobby_id,
                            const unsigned int index);

/**
 * @brief Checks if the game is over
 *
 * @param lobby_id
 * @return true
 * @return false
 */
bool is_game_complete(const unsigned int lobby_id);

/**
 * @brief Get the player score
 *
 * @param lobby_id
 * @param index
 * @return unsigned int
 */
unsigned int get_player_score(const unsigned int lobby_id,
                              const unsigned int index);

/**
 * @brief Get the player interaction object
 *
 * @param lobby_id
 * @param index
 * @return dpp::slashcommand_t
 */
dpp::slashcommand_t get_player_interaction(const unsigned int lobby_id,
                                           const unsigned int index);

/**
 * @brief Starts a player's queue timer
 *
 * @param player_id
 * @param timer
 */
void start_queue_timer(const dpp::snowflake player_id, dpp::timer timer);

/**
 * @brief Deletes a player's queue timer
 *
 * @param player_id
 */
void clear_queue_timer(dpp::cluster *creator, const dpp::snowflake player_id);

/**
 * @brief Starts a player's game timer
 *
 * @param lobby_id
 * @param timer
 */
void start_game_timer(const unsigned int lobby_id, dpp::timer timer);

/**
 * @brief Deletes a player's game timer
 *
 * @param lobby_id
 */
void clear_game_timer(dpp::cluster *creator, const unsigned int lobby_id);
} // namespace game