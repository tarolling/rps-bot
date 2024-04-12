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
#include "time.h"
#include <cstdint>
#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <fmt/format.h>
#include <fstream>
#include <iostream>
#include <sporks/modules.h>
#include <sporks/statusfield.h>
#include <sporks/stringops.h>
#include <streambuf>
#include <string>
#include <unistd.h>

using json = nlohmann::json;

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

RPSModule::~RPSModule() {
  /* We don't just delete threads, they must go through Bot::DisposeThread which
   * joins them first */
  DisposeThread(presence_update);

  /* This explicitly calls the destructor on all states */
  std::lock_guard<std::mutex> state_lock(states_mutex);
  // states.clear();
}

bool RPSModule::OnPresenceUpdate() {
  /* Note: Only updates this cluster's shards! */
  const dpp::shard_list &shards = bot->core->get_shards();
  for (auto i = shards.begin(); i != shards.end(); ++i) {
    dpp::discord_client *shard = i->second;
    uint64_t uptime = shard->get_uptime().secs +
                      (shard->get_uptime().mins * 60) +
                      (shard->get_uptime().hours * 60 * 60) +
                      (shard->get_uptime().days * 60 * 60 * 24);
  }
  return true;
}

bool RPSModule::OnGuildCreate(const dpp::guild_create_t &guild) {
  if (guild.created->is_unavailable()) {
    bot->core->log(dpp::ll_error, "Guild ID " +
                                      std::to_string(guild.created->id) +
                                      " is unavailable due to an outage!");
  }
  return true;
}

bool RPSModule::OnAllShardsReady() {
  /* Called when the framework indicates all shards are connected */
  char hostname[1024];
  hostname[1023] = '\0';
  gethostname(hostname, 1023);

  this->booted = true;

  if (bot->IsTestMode()) {
    /* Don't resume games in test mode */
    bot->core->log(dpp::ll_debug,
                   fmt::format("Not resuming games in test mode"));
    return true;
  } else {
    bot->core->log(dpp::ll_debug, "Resuming games...");
  }

  return true;
}

bool RPSModule::OnChannelDelete(const dpp::channel_delete_t &cd) {
  return true;
}

bool RPSModule::OnGuildDelete(const dpp::guild_delete_t &gd) {
  /* Unavailable guilds means an outage. We don't remove them if it's just an
   * outage */
  if (!gd.deleted.is_unavailable()) {
    {
      std::unique_lock locker(settingcache_mutex);
      settings_cache.erase(gd.deleted.id);
    }
    bot->core->log(dpp::ll_info,
                   fmt::format("Kicked from guild id {}", gd.deleted.id));
  } else {
    bot->core->log(dpp::ll_info,
                   fmt::format("Outage on guild id {}", gd.deleted.id));
  }
  return true;
}

void RPSModule::eraseCache(dpp::snowflake guild_id) {
  std::unique_lock locker(settingcache_mutex);
  auto i = settings_cache.find(guild_id);
  if (i != settings_cache.end()) {
    settings_cache.erase(i);
  }
}

std::string RPSModule::GetDescription() { return "RPS System"; }

void RPSModule::UpdatePresenceLine() {
  uint32_t ticks = 0;
  while (!terminating) {
    try {
      ticks++;
      if (ticks > 100) {
        ticks = 0;
      }
      std::string presence =
          fmt::format("Is RPS all luck, or pure skill? "
                      "Running on {} shards, cluster {}",
                      Comma(bot->core->numshards), bot->GetClusterID());
      bot->core->log(dpp::ll_debug, fmt::format("PRESENCE: {}", presence));
      /* Can't translate this, it's per-shard! */
      bot->core->set_presence(
          dpp::presence(dpp::ps_online, dpp::at_game, presence));
    } catch (std::exception &e) {
      bot->core->log(
          dpp::ll_error,
          fmt::format("Exception in UpdatePresenceLine: {}", e.what()));
    }
    sleep(120);
  }
  bot->core->log(dpp::ll_debug, fmt::format("Presence thread exited."));
}

// void RPSModule::show_stats(const std::string &interaction_token,
//                            dpp::snowflake command_id, dpp::snowflake
//                            guild_id, dpp::snowflake channel_id) {
//   db::resultset topten =
//       db::query("SELECT dayscore, name, emojis, trivia_user_cache.* FROM "
//                 "scores LEFT JOIN trivia_user_cache ON snowflake_id = name "
//                 "LEFT JOIN vw_emojis ON name = user_id WHERE guild_id = ? and
//                 " "dayscore > 0 ORDER BY dayscore DESC limit 10",
//                 {guild_id});
//   size_t count = 1;
//   std::string msg;
//   for (auto &r : topten) {
//     if (!r["username"].empty()) {
//       msg.append(
//           fmt::format("{0}. `{1}#{2:04d}` ({3}) {4}\n", count++,
//           r["username"],
//                       from_string<uint32_t>(r["discriminator"], std::dec),
//                       r["dayscore"], r["emojis"]));
//     } else {
//       msg.append(fmt::format("{}. <@{}> ({})\n", count++, r["snowflake_id"],
//                              r["dayscore"]));
//     }
//   }
//   if (msg.empty()) {
//     msg = "Nobody has played here today! :cry:";
//   }
//   guild_settings_t settings = GetGuildSettings(guild_id);
//   if (settings.premium && !settings.custom_url.empty()) {
//     EmbedWithFields(
//         interaction_token, command_id, settings, _("LEADERBOARD", settings),
//         {{_("TOP_TEN", settings), msg, false},
//          {_("MORE_INFO", settings),
//           fmt::format(_("LEADER_LINK", settings), settings.custom_url),
//           false}},
//         channel_id);
//   } else {
//     EmbedWithFields(
//         interaction_token, command_id, settings, _("LEADERBOARD", settings),
//         {{_("TOP_TEN", settings), msg, false},
//          {_("MORE_INFO", settings),
//           fmt::format(_("LEADER_LINK", settings), guild_id), false}},
//         channel_id);
//   }
// }

