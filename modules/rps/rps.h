#pragma once

#include "commands.h"
#include "game_controller.h"
#include "settings.h"
#include "state.h"
#include <deque>
#include <dpp/dpp.h>
#include <mutex>
#include <shared_mutex>
#include <sporks/modules.h>
#include <string>
#include <vector>

// Number of seconds after which a game is considered hung and its thread exits.
// This can happen if a game gets lost in a discord gateway outage (!)
constexpr int32_t game_reap_secs = 20000;

// Number of seconds between allowed API-bound calls, per channel
constexpr int8_t per_channel_rate_limit = 4;

struct field_t {
  std::string name;
  std::string value;
  bool _inline;
};

class RPSModule : public Module {
  std::unordered_map<dpp::snowflake, time_t> limits;
  std::unordered_map<dpp::snowflake, time_t> last_rl_warning;
  std::thread *presence_update;
  bool terminating;
  std::shared_mutex cmds_mutex;
  std::shared_mutex cmdmutex;
  std::deque<in_cmd> commandqueue;
  std::deque<in_cmd> to_process;
  std::thread *command_processor;
  std::shared_mutex lang_mutex;
  command_list_t commands;
  std::shared_mutex settingcache_mutex;
  std::unordered_map<dpp::snowflake, guild_settings_t> settings_cache;

  bool booted;
  void thinking(bool ephemeral, const dpp::interaction_create_t &event);
  void eraseCache(dpp::snowflake guild_id);
  bool has_rl_warn(dpp::snowflake channel_id);
  bool has_limit(dpp::snowflake channel_id);
  bool set_rl_warn(dpp::snowflake channel_id);
  bool set_limit(dpp::snowflake channel_id);

public:
  time_t startup;
  json *lang;
  std::mutex states_mutex;
  state_t state;
  game_controller controller;
  RPSModule(Bot *instigator, ModuleLoader *ml);
  Bot *GetBot();
  virtual ~RPSModule();
  RPSModule(const RPSModule &other) = delete;                // copy constructor
  RPSModule(RPSModule &&other) noexcept = delete;            // move constructor
  RPSModule &operator=(const RPSModule &other) = delete;     // copy assignment
  RPSModule &operator=(RPSModule &&other) noexcept = delete; // move assignment
  void SetupCommands();
  void HandleInteraction(const dpp::interaction_create_t &event);
  void HandleSelect(const dpp::select_click_t &event);
  void HandleButton(const dpp::button_click_t &event);
  void handle_command(const in_cmd &cmd,
                      const dpp::interaction_create_t &event);
  void ProcessCommands();
  bool OnPresenceUpdate() override;
  std::string _(const std::string &k);
  bool OnAllShardsReady() override;
  bool OnChannelDelete(const dpp::channel_delete_t &cd) override;
  bool OnGuildDelete(const dpp::guild_delete_t &gd) override;

  /* Returns a local count */
  uint64_t GetActiveLocalGames();

  /* These return a sum across all clusters using the database */
  uint64_t GetActiveGames();
  uint64_t GetGuildTotal();
  uint64_t GetMemberTotal();
  uint64_t GetChannelTotal();

  static std::string escape_json(const std::string &s);

  void ProcessEmbed(const class guild_settings_t &settings,
                    const std::string &embed_json, dpp::snowflake channelID);
  void SimpleEmbed(const class guild_settings_t &settings,
                   const std::string &emoji, const std::string &text,
                   dpp::snowflake channelID, const std::string &title = "",
                   const std::string &image = "",
                   const std::string &thumbnail = "");
  void EmbedWithFields(const class guild_settings_t &settings,
                       const std::string &title, std::vector<field_t> fields,
                       dpp::snowflake channelID, const std::string &url = "",
                       const std::string &image = "",
                       const std::string &thumbnail = "",
                       const std::string &description = "");

  void ProcessEmbed(const std::string &interaction_token,
                    dpp::snowflake command_id,
                    const class guild_settings_t &settings,
                    const std::string &embed_json, dpp::snowflake channelID);
  void SimpleEmbed(const std::string &interaction_token,
                   dpp::snowflake command_id,
                   const class guild_settings_t &settings,
                   const std::string &emoji, const std::string &text,
                   dpp::snowflake channelID, const std::string &title = "",
                   const std::string &image = "",
                   const std::string &thumbnail = "");
  void EmbedWithFields(const std::string &interaction_token,
                       dpp::snowflake command_id,
                       const class guild_settings_t &settings,
                       const std::string &title, std::vector<field_t> fields,
                       dpp::snowflake channelID, const std::string &url = "",
                       const std::string &image = "",
                       const std::string &thumbnail = "",
                       const std::string &description = "");

  std::string GetDescription() override;
  void UpdatePresenceLine();
  void show_stats(const std::string &interaction_token,
                  dpp::snowflake command_id, dpp::snowflake guild_id,
                  dpp::snowflake channel_id);
  void Tick();
  void DisposeThread(std::thread *t);
  bool OnMessage(const dpp::message_create_t &message,
                 const std::string &clean_message, bool mentioned,
                 const std::vector<std::string> &stringmentions) override;
  bool OnGuildCreate(const dpp::guild_create_t &guild) override;
  bool RealOnMessage(const dpp::message_create_t &message,
                     const std::string &clean_message, bool mentioned,
                     const std::vector<std::string> &stringmentions,
                     dpp::snowflake author_id = 0);
  void GetHelp(const std::string &interaction_token, dpp::snowflake command_id,
               const std::string &section, dpp::snowflake channelID,
               const std::string &botusername, dpp::snowflake botid,
               const std::string &author, dpp::snowflake authorid,
               const guild_settings_t &settings);

  /** DO NOT CALL THIS METHOD without wrapping it with the states_mutex.
   *
   * This is important for thread safety and to prevent race conditions!
   * Keep hold of the states_mutex until you're done with the object even if all
   * you do is read from it!
   *
   * Returns nullptr if there is no active game on the given channel id.
   */
  // state_t *GetState(dpp::snowflake channel_id);
};