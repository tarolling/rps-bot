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

#include "game_controller.h"
#include "rps.h"
#include <coroutine>
#include <dpp/coro/coro.h>
#include <dpp/coro/task.h>
#include <dpp/coro/when_any.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/misc-enum.h>
#include <dpp/restresults.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <functional>
#include <sporks/bot.h>
#include <thread>

game_controller::game_controller() : creator(nullptr){};
game_controller::game_controller(RPSModule *_creator) : creator(_creator) {
  creator->GetBot()->core->log(
      dpp::ll_debug, fmt::format("game_controller::game_controller()"));
};

game_controller::~game_controller() {
  /* XXX: These are safety values, so that if we access a deleted state at any
   * point, it crashes sooner and can be identified easily in the debugger */
  creator = nullptr;
}

auto play_game(RPSModule &creator, const dpp::user &player) -> dpp::task<void> {
  dpp::message msg;

  {
    std::lock_guard<std::mutex> states_lock(creator.states_mutex);
    msg = fmt::format("Lobby #{} - Game {}", creator.state.global_game_id, 1);
  }

  msg.add_component(
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
          .add_component(dpp::component()
                             .set_type(dpp::component_type::cot_button)
                             .set_label("Scissors")
                             .set_id("Scissors")
                             .set_style(dpp::component_style::cos_primary)));

  dpp::message sent_message =
      creator.GetBot()->core->direct_message_create_sync(player.id, msg);

  auto result = co_await dpp::when_any(
      creator.GetBot()->core->on_button_click.when(
          [&](const dpp::button_click_t &event) {
            return event.command.get_issuing_user().id == player.id;
          }),
      creator.GetBot()->core->co_sleep(60));

  if (result.index() ==
      0) { // Awaitable #0 completed first, that is the button click event
    const dpp::button_click_t &click_event = result.get<0>();
    creator.GetBot()->core->log(dpp::ll_debug, "bro answered");

    creator.GetBot()->core->direct_message_create_sync(
        player.id, dpp::message{"you clicked " + click_event.custom_id});
  } else { // Here index() is 1, the timer expired
    creator.GetBot()->core->log(dpp::ll_debug, "timer expired");
    creator.GetBot()->core->direct_message_create_sync(
        player.id, dpp::message{"times up!"});
  }

  // /* Listen for button clicks on the sent message */
  // creator.GetBot()->core->on_interaction_create(
  //     [=](const dpp::interaction_create_t &event) -> dpp::task<void> {
  //       auto result = co_await dpp::when_any(
  //           event.from->creator->on_button_click.when(
  //               [&event](const dpp::button_click_t &b) {
  //                 return b.custom_id == event.command.id.str();
  //               }),
  //           event.from->creator->co_sleep(60));

  //       if (result.index() ==
  //           0) { // Awaitable #0 completed first, that is the button
  //           click event
  //         // Acknowledge the click and edit the original response,
  //         removing the
  //         // button
  //         const dpp::button_click_t &click_event = result.get<0>();
  //         click_event.reply();
  //         event.edit_original_response(dpp::message{
  //             "You clicked the button with the id " +
  //             click_event.custom_id});
  //       } else { // Here index() is 1, the timer expired
  //         event.edit_original_response(dpp::message{"I haven't got all
  //         day!"});
  //       }
  //     });
}

void game_controller::play_series(const struct rps_game &game) {
  if (game.players.size() != 2) {
    creator->GetBot()->core->log(
        dpp::ll_error, fmt::format("ERROR - Player vector size not 2"));
    return;
  }

  std::thread p1(play_game, std::ref(*creator), std::ref(game.players.front()));
  std::thread p2(play_game, std::ref(*creator), std::ref(game.players.back()));

  p1.join();
  p2.join();
}
