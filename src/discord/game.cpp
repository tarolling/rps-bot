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
#include <dpp/restresults.h>
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
  creator->log(dpp::ll_debug, fmt::format("lobby id: {}", lobby_id));

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

unsigned int get_player_score(const unsigned int lobby_id,
                              const unsigned int index) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players.at(index)->score;
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
      return lobby.players.at(index);
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
 * @brief Set the player message object
 *
 * PROTECTED
 * @param lobby_id
 * @param message
 */
void set_player_message(const unsigned int lobby_id, const unsigned int index,
                        const dpp::message &message) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      lobby.players.at(index)->game_message =
          std::make_shared<dpp::message>(message);
      return;
    }
  }
}

std::shared_ptr<dpp::message> get_player_message(const unsigned int lobby_id,
                                                 const unsigned int index) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      return lobby.players.at(index)->game_message;
    }
  }
  return {};
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

void send_game_messages(const unsigned int lobby_id) {
  rps_lobby found_lobby = get_lobby(lobby_id);
  if (found_lobby.id == 0) {
    creator->log(dpp::ll_critical, "Could not find lobby");
    return;
  }
  unsigned int game_num = get_game_num(lobby_id);

  if (game_num == 1) {
    for (auto &player_info : found_lobby.players) {
      clear_queue_timer(player_info->player.id);
    }
  }

  std::string player_one_name = get_player_name(lobby_id, 0);
  unsigned int player_one_score = get_player_score(lobby_id, 0);
  std::string player_two_name = get_player_name(lobby_id, 1);
  unsigned int player_two_score = get_player_score(lobby_id, 1);

  start_game_timer(lobby_id, creator->start_timer(
                                 [=](unsigned long t) {
                                   handle_timeout(lobby_id);
                                   creator->stop_timer(t);
                                 },
                                 GAME_TIMEOUT));

  if (game_num == 1) {
    for (unsigned long i = 0; i < found_lobby.players.size(); ++i) {
      creator->direct_message_create(
          get_player_id(lobby_id, i),
          dpp::message()
              .add_embed(embeds::game(lobby_id, game_num, player_one_name,
                                      player_one_score, player_two_name,
                                      player_two_score))
              .add_component(embeds::game_buttons()),
          [lobby_id, i](const dpp::confirmation_callback_t &callback) {
            if (callback.is_error()) {
              creator->log(
                  dpp::ll_error,
                  fmt::format("Error in sending player {} a message - lobby {}",
                              i + 1, lobby_id));
              return;
            }
            set_player_message(lobby_id, i, callback.get<dpp::message>());
          });
    }
  } else {
    for (unsigned long i = 0; i < found_lobby.players.size(); ++i) {
      auto player_message = get_player_message(lobby_id, i);
      player_message->embeds[0] =
          embeds::game(lobby_id, game_num, player_one_name, player_one_score,
                       player_two_name, player_two_score);
      player_message->components[0] = embeds::game_buttons();
      creator->message_edit(*player_message);
      // creator->message_get(
      //     player_message->id, player_message->channel_id,
      //     [=](const dpp::confirmation_callback_t &callback) {
      //       if (callback.is_error()) {
      //         creator->log(
      //             dpp::ll_error,
      //             fmt::format(
      //                 "Error in editing message for player {} - lobby {}",
      //                 i + 1, lobby_id));
      //         return;
      //       }

      //       auto message = callback.get<dpp::message>();
      //       message = embeds::game(lobby_id, game_num, player_one_name,
      //                              player_one_score, player_two_name,
      //                              player_two_score);
      //       creator->message_edit(message);
      //       set_player_message(lobby_id, i, message);
      //     });
    }
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
  unsigned int game_num = get_game_num(lobby_id);
  std::string player_one_name = get_player_name(lobby_id, 0);
  std::string player_one_choice = get_player_choice(get_player_id(lobby_id, 0));
  std::string player_two_name = get_player_name(lobby_id, 1);
  std::string player_two_choice = get_player_choice(get_player_id(lobby_id, 1));

  dpp::message msg_win = dpp::message().add_embed(
      embeds::game_result(game_num, player_one_name, player_one_choice,
                          player_two_name, player_two_choice, "WIN"));
  dpp::message msg_loss = dpp::message().add_embed(
      embeds::game_result(game_num, player_one_name, player_one_choice,
                          player_two_name, player_two_choice, "LOSS"));
  if (draw) {
    msg_win = dpp::message().add_embed(
        embeds::game_result(game_num, player_one_name, player_one_choice,
                            player_two_name, player_two_choice, "DRAW"));
    msg_loss = dpp::message().add_embed(
        embeds::game_result(game_num, player_one_name, player_one_choice,
                            player_two_name, player_two_choice, "DRAW"));
  }

  /* These need to be sent before the next game message is sent, so we make
   * them synchronous */
  creator->direct_message_create_sync(get_player_id(lobby_id, winner), msg_win);
  creator->direct_message_create_sync(get_player_id(lobby_id, loser), msg_loss);
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
void clear_game_timer(const unsigned int lobby_id) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      creator->stop_timer(lobby.game_timer);
      return;
    }
  }
}

void send_match_results(const unsigned int lobby_id, const dpp::user &winner,
                        bool double_afk = false) {
  dpp::message msg = embeds::match_result(
      lobby_id, get_game_num(lobby_id), get_player_name(lobby_id, 0),
      get_player_score(lobby_id, 0), get_player_name(lobby_id, 1),
      get_player_score(lobby_id, 1), winner, double_afk);
  creator->direct_message_create(get_player_id(lobby_id, 0), msg);
  creator->direct_message_create(get_player_id(lobby_id, 1), msg);
}

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
    clear_game_timer(player_lobby_id);
    std::string result = determine_winner(player_lobby_id);
    dpp::user winner;
    if (result == "1") {
      winner = get_player_info(player_lobby_id, 0)->player;
      increment_player_score(player_lobby_id, 0);
      send_result_messages(player_lobby_id, 0, 1);
    } else if (result == "2") {
      winner = get_player_info(player_lobby_id, 1)->player;
      increment_player_score(player_lobby_id, 1);
      send_result_messages(player_lobby_id, 1, 0);
    } else if (result == "D") {
      send_result_messages(player_lobby_id, 0, 1, true);
    }

    if (is_game_complete(player_lobby_id)) {
      /* Finish up */
      send_match_results(player_lobby_id, winner);
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
  std::string result = determine_winner(lobby_id);
  dpp::user winner;
  if (result == "1") {
    increment_player_score(lobby_id, 0);
    send_match_results(lobby_id, get_player_info(lobby_id, 0)->player);
  } else if (result == "2") {
    increment_player_score(lobby_id, 1);
    send_match_results(lobby_id, get_player_info(lobby_id, 1)->player);
  } else if (result == "FF") {
    send_match_results(lobby_id, winner, true);
  }

  remove_lobby_from_queue(lobby_id, true);
}

} // namespace game
