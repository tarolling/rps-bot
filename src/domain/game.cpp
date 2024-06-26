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

  for (const auto &player_info : found_lobby.players) {
    creator->direct_message_create(
        player_info->player.id,
        embeds::game(player_info->init_interaction, lobby_id, game_num,
                     player_one_name, player_one_score, player_two_name,
                     player_two_score));
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
  unsigned int player_one_score = get_player_score(lobby_id, 0);
  std::string player_two_name = get_player_name(lobby_id, 1);
  std::string player_two_choice = get_player_choice(get_player_id(lobby_id, 1));
  unsigned int player_two_score = get_player_score(lobby_id, 1);
  dpp::slashcommand_t winner_int = get_player_interaction(lobby_id, winner);
  dpp::slashcommand_t loser_int = get_player_interaction(lobby_id, loser);

  dpp::message msg_win = embeds::game_result(
      winner_int, game_num, player_one_name, player_one_choice, player_two_name,
      player_two_choice, "WIN");
  dpp::message msg_loss = embeds::game_result(
      loser_int, game_num, player_one_name, player_one_choice, player_two_name,
      player_two_choice, "LOSS");
  if (draw) {
    msg_win = embeds::game_result(winner_int, game_num, player_one_name,
                                  player_one_choice, player_two_name,
                                  player_two_choice, "DRAW");
    msg_loss = embeds::game_result(loser_int, game_num, player_one_name,
                                   player_one_choice, player_two_name,
                                   player_two_choice, "DRAW");
  }

  /* These need to be sent before the next game message is sent, so we make them
   * synchronous */
  creator->direct_message_create_sync(get_player_id(lobby_id, winner), msg_win);
  creator->direct_message_create_sync(get_player_id(lobby_id, loser), msg_loss);

  /* Hack to set player emoji */
  std::string player_one_emoji_choice =
      (player_one_choice == "Rock")
          ? ":rock:"
          : ((player_one_choice == "Paper") ? ":page_facing_up:"
                                            : ":scissors:");
  std::string player_two_emoji_choice =
      (player_two_choice == "Rock")
          ? ":rock:"
          : ((player_two_choice == "Paper") ? ":page_facing_up:"
                                            : ":scissors:");

  /* Create normal text message for result (may have a higher rate limit?) */
  dpp::message result_msg;
  if (draw) {
    result_msg = dpp::message(fmt::format(
        "__**Lobby #{} - Game {}**__\n{}  {}  {}  |  {}  {}  {}", lobby_id,
        game_num, player_one_name, player_one_emoji_choice, player_one_score,
        player_two_score, player_two_emoji_choice, player_two_name));
  } else {
    /* Determine which name + score to bold */
    if (winner == 0) {
      result_msg = dpp::message(fmt::format(
          "__**Lobby #{} - Game {}**__\n**{}**  {}  **{}**  |  {}  {}  {}",
          lobby_id, game_num, player_one_name, player_one_emoji_choice,
          player_one_score, player_two_score, player_two_emoji_choice,
          player_two_name));
    } else {
      result_msg = dpp::message(fmt::format(
          "__**Lobby #{} - Game {}**__\n{}  {}  {}  |  **{}**  {}  **{}**",
          lobby_id, game_num, player_one_name, player_one_emoji_choice,
          player_one_score, player_two_score, player_two_emoji_choice,
          player_two_name));
    }
  }

  /* Send results in channels that players queued in */
  dpp::slashcommand_t player_one_interaction =
      get_player_interaction(lobby_id, 0);
  dpp::slashcommand_t player_two_interaction =
      get_player_interaction(lobby_id, 1);

  /* Both of them were in DMs, so no work needed */
  if ((player_one_interaction.command.guild_id.empty() ||
       player_one_interaction.command.guild_id == 0) &&
      (player_two_interaction.command.guild_id.empty() ||
       player_two_interaction.command.guild_id == 0)) {
    return;
  }

  /* Just send one if they are the same */
  if (player_one_interaction.command.channel_id ==
      player_two_interaction.command.channel_id) {
    creator->message_create(
        result_msg.set_guild_id(player_one_interaction.command.guild_id)
            .set_channel_id(player_one_interaction.command.channel_id));
    return;
  }

  if (!player_one_interaction.command.guild_id.empty() &&
      player_one_interaction.command.guild_id != 0) {
    creator->message_create(
        result_msg.set_guild_id(player_one_interaction.command.guild_id)
            .set_channel_id(player_one_interaction.command.channel_id));
  }

  if (!player_two_interaction.command.guild_id.empty() &&
      player_two_interaction.command.guild_id != 0) {
    creator->message_create(
        result_msg.set_guild_id(player_two_interaction.command.guild_id)
            .set_channel_id(player_two_interaction.command.message_id));
  }
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
  dpp::slashcommand_t player_one_interaction =
      get_player_interaction(lobby_id, 0);
  dpp::slashcommand_t player_two_interaction =
      get_player_interaction(lobby_id, 1);

  dpp::message player_one_message = embeds::match_result(
      player_one_interaction, lobby_id, get_game_num(lobby_id),
      get_player_name(lobby_id, 0), get_player_score(lobby_id, 0),
      get_player_name(lobby_id, 1), get_player_score(lobby_id, 1), winner,
      double_afk);

  dpp::message player_two_message = embeds::match_result(
      player_two_interaction, lobby_id, get_game_num(lobby_id),
      get_player_name(lobby_id, 0), get_player_score(lobby_id, 0),
      get_player_name(lobby_id, 1), get_player_score(lobby_id, 1), winner,
      double_afk);

  creator->direct_message_create(get_player_id(lobby_id, 0),
                                 player_one_message);
  creator->direct_message_create(get_player_id(lobby_id, 1),
                                 player_two_message);

  /* Send results in channels that players queued in */
  dpp::message msg = embeds::match_result(
      dpp::interaction_create_t(), lobby_id, get_game_num(lobby_id),
      get_player_name(lobby_id, 0), get_player_score(lobby_id, 0),
      get_player_name(lobby_id, 1), get_player_score(lobby_id, 1), winner,
      double_afk);

  /* Both of them were in DMs, so no work needed */
  if ((player_one_interaction.command.guild_id.empty() ||
       player_one_interaction.command.guild_id == 0) &&
      (player_two_interaction.command.guild_id.empty() ||
       player_two_interaction.command.guild_id == 0)) {
    return;
  }

  /* Just send one if they are the same */
  if (player_one_interaction.command.channel_id ==
      player_two_interaction.command.channel_id) {
    creator->message_create(
        msg.set_guild_id(player_one_interaction.command.guild_id)
            .set_channel_id(player_one_interaction.command.channel_id));
    return;
  }

  if (!player_one_interaction.command.guild_id.empty() &&
      player_one_interaction.command.guild_id != 0) {
    creator->message_create(
        msg.set_guild_id(player_one_interaction.command.guild_id)
            .set_channel_id(player_one_interaction.command.channel_id));
    return;
  }

  if (!player_two_interaction.command.guild_id.empty() &&
      player_two_interaction.command.guild_id != 0) {
    creator->message_create(
        msg.set_guild_id(player_two_interaction.command.guild_id)
            .set_channel_id(player_two_interaction.command.message_id));
    return;
  }
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