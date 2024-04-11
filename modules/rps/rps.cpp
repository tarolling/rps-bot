/************************************************************************************
 *
 * Copyright 2004 Craig Edwards <support@brainbox.cc>
 *
 * Core based on Sporks, the Learning Discord Bot, Craig Edwards (c) 2019.
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

#include "rps.h"
#include <dpp/dpp.h>
#include <fmt/format.h>
#include <sporks/modules.h>
#include <sporks/stringops.h>
#include <string>
#include <unistd.h>

RPSModule::RPSModule(Bot *instigator, ModuleLoader *ml)
    : Module(instigator, ml), terminating(false), booted(false) {
  /* Attach D++ events to module */
  ml->Attach({I_OnMessage, I_OnPresenceUpdate, I_OnChannelDelete,
              I_OnGuildDelete, I_OnAllShardsReady, I_OnGuildCreate},
             this);

  /* Create threads */
  presence_update = new std::thread(&RPSModule::UpdatePresenceLine, this);

  /* Setup built in commands */
  SetupCommands();
}

void RPSModule::ProcessCommands() {}

Bot *RPSModule::GetBot() { return bot; }

bool RPSModule::OnGuildCreate(const dpp::guild_create_t &guild) {
  if (guild.created->is_unavailable()) {
    bot->core->log(dpp::ll_error, "Guild ID " +
                                      std::to_string(guild.created->id) +
                                      " is unavailable due to an outage!");
  }
  return true;
}