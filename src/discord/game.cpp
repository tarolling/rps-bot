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

#include "rps/embeds.h"
#include <dpp/exception.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>
#include <dpp/snowflake.h>
#include <fmt/format.h>
#include <list>
#include <memory>
#include <mutex>
#include <rps/game.h>

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
 * @brief Creating rps_bot
 */
dpp::cluster *creator{nullptr};

void init(dpp::cluster &bot) {
  creator = &bot;
  creator->log(dpp::ll_info, "Game state initialized");
}

unsigned int find_player_lobby_id(const dpp::snowflake player_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);

  if (lobby_queue.empty()) {
    creator->log(dpp::ll_debug, "No lobbies available");
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

unsigned int find_open_lobby_id() {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);

  for (const auto &lobby : lobby_queue) {
    if (lobby.players.size() < 2) {
      return lobby.id;
    }
  }

  return 0;
}

unsigned int get_global_lobby_id() {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  return global_lobby_id;
}

/**
 * @brief Type here.
 */
void increment_global_lobby_id() {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  ++global_lobby_id;
}

void decrement_global_lobby_id() {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  --global_lobby_id;
}

void remove_lobby_from_queue(const unsigned int lobby_id, bool game_over) {
  if (!game_over) {
    decrement_global_lobby_id();
  }

  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto it = lobby_queue.begin(); it != lobby_queue.end(); ++it) {
    if (it->id == lobby_id) {
      lobby_queue.erase(it);
      return;
    }
  }
}

unsigned int create_lobby() {
  rps_lobby lobby;
  increment_global_lobby_id();
  lobby.id = get_global_lobby_id();

  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  lobby_queue.push_back(lobby);
  return lobby.id;
}

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

void remove_player_from_lobby(const unsigned int lobby_id,
                              const dpp::snowflake player_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &it : lobby_queue) {
    if (it.id == lobby_id) {
      auto it_player = std::find_if(
          it.players.begin(), it.players.end(),
          [player_id](const std::shared_ptr<player_info> &player_info) {
            return player_info->player.id == player_id;
          });
      if (it_player != it.players.end()) {
        it.players.erase(it_player);
      }
      return;
    }
  }
}

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

unsigned int get_num_players(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players.size();
    }
  }

  return 0;
}

rps_lobby get_lobby(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby;
    }
  }
}

unsigned int get_player_score(const unsigned int lobby_id,
                              const dpp::snowflake &player_id) {
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      for (const auto &player_info : lobby.players) {
        if (player_info->player.id == player_id) {
          return player_info->score;
        }
      }
    }
  }
}

std::shared_ptr<player_info> get_player_info(const dpp::snowflake &player_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    for (const auto &player_info : lobby.players) {
      if (player_info->player.id == player_id) {
        return player_info;
      }
    }
  }
  return nullptr;
}

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

void increment_player_score(const unsigned int lobby_id,
                            const unsigned int player_num) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      lobby.players[player_num - 1]->score++;
      return;
    }
  }
}

unsigned int get_game_num(const unsigned int lobby_id) {
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.game_number;
    }
  }
  return 0;
}

void increment_game_num(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      lobby.game_number++;
    }
  }
}

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
  rps_lobby player_lobby;
  {
    std::lock_guard<std::shared_mutex> game_lock(game_mutex);
    for (const auto &lobby : lobby_queue) {
      if (lobby.id == lobby_id) {
        player_lobby = lobby;
        break;
      }
    }
  }
  std::string player_one_choice =
      get_player_choice(player_lobby.players.front()->player.id);
  std::string player_two_choice =
      get_player_choice(player_lobby.players.back()->player.id);

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
}

void send_game_messages(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);

  rps_lobby found_lobby;
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      found_lobby = lobby;
      break;
    }
  }

  std::string player_one_name =
      found_lobby.players.front()->player.format_username();
  unsigned int player_one_score =
      get_player_score(lobby_id, found_lobby.players.front()->player.id);
  std::string player_two_name =
      found_lobby.players.back()->player.format_username();
  unsigned int player_two_score =
      get_player_score(lobby_id, found_lobby.players.back()->player.id);

  for (const auto &player_info : found_lobby.players) {
    player_info->init_interaction.from->creator->direct_message_create_sync(
        player_info->player.id,
        embeds::game(lobby_id, get_game_num(lobby_id), player_one_name,
                     player_one_score, player_two_name, player_two_score));
  }
}

bool is_game_complete(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players.front()->score == 4 ||
             lobby.players.back()->score == 4;
    }
  }
  return false;
}

void send_result_messages(const unsigned int lobby_id,
                          const unsigned int winner, const unsigned int loser,
                          bool draw) {
  dpp::message msg_win = dpp::message("Game win!");
  dpp::message msg_loss = dpp::message("Game loss.");
  if (draw) {
    msg_win = dpp::message("Game draw.");
    msg_loss = dpp::message("Game draw.");
  }

  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      lobby.players.at(winner - 1)
          ->init_interaction.from->creator->direct_message_create_sync(
              lobby.players.at(winner - 1)->player.id, msg_win);
      lobby.players.at(loser - 1)
          ->init_interaction.from->creator->direct_message_create_sync(
              lobby.players.at(loser - 1)->player.id, msg_loss);
    }
  }
}

void handle_game(const dpp::button_click_t &event) {
  /* Find player lobby */
  unsigned int player_lobby_id =
      find_player_lobby_id(event.command.get_issuing_user().id);

  /* 1. Go set the choice, then send a confirmation message */
  set_player_choice(event.command.get_issuing_user().id, event.custom_id);
  event.from->creator->direct_message_create_sync(
      event.command.get_issuing_user().id,
      dpp::message(fmt::format("You selected {}! Waiting for opponent...",
                               event.custom_id)));

  /* 2. If both choices are selected, determine who won and increment winner
   */
  if (check_both_responses(player_lobby_id)) {
    std::string result = determine_winner(player_lobby_id);
    if (result == "1") {
      increment_player_score(player_lobby_id, 1);
      send_result_messages(player_lobby_id, 1, 2);
    } else if (result == "2") {
      increment_player_score(player_lobby_id, 2);
      send_result_messages(player_lobby_id, 2, 1);
    } else {
      send_result_messages(player_lobby_id, 1, 2, true);
    }

    if (is_game_complete(player_lobby_id)) {
      /* Finish up */
      remove_lobby_from_queue(player_lobby_id, true);
    } else {
      /* Send message */
      increment_game_num(player_lobby_id);
      reset_choices(player_lobby_id);
      send_game_messages(player_lobby_id);
    }
  }
}

} // namespace game