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

#include <dpp/dispatcher.h>
#include <dpp/exception.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/snowflake.h>
#include <dpp/timer.h>
#include <fmt/format.h>
#include <list>
#include <memory>
#include <mutex>
#include <rps/domain/embeds.h>
#include <rps/domain/game.h>

namespace game {

/**
 * @brief Tracks global lobby ID
 */
unsigned int global_lobby_id{0};

/**
 * @brief Important that only one client reads/writes to game state!
 */
std::shared_mutex game_mutex;

/**
 * @brief Collection of pending lobbies
 */
std::list<rps_lobby> lobby_queue;

/**
 * @brief PROTECTED
 *
 * @param player_id
 * @return unsigned int
 */
unsigned int find_player_lobby_id(const dpp::snowflake player_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);

  if (lobby_queue.empty()) {
    return 0;
  }

  for (const auto &lobby : lobby_queue) {
    for (const auto &player_info : lobby.players) {
      if (player_info->player.id == player_id) {
        return lobby.id;
      }
    }
  }

  return 0;
}

/**
 * @brief PROTECTED
 *
 * @return unsigned int
 */
unsigned int find_open_lobby_id() {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);

  for (const auto &lobby : lobby_queue) {
    if (lobby.players.size() < 2) {
      return lobby.id;
    }
  }

  return 0;
}

/**
 * @brief Get the global lobby id object
 * PROTECTED
 * @return unsigned int
 */
unsigned int get_global_lobby_id() {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  return global_lobby_id;
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 * @param game_over
 */
void remove_lobby_from_queue(const unsigned int lobby_id, bool game_over) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);

  if (!game_over) {
    global_lobby_id--;
  }

  for (auto it = lobby_queue.begin(); it != lobby_queue.end(); ++it) {
    if (it->id == lobby_id) {
      lobby_queue.erase(it);
      return;
    }
  }
}

/**
 * @brief Create a lobby object
 * PROTECTED
 * @return unsigned int
 */
unsigned int create_lobby() {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);

  rps_lobby lobby;
  lobby.id = ++global_lobby_id;
  lobby_queue.push_back(lobby);
  return lobby.id;
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 * @param event
 */
void add_player_to_lobby(const unsigned int lobby_id,
                         const dpp::slashcommand_t &event) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &it : lobby_queue) {
    if (it.id == lobby_id) {
      it.players.emplace_back(
          std::make_shared<player_info>(event.command.usr, event));
    }
  }
}

/**
 * @brief Set the player choice object
 *
 * PROTECTED
 * @param player_id
 * @param choice
 */
void set_player_choice(const dpp::snowflake player_id,
                       const std::string &choice) {
  unsigned int player_lobby_id = find_player_lobby_id(player_id);

  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  if (player_lobby_id == 0) {
    return;
  }

  for (auto &lobby : lobby_queue) {
    for (auto &player_info : lobby.players) {
      if (player_info->player.id == player_id) {
        player_info->choice = choice;
        return;
      }
    }
  }
}

/**
 * @brief Get the player choice object
 *
 * PROTECTED
 * @param player_id
 * @return std::string
 */
std::string get_player_choice(const dpp::snowflake player_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    for (auto &player_info : lobby.players) {
      if (player_info->player.id == player_id) {
        return player_info->choice;
      }
    }
  }

  return "";
}

/**
 * @brief Get the num players object
 *
 * PROTECTED
 * @param lobby_id
 * @return unsigned int
 */
unsigned int get_num_players(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players.size();
    }
  }

  return 0;
}

/**
 * @brief Get the lobby object
 *
 * PROTECTED
 * @param lobby_id
 * @return rps_lobby
 */
rps_lobby get_lobby(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby;
    }
  }
  return {};
}

/**
 * @brief Get the player score object
 *
 * PROTECTED
 * @param lobby_id
 * @param index
 * @return unsigned int
 */
unsigned int get_player_score(const unsigned int lobby_id,
                              const unsigned int index) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players[index]->score;
    }
  }
  return 0;
}

/**
 * @brief Get the player info object
 *
 * PROTECTED
 * @param player_id
 * @return std::shared_ptr<player_info>
 */
std::shared_ptr<player_info> get_player_info(const unsigned int lobby_id,
                                             const unsigned int index) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players[index];
    }
  }
  return nullptr;
}

/**
 * @brief Get the player id object
 *
 * PROTECTED
 * @param lobby_id
 * @param player_index
 * @return dpp::snowflake
 */
dpp::snowflake get_player_id(const unsigned int lobby_id,
                             const unsigned int player_index) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players[player_index]->player.id;
    }
  }
  return 0;
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 */
void reset_choices(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      for (auto &player_info : lobby.players) {
        player_info->choice = "";
      }
      return;
    }
  }
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 * @param player_num
 */
