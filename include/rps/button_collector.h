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

#include <dpp/appcommand.h>
#include <dpp/collector.h>
#include <dpp/dispatcher.h>
#include <dpp/dpp.h>
#include <dpp/managed.h>
#include <dpp/message.h>
#include <dpp/misc-enum.h>

class collected_button_click : public dpp::managed {
public:
  /**
   * @brief Reacting user.
   */
  dpp::user react_user;

  /**
   * @brief Reacting channel.
   */
  dpp::channel react_channel{};

  /**
   * @brief Reacted button.
   */
  dpp::component_interaction react_button;
};

using button_collector_t =
    dpp::collector<dpp::button_click_t, collected_button_click>;

class button_collector : public button_collector_t {
protected:
  /**
   * @brief The lobby ID that this collector is collecting interactions for.
   */
  unsigned int lobby_id;

  /**
   * @brief Player one's message.
   */
  dpp::message player_one_message;

  /**
   * @brief Player two's message.
   */
  dpp::message player_two_message;

  /**
   * @brief The button click.
   */
  collected_button_click click;

public:
  /**
   * @brief Construct a new reaction collector object
   *
   * @param cl cluster to associate the collector with
   * @param duration Duration of time to run the collector for in seconds
   * @param player_one_msg_id Player one's message ID. Only collects reactions
   * for the given message
   * @param player_one_msg_id Player two's message ID. Only collects reactions
   * for the given message
   */
  button_collector(dpp::cluster *cl, uint64_t duration, unsigned int lobby_id,
                   dpp::message &player_one_msg, dpp::message &player_two_msg)
      : button_collector_t::collector(cl, duration, cl->on_button_click),
        lobby_id(lobby_id), player_one_message(player_one_msg),
        player_two_message(player_two_msg) {}

  /**
   * @brief Return the completed collection
   *
   * @param list items collected during the timeframe specified
   */
  virtual void completed(const std::vector<collected_button_click> &list) = 0;

  /**
   * @brief Select and filter the items which are to appear in the list
   * This is called every time a new event is fired, to filter the event and
   * determine which of the items is sent to the list. Returning nullptr
   * excludes the item from the list.
   *
   * @param element element to filter
   * @return Returned item to add to the list, or nullptr to skip adding this
   * element
   */
  virtual const collected_button_click *
  filter(const dpp::button_click_t &element) {}

  /**
   * @brief Destroy the reaction collector object
   */
  virtual ~button_collector() = default;
};