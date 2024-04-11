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

#define SPDLOG_FMT_EXTERNAL
#include <cstdlib>
#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <fmt/format.h>
#include <fstream>
#include <getopt.h>
#include <iostream>
#include <spdlog/async.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <sporks/bot.h>
#include <sporks/includes.h>
#include <sporks/modules.h>
#include <sporks/stringops.h>
#include <sys/sysinfo.h>
#include <sys/types.h>

using json = nlohmann::json;

/**
 * Parsed configuration file
 */
json configdocument;

/**
 * Constructor (creates threads, loads all modules)
 */
Bot::Bot(bool development, bool testing, bool intents, dpp::cluster *dppcluster,
         uint32_t cluster_id)
    : dev(development), test(testing), memberintents(intents),
      thr_presence(nullptr), terminate(false), shard_init_count(0),
      core(dppcluster), sent_messages(0), received_messages(0),
      Loader(new ModuleLoader(this)), my_cluster_id(cluster_id) {

  Loader->LoadAll();

  thr_presence = new std::thread(&Bot::UpdatePresenceThread, this);
}

/**
 * Join and delete a thread
 */
void Bot::DisposeThread(std::thread *t) {
  if (t != nullptr) {
    try {
      t->join();
    } catch (const std::exception &e) {
    }
    delete t;
  }
}

/**
 * Destructor
 */
Bot::~Bot() {
  terminate = true;

  DisposeThread(thr_presence);

  delete Loader;
}

/**
 * Returns the named string value from config.json
 */
std::string Bot::GetConfig(const std::string &name) {
  return configdocument[name].get<std::string>();
}

/**
 * Returns true if the bot is running in development mode (different token)
 */
bool Bot::IsDevMode() { return dev; }

/**
 * Returns true if the bot is running in testing mode (live token, ignoring
 * messages except on specific server)
 */
bool Bot::IsTestMode() { return test; }

/**
 * Returns true if the bot has member intents enabled, "GUILD_MEMBERS" which
 * will eventually require discord HQ approval process.
 */
bool Bot::HasMemberIntents() { return memberintents; }

/**
 * On adding a new server, the details of that server are inserted or updated in
 * the shard map. We also make sure settings exist for each channel on the
 * server by calling getSettings() for each channel and throwing the result
 * away, which causes record creation. New users for the guild are pushed into
 * the userqueue which is processed in a separate thread within
 * SaveCachedUsersThread().
 */
void Bot::onServer(const dpp::guild_create_t &gc) {
  FOREACH_MOD(I_OnGuildCreate, OnGuildCreate(gc));
}

/**
 * This runs its own thread that wakes up every 30 seconds (after an initial 2
 * minute warmup). Modules can attach to it for a simple 30 second interval
 * timer via the OnPresenceUpdate() method.
 */
void Bot::UpdatePresenceThread() {
  dpp::utility::set_thread_name("bot/presence_ev");
  std::this_thread::sleep_for(std::chrono::seconds(120));
  while (!this->terminate) {
    FOREACH_MOD(I_OnPresenceUpdate, OnPresenceUpdate());
    std::this_thread::sleep_for(std::chrono::seconds(30));
  }
}

/**
 * Stores a new guild member to the database for use in the dashboard
 */
void Bot::onMember(const dpp::guild_member_add_t &gma) {
  FOREACH_MOD(I_OnGuildMemberAdd, OnGuildMemberAdd(gma));
}

/**
 * Returns the bot's snowflake id
 */
int64_t Bot::getID() { return this->user.id; }

/**
 * Announces that the bot is online. Each shard receives one of the events.
 */
void Bot::onReady(const dpp::ready_t &ready) {
  this->user = core->me;
  FOREACH_MOD(I_OnReady, OnReady(ready));

  /* Event broadcast when all shards are ready */
  shard_init_count++;

  core->log(dpp::ll_debug,
            fmt::format("onReady({}/{})", shard_init_count,
                        core->numshards /
                            (core->maxclusters != 0U ? core->maxclusters : 1)));

  /* Event broadcast when all shards are ready */
  /* BUGFIX: In a clustered environment, the shard max is divided by the number
   * of clusters */
  if (shard_init_count ==
      core->numshards / (core->maxclusters != 0U ? core->maxclusters : 1)) {
    core->log(dpp::ll_debug, fmt::format("OnAllShardsReady()!"));
    FOREACH_MOD(I_OnAllShardsReady, OnAllShardsReady());
  }
}

uint32_t Bot::GetMaxClusters() { return core->maxclusters; }

