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
#include <rps/domain/embeds.h>
#include <rps/domain/rps.h>

using namespace i18n;

namespace embeds {
dpp::message queue(const dpp::interaction_create_t &interaction,
                   const dpp::user &player, const unsigned int player_count) {
  std::string type_to_join =
      fmt::format(fmt::runtime(tr("E_TYPE_TO_JOIN", interaction)),
                  tr("c_queue", interaction), tr("c_queue", interaction));

  if (player_count == 1) {
    return dpp::embed()
        .set_title(tr("E_ONE_PLAYER", interaction))
        .set_description(
            fmt::format("**{}** has joined.", player.format_username()))
        .set_thumbnail(player.get_avatar_url(AVATAR_SIZE))
        .add_field(tr("E_WANT_TO_JOIN", interaction), type_to_join)
        .set_footer(footer(interaction))
        .set_color(EMBED_COLOR);
  } else {
    return dpp::embed()
        .set_title(tr("E_TWO_PLAYERS", interaction))
        .set_description(
            fmt::format("**{}** has joined.", player.format_username()))
        .set_thumbnail(player.get_avatar_url(AVATAR_SIZE))
        .set_footer(footer(interaction))
        .set_color(EMBED_COLOR);
  }
}

dpp::message leave(const dpp::interaction_create_t &interaction,
                   const dpp::user &player) {
  return dpp::message().add_embed(
      dpp::embed()
          .set_title(tr("E_ZERO_PLAYERS", interaction))
          .set_description(
              fmt::format("**{}** has left.", player.format_username()))
          .set_thumbnail(player.get_avatar_url(AVATAR_SIZE))
          .set_footer(footer(interaction))
          .set_color(EMBED_COLOR));
}

dpp::message game(const dpp::interaction_create_t &interaction,
                  const unsigned int lobby_id, const unsigned int game_num,
                  const std::string &player_one_name,
                  const unsigned int player_one_score,
                  const std::string &player_two_name,
                  const unsigned int player_two_score) {
  return dpp::message()
      .add_embed(
          dpp::embed()
              .set_title(fmt::format("Lobby #{} - Game {}", lobby_id, game_num))
              /* TODO: Add variable for first to 4 wins */
              .set_description(tr("E_MAKE_SELECTION", interaction))
              .add_field(fmt::format("{}", player_one_score), player_one_name,
                         true)
              .add_field(fmt::format("{}", player_two_score), player_two_name,
                         true)
              .set_footer(footer(interaction))
              .set_color(EMBED_COLOR))
      .add_component(
          dpp::component()
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
              .add_component(
                  dpp::component()
                      .set_type(dpp::component_type::cot_button)
                      .set_label("Scissors")
                      .set_id("Scissors")
                      .set_style(dpp::component_style::cos_primary)));
}

dpp::message waiting(const dpp::interaction_create_t &interaction,
                     const unsigned int game_num,
                     const std::string &player_one_name,
                     const std::string &player_one_choice,
                     const std::string &player_two_name,
                     const std::string &player_two_choice) {
  return dpp::message().add_embed(
      dpp::embed()
          .set_title(tr("E_WAITING", interaction))
          .set_description(fmt::format("Game {}", game_num))
          .add_field(player_one_choice.empty() ? "???" : player_one_choice,
                     player_one_name, true)
          .add_field(player_two_choice.empty() ? "???" : player_two_choice,
                     player_two_name, true)
          .set_footer(footer(interaction))
          .set_color(EMBED_COLOR));
}

dpp::message game_result(const dpp::interaction_create_t &interaction,
                         const unsigned int game_num,
                         const std::string &player_one_name,
                         const std::string &player_one_choice,
                         const std::string &player_two_name,
                         const std::string &player_two_choice,
                         const std::string &result) {
  return dpp::message().add_embed(
      dpp::embed()
          .set_title(fmt::format("GAME {}", result))
          .set_description(fmt::format("Game {}", game_num))
          .add_field(player_one_choice.empty() ? "DNP" : player_one_choice,
                     player_one_name, true)
          .add_field(player_two_choice.empty() ? "DNP" : player_two_choice,
                     player_two_name, true)
          .set_footer(footer(interaction))
          .set_color(EMBED_COLOR));
}

dpp::message match_result(const dpp::interaction_create_t &interaction,
                          const unsigned int lobby_id,
                          const unsigned int game_num,
                          const std::string &player_one_name,
                          const unsigned int player_one_score,
                          const std::string &player_two_name,
                          const unsigned int player_two_score,
                          const dpp::user &winner, bool double_afk) {
  return dpp::message().add_embed(
      dpp::embed()
          .set_title(fmt::format("Lobby #{} Results", lobby_id))
          .set_description(fmt::format("**Games Played:** {}", game_num))
          .add_field(fmt::format("{}", player_one_score), player_one_name, true)
          .add_field(fmt::format("{}", player_two_score), player_two_name, true)
          .set_thumbnail(double_afk ? "" : winner.get_avatar_url(AVATAR_SIZE))
          .set_footer(footer(interaction))
          .set_color(EMBED_COLOR));
}

}; // namespace embeds