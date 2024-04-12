/************************************************************************************
 *
 * Copyright 2019 Craig Edwards <support@sporks.gg>
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

#include <cstdint>
#include <sporks/bot.h>
#include <sporks/modules.h>
#include <sporks/stringops.h>
#include <string>

class PresenceModule : public Module {
  int64_t halfminutes{};

public:
  PresenceModule(Bot *instigator, ModuleLoader *ml) : Module(instigator, ml) {
    ml->Attach({I_OnPresenceUpdate}, this);
  }

  virtual ~PresenceModule() {}

  std::string GetDescription() override {
    return "Updates presence and stats counters";
  }

  bool OnPresenceUpdate() override {
    int64_t users = 0;
    int64_t channel_count = 0;
    int64_t servers = 0;
    for (const auto &s : bot->core->get_shards()) {
      users += (int64_t)s.second->get_member_count();
      channel_count += (int64_t)s.second->get_channel_count();
      servers += (int64_t)s.second->get_guild_count();
    }
    if (++halfminutes > 20) {
      /* Reset counters every 10 mins. Chewey stats uses these counters and
       * expects this */
      halfminutes = bot->sent_messages = bot->received_messages = 0;
    }
    return true;
  }
};

ENTRYPOINT(PresenceModule);