/**
 * Called on receipt of each message. We do our own cleanup of the message,
 * sanitising any mentions etc from the text before passing it along to modules.
 * The bot's builtin ignore list and a hard coded check against bots/webhooks
 * and itself happen before any module calls, and can't be overridden.
 */
void Bot::onMessage(const dpp::message_create_t &message) {
  if (message.msg.author.id == 0U) {
    core->log(dpp::ll_info, fmt::format("Message dropped, no author: {}",
                                        message.msg.content));
    return;
  }
  /* Ignore self, and bots */
  if (message.msg.author.id != user.id && !message.msg.author.is_bot()) {
    received_messages++;

    /* Replace all mentions with raw nicknames */
    bool mentioned = false;
    std::string mentions_removed = message.msg.content;
    std::vector<std::string> stringmentions;
    for (const auto &mention : message.msg.mentions) {
      stringmentions.push_back(std::to_string(mention.first.id));
      mentions_removed = ReplaceString(
          mentions_removed,
          std::string("<@") + std::to_string(mention.first.id) + ">",
          mention.first.username);
      mentions_removed = ReplaceString(
          mentions_removed,
          std::string("<@!") + std::to_string(mention.first.id) + ">",
          mention.first.username);
      if (mention.first.id == user.id) {
        mentioned = true;
      }
    }

    std::string botusername = this->user.username;

    /* Remove bot's nickname from start of message, if it's there */
    while (mentions_removed.substr(0, botusername.length()) == botusername) {
      mentions_removed = dpp::trim(mentions_removed.substr(
          botusername.length(), mentions_removed.length()));
    }
    /* Remove linefeeds, they mess with botnix */
    mentions_removed = dpp::trim(mentions_removed);

    /* Call modules */
    FOREACH_MOD(I_OnMessage, OnMessage(message, mentions_removed, mentioned,
                                       stringmentions));
  }
}

void Bot::onChannel(const dpp::channel_create_t &channel_create) {
  FOREACH_MOD(I_OnChannelCreate, OnChannelCreate(channel_create));
}

void Bot::onChannelDelete(const dpp::channel_delete_t &cd) {
  FOREACH_MOD(I_OnChannelDelete, OnChannelDelete(cd));
}

void Bot::onServerDelete(const dpp::guild_delete_t &gd) {
  FOREACH_MOD(I_OnGuildDelete, OnGuildDelete(gd));
}

