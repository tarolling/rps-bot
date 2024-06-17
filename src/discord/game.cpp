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

#include <condition_variable>
#include <dpp/exception.h>
#include <dpp/misc-enum.h>
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

unsigned int find_player_lobby_id(const dpp::user &player) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);

  if (lobby_queue.empty()) {
    creator->log(dpp::ll_debug, "No lobbies available");
    return 0;
  }

  for (const auto &lobby : lobby_queue) {
    for (const auto &player_info : lobby.players) {
      if (player_info->player.id == player.id) {
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
 * @brief Doesn't require a mutex, since it will solely be called from other
 * methods that do acquire locks
 */
void increment_global_lobby_id() { ++global_lobby_id; }

/**
 * @brief Doesn't require a mutex, since it will solely be called from other
 * methods that do acquire locks
 */
void decrement_global_lobby_id() { --global_lobby_id; }

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
  increment_global_lobby_id();
  rps_lobby lobby;
  lobby.id = get_global_lobby_id();
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  lobby.game_number = 0;
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
                              const dpp::user &player) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &it : lobby_queue) {
    if (it.id == lobby_id) {
      it.players.remove_if(
          [=](const std::shared_ptr<player_info> &player_info) {
            return player_info->player.id == player.id;
          });
    }
  }
}

void set_player_choice(const dpp::user &player, const std::string &choice) {
  unsigned int player_lobby_id = find_player_lobby_id(player);
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);

  if (player_lobby_id == 0) {
    creator->log(
        dpp::ll_critical,
        fmt::format("set_player_choice: Unable to find player lobby for {}",
                    player.format_username()));
    return;
  }

  for (auto &lobby : lobby_queue) {
    for (auto &player_info : lobby.players) {
      if (player_info->player.id == player.id) {
        player_info->choice = choice;
      }
    }
  }
}

std::string get_player_choice(const dpp::user &player) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    for (auto &player_info : lobby.players) {
      if (player_info->player.id == player.id) {
        return player_info->choice;
      }
    }
  }
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
                              const dpp::user &player) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      for (const auto &player_info : lobby.players) {
        if (player_info->player.id == player.id) {
          return player_info->score;
        }
      }
    }
  }
}

void increment_player_score(const unsigned int lobby_id,
                            const dpp::user &player) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    if (lobby.id == lobby_id) {
      for (auto &player_info : lobby.players) {
        if (player_info->player.id == player.id) {
          player_info->score++;
          return;
        }
      }
    }
  }
}

void notify_player_cv(const dpp::user &player) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (auto &lobby : lobby_queue) {
    for (auto &player_info : lobby.players) {
      if (player_info->player.id == player.id) {
        player_info->cv.notify_all();
        return;
      }
    }
  }
}

std::shared_ptr<player_info> get_player_info(const dpp::user &player) {
  std::lock_guard<std::shared_mutex> game_lock(game_mutex);
  for (const auto &lobby : lobby_queue) {
    for (const auto &player_info : lobby.players) {
      if (player_info->player.id == player.id) {
        return player_info;
      }
    }
  }
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
} // namespace game