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

#include <fmt/format.h>
#include <rps/button_collector.h>
#include <rps/button_int_collector.h>
#include <rps/game.h>

/* Collector will run for 30 seconds */
button_int_collector::button_int_collector(dpp::cluster *cl,
                                           unsigned int lobby_id,
                                           dpp::message p_one_msg,
                                           dpp::message p_two_msg)
    : button_collector(cl, GAME_TIMEOUT, lobby_id, p_one_msg, p_two_msg) {}

button_int_collector::~button_int_collector() {}

/* Override the "completed" event and then output the number of collected
 * button clicks as a message. */
void button_int_collector::completed(
    const std::vector<collected_button_click> &list) {
  std::string result = game::determine_winner(lobby_id);
  if (result == "1") {
    game::increment_player_score(lobby_id, 1);
    game::send_result_messages(lobby_id, 1, 2);
  } else if (result == "2") {
    game::increment_player_score(lobby_id, 2);
    game::send_result_messages(lobby_id, 2, 1);
  } else {
    game::send_result_messages(lobby_id, 1, 2, true);
  }

  if (game::is_game_complete(lobby_id)) {
    /* Finish up */
    game::remove_lobby_from_queue(lobby_id, true);
  } else {
    /* Send message */
    game::increment_game_num(lobby_id);
    game::reset_choices(lobby_id);
    game::send_game_messages(lobby_id);
  }
}

const collected_button_click *
button_int_collector::filter(const dpp::button_click_t &element) {
  /* Capture reactions for given message ID only */
  if (player_one_message.id.empty() || player_two_message.id.empty() ||
      element.command.message_id == player_one_message.id ||
      element.command.message_id == player_two_message.id) {
    game::set_player_choice(element.command.get_issuing_user().id,
                            element.custom_id);
    /* Edit original message */
    // owner->message_edit(const struct message &m);
    element.from->creator->direct_message_create_sync(
        element.command.get_issuing_user().id,
        dpp::message(fmt::format("You selected {}! Waiting for opponent...",
                                 element.custom_id)));

    if (game::check_both_responses(lobby_id)) {
      std::string result = game::determine_winner(lobby_id);
      if (result == "1") {
        game::increment_player_score(lobby_id, 1);
        game::send_result_messages(lobby_id, 1, 2);
      } else if (result == "2") {
        game::increment_player_score(lobby_id, 2);
        game::send_result_messages(lobby_id, 2, 1);
      } else {
        game::send_result_messages(lobby_id, 1, 2, true);
      }

      if (game::is_game_complete(lobby_id)) {
        /* Finish up */
        game::remove_lobby_from_queue(lobby_id, true);
      } else {
        /* Send message */
        game::increment_game_num(lobby_id);
        game::reset_choices(lobby_id);
        game::send_game_messages(lobby_id);
      }
      cancel();
    }
    return &click;
  } else {
    return nullptr;
  }
}