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

#include <dpp/message.h>
#include <dpp/snowflake.h>
#include <dpp/user.h>
#include <fmt/format.h>
#include <rps/embeds.h>

using namespace i18n;

namespace embeds {
dpp::embed queue(const dpp::user &player, const unsigned int player_count) {
  std::string adjustment = (player_count == 1) ? " is" : "s are";
  return dpp::embed()
      .set_title(
          fmt::format("{} player{} in the queue", player_count, adjustment))
      .add_field("Want to join?",
                 "Type `/queue` or `!queue` to join this lobby!")
      .set_description(
          fmt::format("**{}** has joined.", player.format_username()))
      .set_thumbnail(player.get_avatar_url(AVATAR_SIZE))
      .set_footer(footer())
      .set_color(EMBED_COLOR);
}

dpp::embed leave(const dpp::user &player) {
  return dpp::embed()
      .set_title("0 players are in the queue")
      .set_description(
          fmt::format("**{}** has left.", player.format_username()))
      .set_thumbnail(player.get_avatar_url(AVATAR_SIZE))
      .set_footer(footer())
      .set_color(EMBED_COLOR);
}

dpp::embed game(const unsigned int lobby_id, const unsigned int game_num,
                const std::string &player_one_name,
                const unsigned int player_one_score,
                const std::string &player_two_name,
                const unsigned int player_two_score) {
  return dpp::embed()
      .set_title(fmt::format("Lobby #{} - Game {}", lobby_id, game_num))
      /* TODO: Add variable for first to 4 wins */
      .set_description(
          "Make your selection. You have 30 seconds! First to 4 wins.")
      .add_field(fmt::format("{}", player_one_score), player_one_name, true)
      .add_field(fmt::format("{}", player_two_score), player_two_name, true)
      .set_footer(footer())
      .set_color(EMBED_COLOR);
}

dpp::component game_buttons() {
  return dpp::component()
      .add_component(dpp::component()
                         .set_type(dpp::component_type::cot_button)
                         .set_label("Rock")
                         .set_id("Rock")
                         .set_style(dpp::component_style::cos_primary))
      .add_component(dpp::component()
                         .set_type(dpp::component_type::cot_button)
                         .set_label("Paper")
                         .set_id("Paper")
                         .set_style(dpp::component_style::cos_primary))
      .add_component(dpp::component()
                         .set_type(dpp::component_type::cot_button)
                         .set_label("Scissors")
                         .set_id("Scissors")
                         .set_style(dpp::component_style::cos_primary));
}

dpp::embed waiting(const unsigned int game_num,
                   const std::string &player_one_name,
                   const std::string &player_one_choice,
                   const std::string &player_two_name,
                   const std::string &player_two_choice) {
  return dpp::embed()
      .set_title("Waiting for opponent...")
      .set_description(fmt::format("Game {}", game_num))
      .add_field(player_one_choice.empty() ? "???" : player_one_choice,
                 player_one_name, true)
      .add_field(player_two_choice.empty() ? "???" : player_two_choice,
                 player_two_name, true)
      .set_footer(footer())
      .set_color(EMBED_COLOR);
}

dpp::embed game_result(const unsigned int game_num,
                       const std::string &player_one_name,
                       const std::string &player_one_choice,
                       const std::string &player_two_name,
                       const std::string &player_two_choice,
                       const std::string &result) {
  return dpp::embed()
      .set_title(fmt::format("GAME {}", result))
      .set_description(fmt::format("Game {}", game_num))
      .add_field(player_one_choice.empty() ? "DNP" : player_one_choice,
                 player_one_name, true)
      .add_field(player_two_choice.empty() ? "DNP" : player_two_choice,
                 player_two_name, true)
      .set_footer(footer())
      .set_color(EMBED_COLOR);
}

dpp::embed match_results(const unsigned int lobby_id,
                         const unsigned int game_num,
                         const std::string &player_one_name,
                         const unsigned int player_one_score,
                         const std::string &player_two_name,
                         const unsigned int player_two_score) {
  return dpp::embed()
      .set_title(fmt::format("Lobby #{} Results", lobby_id))
      .set_description(fmt::format("**Games Played:** {}", game_num))
      .set_footer(footer())
      .set_color(EMBED_COLOR);
}

}; // namespace embeds