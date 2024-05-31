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

#include "state.h"

class game_controller {
  class RPSModule *creator;

public:
  game_controller();
  game_controller(RPSModule *_creator);
  ~game_controller();
  game_controller(const game_controller &) = default; // copy constructor
  game_controller(game_controller &&other) noexcept =
      default; // move constructor
  game_controller &
  operator=(const game_controller &other) = default; // copy assignment
  game_controller &
  operator=(game_controller &&other) noexcept = default; // move assignment
  void play_series(const struct rps_game &game);
};