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
#include <rps/embeds.h>
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

/**
 * @brief Get the player info object
 *
 * PROTECTED
 * @param player_id
 * @return std::shared_ptr<player_info>
 */
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
      return lobby.players.at(player_index)->player.id;
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
      lobby.players[player_num - 1]->score++;
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
                            const unsigned int player_index) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players.at(player_index)->player.format_username();
    }
  }
  return "";
}

void send_game_messages(const unsigned int lobby_id) {
  creator->log(dpp::ll_debug, "HEY WE MADE IT MOTHERFUCKA");
  rps_lobby found_lobby;
  unsigned int game_num = 0;

  {
    std::shared_lock<std::shared_mutex> game_lock(game_mutex);

    found_lobby = get_lobby(lobby_id);
    game_num = get_game_num(lobby_id);
  }

  creator->log(dpp::ll_debug, "OTHERFUCKA what");

  if (game_num == 1) {
    for (auto &player_info : found_lobby.players) {
      clear_queue_timer(player_info->player.id);
    }
  }

  std::string player_one_name = get_player_name(lobby_id, 0);
  unsigned int player_one_score = 0;
  dpp::snowflake player_one_id = get_player_id(lobby_id, 0);
  std::string player_two_name = get_player_name(lobby_id, 1);
  unsigned int player_two_score = 0;
  dpp::snowflake player_two_id = get_player_id(lobby_id, 1);

  creator->log(dpp::ll_debug, "what about here?");

  {
    std::lock_guard<std::shared_mutex> game_lock(game_mutex);
    player_one_score = get_player_score(lobby_id, player_one_id);
    player_two_score = get_player_score(lobby_id, player_two_id);
  }

  creator->log(dpp::ll_debug, "got here, any lock?");

  for (const auto &player_info : found_lobby.players) {
    start_game_timer(player_info->player.id,
                     creator->start_timer(
                         [=](unsigned long t) {
                           creator->stop_timer(t);
                           handle_timeout(player_info->player.id);
                         },
                         GAME_TIMEOUT));
    creator->direct_message_create(
        player_info->player.id,
        embeds::game(lobby_id, get_game_num(lobby_id), player_one_name,
                     player_one_score, player_two_name, player_two_score));
  }

  creator->log(dpp::ll_debug, "we got here which is baffling");
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
  dpp::message msg_win =
      embeds::game_result(get_game_num(lobby_id), get_player_name(lobby_id, 0),
                          get_player_choice(get_player_id(lobby_id, 0)),
                          get_player_name(lobby_id, 1),
                          get_player_choice(get_player_id(lobby_id, 1)), "WIN");
  dpp::message msg_loss = embeds::game_result(
      get_game_num(lobby_id), get_player_name(lobby_id, 0),
      get_player_choice(get_player_id(lobby_id, 0)),
      get_player_name(lobby_id, 1),
      get_player_choice(get_player_id(lobby_id, 1)), "LOSS");
  if (draw) {
    msg_win = embeds::game_result(
        get_game_num(lobby_id), get_player_name(lobby_id, 0),
        get_player_choice(get_player_id(lobby_id, 0)),
        get_player_name(lobby_id, 1),
        get_player_choice(get_player_id(lobby_id, 1)), "DRAW");
    msg_loss = embeds::game_result(
        get_game_num(lobby_id), get_player_name(lobby_id, 0),
        get_player_choice(get_player_id(lobby_id, 0)),
        get_player_name(lobby_id, 1),
        get_player_choice(get_player_id(lobby_id, 1)), "DRAW");
  }

  creator->direct_message_create(get_player_id(lobby_id, winner - 1), msg_win);
  creator->direct_message_create(get_player_id(lobby_id, loser - 1), msg_loss);
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
void clear_queue_timer(const dpp::snowflake player_id) {
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

void start_game_timer(const unsigned int lobby_id, dpp::timer timer) {}

void clear_game_timer(const unsigned int lobby_id) {}

void handle_choice(const dpp::button_click_t &event) {
  /* Find player lobby */
  unsigned int player_lobby_id =
      find_player_lobby_id(event.command.get_issuing_user().id);

  rps_lobby player_lobby = get_lobby(player_lobby_id);

  /* 1. Go set the choice, then send a confirmation message */
  clear_queue_timer(event.command.get_issuing_user().id);
  set_player_choice(event.command.get_issuing_user().id, event.custom_id);
  creator->direct_message_create(
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

void handle_timeout(const unsigned int lobby_id) {
  creator->log(dpp::ll_debug, "uhmmmm someone timed out :(");
}
} // namespace game