int main(int argc, char **argv) {
  int dev = 0; /* Note: getopt expects ints, this is actually treated as bool */
  int test = 0;
  int members = 0;

  /* Parse command line parameters using getopt() */
  struct option longopts[] = {{"dev", no_argument, &dev, 1},
                              {"test", no_argument, &test, 1},
                              {"members", no_argument, &members, 1},
                              {"clusterid", required_argument, NULL, 'c'},
                              {"maxclusters", required_argument, NULL, 'm'},
                              {0, 0, 0, 0}};

  /* These are our default intents for the bot, basically just receive messages,
   * see reactions to the messages and see who's in our guilds */
  uint32_t intents = dpp::i_default_intents | dpp::i_message_content;

  /* Yes, getopt is ugly, but what you gonna do... */
  int index = 0;
  char arg = 0;
  bool clusters_defined = false;
  uint32_t clusterid = 0;
  uint32_t maxclusters = 1;

  /* opterr is an extern int. Doesn't smell of thread safety to me, bad GNU bad!
   */
  opterr = 0;
  while ((arg = getopt_long_only(argc, argv, "", longopts, &index)) != -1) {
    switch (arg) {
    case 0:
      /* getopt_long_only() set an int variable, just keep going */
      break;
    case 'c':
      /* Cluster id */
      clusterid = from_string<uint32_t>(optarg, std::dec);
      clusters_defined = true;
      break;
    case 'm':
      /* Number of clusters */
      maxclusters = from_string<uint32_t>(optarg, std::dec);
      break;
    case '?':
    default:
      std::cerr << "Unknown parameter '" << argv[optind - 1] << "'\n";
      std::cerr << "Usage: " << argv[0] << " [-dev] [-test] [-members]\n\n";
      std::cerr << "-dev          Run using development token\n";
      std::cerr << "-test:        Run using live token, but eat all outbound "
                   "messages except on test server\n";
      std::cerr << "-members:     Issue a GUILD_MEMBERS intent on shard "
                   "registration\n";
      std::cerr << "-clusterid:   The current cluster id to identify for, must "
                   "be set with -maxclusters\n";
      std::cerr << "-maxclusters: The maximum number of clusters the bot is "
                   "running, must be set with -clusterid\n";
      exit(1);
      break;
    }
  }

  /* This will eventually need approval from discord HQ, so make sure it's a
   * command line parameter we have to explicitly enable */
  if (members != 0) {
    intents |= dpp::i_guild_members;
  }

  if (clusters_defined && maxclusters == 0) {
    std::cerr << "ERROR: You have defined a cluster id with -clusterid but no "
                 "cluster count with -maxclusters.\n";
    exit(2);
  }

  /* Load configuration file */
  std::ifstream configfile("config.json");
  configfile >> configdocument;

  /* Set up spdlog logger */
  std::shared_ptr<spdlog::logger> log;
  spdlog::init_thread_pool(8192, 2);
  std::vector<spdlog::sink_ptr> sinks;
  auto stdout_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto rotating = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      fmt::format("logs/rpsbot{:02d}.log", clusterid), 1024 * 1024 * 5, 10);
  sinks.push_back(stdout_sink);
  sinks.push_back(rotating);
  log = std::make_shared<spdlog::async_logger>(
      "test", sinks.begin(), sinks.end(), spdlog::thread_pool(),
      spdlog::async_overflow_policy::block);
  spdlog::register_logger(log);
  log->set_pattern("%^%Y-%m-%d %H:%M:%S.%e [%L] [th#%t]%$ : %v");
  log->set_level(spdlog::level::level_enum::debug);

  /* Get the correct token from config file for either development or production
   * environment */
  std::string token =
      (dev != 0 ? Bot::GetConfig("devtoken") : Bot::GetConfig("livetoken"));

  /* It's go time! */
  while (true) {
    /* Set cache policy for D++ library
     * --------------------------------
     * User caching:  none
     * Emoji caching: none
     * Role caching:  none (WAS: aggressive)
     */
    dpp::cache_policy_t cp = {dpp::cp_none, dpp::cp_none, dpp::cp_none};
    /* Construct cluster */
    dpp::cluster bot(token, intents,
                     dev != 0 ? 1
                              : from_string<uint32_t>(
                                    Bot::GetConfig("shardcount"), std::dec),
                     clusterid, maxclusters, true, cp);

    /* TODO: Connect to Neo4j database */
    // if (!db::connect(
    //         &bot, Bot::GetConfig("dbhost"), Bot::GetConfig("dbuser"),
    //         Bot::GetConfig("dbpass"), Bot::GetConfig("dbname"),
    //         from_string<uint32_t>(Bot::GetConfig("dbport"), std::dec)))
    //         {
    //   std::cerr << "Database connection failed\n";
    //   exit(2);
    // }

    /* Integrate spdlog logger to D++ log events */
    bot.on_log([&bot, &log](const dpp::log_t &event) {
      switch (event.severity) {
      case dpp::ll_trace:
        log->trace("{}", event.message);
        break;
      case dpp::ll_debug:
        log->debug("{}", event.message);
        break;
      case dpp::ll_info:
        log->info("{}", event.message);
        break;
      case dpp::ll_warning:
        log->warn("{}", event.message);
        break;
      case dpp::ll_error:
        log->error("{}", event.message);
        break;
      case dpp::ll_critical:
      default:
        log->critical("{}", event.message);
        break;
      }
    });

    Bot client(dev != 0, test != 0, members != 0, &bot, clusterid);

    /* Attach events to the Bot class methods */
    bot.on_message_create([ObjectPtr = &client](auto &&PH1) {
      ObjectPtr->onMessage(std::forward<decltype(PH1)>(PH1));
    });
    bot.on_ready([ObjectPtr = &client](auto &&PH1) {
      ObjectPtr->onReady(std::forward<decltype(PH1)>(PH1));
    });
    bot.on_guild_member_add([ObjectPtr = &client](auto &&PH1) {
      ObjectPtr->onMember(std::forward<decltype(PH1)>(PH1));
    });
    bot.on_guild_create([ObjectPtr = &client](auto &&PH1) {
      ObjectPtr->onServer(std::forward<decltype(PH1)>(PH1));
    });
    bot.on_guild_delete([ObjectPtr = &client](auto &&PH1) {
      ObjectPtr->onServerDelete(std::forward<decltype(PH1)>(PH1));
    });

    bot.set_websocket_protocol(dpp::ws_etf);

    try {
      /* Actually connect and start the event loop */
      bot.start(false);
    } catch (std::exception &e) {
      bot.log(dpp::ll_error, fmt::format("Bruh. {}", e.what()));
    }

    /* Reconnection delay to prevent hammering discord */
    ::sleep(30);
  }
}

uint32_t Bot::GetClusterID() { return my_cluster_id; }

void Bot::SetClusterID(uint32_t c) { my_cluster_id = c; }