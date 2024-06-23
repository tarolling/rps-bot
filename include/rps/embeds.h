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

#include <cstdint>
#include <dpp/message.h>
#include <rps/config.h>
#include <rps/lang.h>

using namespace i18n;

constexpr uint16_t AVATAR_SIZE = 1024;

constexpr uint32_t EMBED_COLOR = 0xd5b994;

inline dpp::embed_footer footer() {
  return dpp::embed_footer()
      .set_text("Powered By RPS Bot")
      .set_icon(config::get("icon"));
};

namespace embeds {
[[nodiscard]] dpp::embed queue(const dpp::user &player,
                               const unsigned int player_count);

[[nodiscard]] dpp::embed leave(const dpp::user &player);

[[nodiscard]] dpp::embed
game(const unsigned int lobby_id, const unsigned int game_num,
     const std::string &player_one_name, const unsigned int player_one_score,
     const std::string &player_two_name, const unsigned int player_two_score);

dpp::component game_buttons();

[[nodiscard]] dpp::embed waiting(const unsigned int game_num,
                                 const std::string &player_one_name,
                                 const std::string &player_one_choice,
                                 const std::string &player_two_name,
                                 const std::string &player_two_choice);

[[nodiscard]] dpp::embed game_result(const unsigned int game_num,
                                     const std::string &player_one_name,
                                     const std::string &player_one_choice,
                                     const std::string &player_two_name,
                                     const std::string &player_two_choice,
                                     const std::string &result);

[[nodiscard]] dpp::embed match_results(const unsigned int lobby_id,
                                       const unsigned int game_num,
                                       const std::string &player_one_name,
                                       const unsigned int player_one_score,
                                       const std::string &player_two_name,
                                       const unsigned int player_two_score);

}; // namespace embeds