void increment_player_score(const unsigned int lobby_id,
                            const unsigned int player_num) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      lobby.players[player_num]->score++;
      return;
    }
  }
}

/**
 * @brief Get the game num object
 *
 * PROTECTED
 * @param lobby_id
 * @return unsigned int
 */
unsigned int get_game_num(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.game_number;
    }
  }
  return 0;
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 */
void increment_game_num(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      lobby.game_number++;
    }
  }
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 * @return true if both players have responded
 * @return false if at least one player has not responded
 */
bool check_both_responses(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return !lobby.players.front()->choice.empty() &&
             !lobby.players.back()->choice.empty();
    }
  }
  return false;
}

std::string determine_winner(const unsigned int lobby_id) {
  rps_lobby player_lobby = get_lobby(lobby_id);
  std::string player_one_choice = get_player_choice(get_player_id(lobby_id, 0));
  std::string player_two_choice = get_player_choice(get_player_id(lobby_id, 1));

  return calculate_winner(player_one_choice, player_two_choice);
}

std::string calculate_winner(const std::string &player_one_choice,
                             const std::string &player_two_choice) {
  if (player_one_choice.empty() || player_two_choice.empty()) {
    if (!player_one_choice.empty()) {
      return "1";
    }
    if (!player_two_choice.empty()) {
      return "2";
    }
  }
  if (player_one_choice == "Rock") {
    if (player_two_choice == "Rock") {
      return "D";
    }
    if (player_two_choice == "Paper") {
      return "2";
    }
    if (player_two_choice == "Scissors") {
      return "1";
    }
  }
  if (player_one_choice == "Paper") {
    if (player_two_choice == "Rock") {
      return "1";
    }
    if (player_two_choice == "Paper") {
      return "D";
    }
    if (player_two_choice == "Scissors") {
      return "2";
    }
  }
  if (player_one_choice == "Scissors") {
    if (player_two_choice == "Rock") {
      return "2";
    }
    if (player_two_choice == "Paper") {
      return "1";
    }
    if (player_two_choice == "Scissors") {
      return "D";
    }
  }
  return "FF";
}

/**
 * @brief Get the player name object
 *
 * PROTECTED
 * @param lobby_id
 * @param player_index
 * @return std::string
 */
std::string get_player_name(const unsigned int lobby_id,
                            const unsigned int index) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players.at(index)->player.format_username();
    }
  }
  return "";
}

bool is_game_complete(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players.front()->score == lobby.first_to ||
             lobby.players.back()->score == lobby.first_to;
    }
  }
  return false;
}

/**
 * @brief Get the player interaction object
 *
 * PROTECTED
 * @param lobby_id
 * @param index
 * @return dpp::slashcommand_t
 */
dpp::slashcommand_t get_player_interaction(const unsigned int lobby_id,
                                           const unsigned int index) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players[index]->init_interaction;
    }
  }
  return {};
}

/**
 * @brief PROTECTED
 *
 * @param player_id
 * @param timer
 */
void start_queue_timer(const dpp::snowflake player_id, dpp::timer timer) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    for (auto &player_info : lobby.players) {
      if (player_info->player.id == player_id) {
        player_info->queue_timer = timer;
        return;
      }
    }
  }
}

/**
 * @brief PROTECTED
 *
 * @param player_id
 */
void clear_queue_timer(dpp::cluster *creator, const dpp::snowflake player_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    for (auto &player_info : lobby.players) {
      if (player_info->player.id == player_id) {
        creator->stop_timer(player_info->queue_timer);
        return;
      }
    }
  }
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 * @param timer
 */
void start_game_timer(const unsigned int lobby_id, dpp::timer timer) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      lobby.game_timer = timer;
      return;
    }
  }
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 */
void clear_game_timer(dpp::cluster *creator, const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      creator->stop_timer(lobby.game_timer);
      return;
    }
  }
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 * @param player_id
 */
void set_player_ban(const unsigned int lobby_id, const dpp::snowflake player_id,
                    const unsigned int ban) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      for (auto &player_info : lobby.players) {
        if (player_info->player.id == player_id) {
          player_info->ban = ban;
          return;
        }
      }
    }
  }
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 * @param index
 * @return std::string
 */
unsigned int get_player_ban(const unsigned int lobby_id,
                            const unsigned int index) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players[index]->ban;
    }
  }
  return 0;
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 * @param first_to
 */
void set_first_to(const unsigned int lobby_id, const unsigned int first_to) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      lobby.first_to = first_to;
      return;
    }
  }
}

/**
 * @brief PROTECTED
 *
 * @param lobby_id
 * @return unsigned int
 */
unsigned int get_first_to(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.first_to;
    }
  }
  return 4;
}
} // namespace game