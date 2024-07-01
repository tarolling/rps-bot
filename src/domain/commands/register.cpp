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

#include <dpp/appcommand.h>
#include <rps/data_source/database.h>
#include <rps/domain/commands/register.h>
#include <rps/domain/embeds.h>
#include <rps/domain/game.h>
#include <rps/domain/game_manager.h>

using namespace i18n;

dpp::slashcommand register_command_t::register_command(dpp::cluster &bot) {
  return tr(dpp::slashcommand("c_register", "d_register", bot.me.id)
                .set_dm_permission(true));
}

void register_command_t::route(const dpp::slashcommand_t &event) {
  /* Give database connection some time */
  /* Surely it will respond within 15 minutes */
  event.thinking(true);

  if (db::test_register_query(event.command.get_issuing_user().id)) {
    event.edit_original_response(embeds::_register(event));
  } else {
    event.edit_original_response(
        dpp::message("Registration failed. You must already be registered..."));
  }
}