void RPSModule::Tick() {
  while (!terminating) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    try {
      std::lock_guard<std::mutex> states_lock(states_mutex);
      std::vector<uint64_t> expired;
      time_t now = time(NULL);

      // for (auto &s : states) {
      //   if (now >= s.second.next_tick) {
      //     bot->core->log(
      //         dpp::ll_trace,
      //         fmt::format("Ticking state id {} (now={}, next_tick={})",
      //         s.first,
      //                     now, s.second.next_tick));
      //     s.second.tick();
      //     if (s.second.terminating) {
      //       uint64_t e = s.first;
      //       expired.push_back(e);
      //     }
      //   }
      // }

      for (auto e : expired) {
        bot->core->log(dpp::ll_debug,
                       fmt::format("Terminating state id {}", e));
        // states.erase(e);
      }
    } catch (const std::exception &e) {
      bot->core->log(
          dpp::ll_warning,
          fmt::format("Uncaught std::exception in RPSModule::Tick(): {}",
                      e.what()));
    }
  }
}

void RPSModule::DisposeThread(std::thread *t) { bot->DisposeThread(t); }

bool RPSModule::OnMessage(const dpp::message_create_t &message,
                          const std::string &clean_message, bool mentioned,
                          const std::vector<std::string> &stringmentions) {
  return RealOnMessage(message, clean_message, mentioned, stringmentions, 0);
}

bool RPSModule::RealOnMessage(const dpp::message_create_t &message,
                              const std::string &clean_message, bool mentioned,
                              const std::vector<std::string> &stringmentions,
                              dpp::snowflake _author_id) {
  std::string username;
  dpp::message msg = message.msg;
  bool is_from_dashboard = (_author_id != 0);
  double start = time_f();

  // Allow overriding of author id from remote start code
  uint64_t author_id = _author_id ? _author_id : msg.author.id;

  bool isbot = msg.author.is_bot();
  username = message.msg.author.username;
  if (isbot) {
    /* Drop bots here */
    return true;
  }

  dpp::snowflake guild_id = message.msg.guild_id;
  dpp::snowflake channel_id = message.msg.channel_id;
  dpp::guild_member gm = message.msg.member;

  // if (msg.channel_id.empty()) {
  //   /* No channel! */
  //   bot->core->log(
  //       dpp::ll_debug,
  //       fmt::format("Message without channel, M:{} A:{}", msg.id,
  //       author_id));
  // } else {

  //   if (mentioned && prefix_match->Match(clean_message)) {
  //     guild_settings_t settings = GetGuildSettings(guild_id);
  //     bot->core->message_create(dpp::message(
  //         channel_id, fmt::format(_("PREFIX", settings), settings.prefix,
  //                                 settings.prefix)));
  //     bot->core->log(
  //         dpp::ll_debug,
  //         fmt::format("Respond to prefix request on channel C:{} A:{}",
  //                     channel_id, author_id));
  //   } else {

  //     guild_settings_t settings = GetGuildSettings(guild_id);

  //     // Commands
  //     if (lowercase(clean_message.substr(0, settings.prefix.length())) ==
  //         lowercase(settings.prefix)) {
  //       std::string command = clean_message.substr(
  //           settings.prefix.length(),
  //           clean_message.length() - settings.prefix.length());
  //       queue_command(command, author_id, channel_id, guild_id, mentioned,
  //                     username, is_from_dashboard, message.msg.author, gm);
  //       bot->core->log(dpp::ll_info,
  //                      fmt::format("CMD (USER={}, GUILD={}): <{}> {}",
  //                                  author_id, guild_id, username,
  //                                  clean_message));
  //     }

  //     // Answers for active games
  //     {
  //       std::lock_guard<std::mutex> states_lock(states_mutex);
  //       state_t *state = GetState(channel_id);
  //       if (state) {
  //         /* The state_t class handles potential answers, but only when a
  //         game
  //          * is running on this guild */
  //         state->queue_message(settings, clean_message, author_id, username,
  //                              mentioned, message.msg.author, gm);
  //         bot->core->log(
  //             dpp::ll_debug,
  //             fmt::format(
  //                 "Processed potential answer message from A:{} on C:{}",
  //                 author_id, channel_id));
  //       }
  //     }
  //   }
  // }

  double end = time_f();
  double time_taken = end - start;

  if (bot->IsDevMode()) {
    bot->core->log(
        dpp::ll_debug,
        fmt::format("Message processing took {:.7f} seconds, channel: {}",
                    time_taken, msg.channel_id));
  } else {
    if (time_taken > 0.1) {
      bot->core->log(
          dpp::ll_warning,
          fmt::format("Message processing took {:.7f} seconds!!! Channel: {}",
                      time_taken, msg.channel_id));
    }
  }

  return true;
}

// state_t *RPSModule::GetState(dpp::snowflake channel_id) {
//   auto state_iter = states.find(channel_id);
//   if (state_iter != states.end()) {
//     return &state_iter->second;
//   }
//   return nullptr;
// }

ENTRYPOINT(RPSModule);