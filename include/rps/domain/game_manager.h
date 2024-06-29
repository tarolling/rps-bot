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

#include <dpp/dispatcher.h>

constexpr unsigned int GAME_TIMEOUT = 30;

namespace game_manager {
/**
 * @brief Initialize global game state
 *
 * @param bot creating D++ cluster
 */
void init(dpp::cluster &bot);

void send_game_messages(const unsigned int lobby_id);
void send_result_messages(const unsigned int lobby_id,
                          const unsigned int winner, const unsigned int loser,
                          bool draw = false);
void send_match_results(const unsigned int lobby_id, const dpp::user &winner,
                        bool double_afk = false);
void handle_choice(const dpp::button_click_t &event);
void handle_timeout(const unsigned int lobby_id);
void handle_leave(const dpp::slashcommand_t &event,
                  const unsigned int lobby_id);
} // namespace game_manager