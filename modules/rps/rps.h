#pragma once

#include "commands.h"
#include "settings.h"
#include <atomic>
#include <deque>
#include <dpp/dpp.h>
#include <map>
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
  std::mutex states_mutex;
  // std::map<dpp::snowflake, state_t> states;
  RPSModule(Bot *instigator, ModuleLoader *ml);
  Bot *GetBot();
  virtual ~RPSModule();
  RPSModule(const RPSModule &other);                     // copy constructor
  RPSModule(RPSModule &&other) noexcept;                 // move constructor
  RPSModule &operator=(const RPSModule &other) = delete; // copy assignment
  RPSModule &operator=(RPSModule &&other) noexcept;      // move assignment
  void SetupCommands();
  void HandleInteraction(const dpp::interaction_create_t &event);
  void HandleSelect(const dpp::select_click_t &event);
  void HandleButton(const dpp::button_click_t &event);
  void handle_command(const in_cmd &cmd,
                      const dpp::interaction_create_t &event);
  void ProcessCommands();
  virtual bool OnPresenceUpdate();
  bool OnAllShardsReady();
  virtual bool OnChannelDelete(const dpp::channel_delete_t &cd);
  virtual bool OnGuildDelete(const dpp::guild_delete_t &gd);

  /* Returns a local count */
  uint64_t GetActiveLocalGames();

  /* These return a sum across all clusters using the database */
  uint64_t GetActiveGames();
  uint64_t GetGuildTotal();
  uint64_t GetMemberTotal();
  uint64_t GetChannelTotal();

  virtual std::string GetDescription();
  int random(int min, int max);
  std::string dec_to_roman(uint64_t decimal, const guild_settings_t &settings);
  std::string tidy_num(std::string num);
  void UpdatePresenceLine();
  std::string conv_num(std::string datain, const guild_settings_t &settings);
  std::string numbertoname(uint64_t number, const guild_settings_t &settings);
  std::string GetNearestNumber(uint64_t number,
                               const guild_settings_t &settings);
  uint64_t GetNearestNumberVal(uint64_t number,
                               const guild_settings_t &settings);
  int min3(int x, int y, int z);
  int levenstein(std::string str1, std::string str2);
  bool is_number(const std::string &s);
  std::string MakeFirstHint(const std::string &s,
                            const guild_settings_t &settings,
                            bool indollars = false);
  void show_stats(const std::string &interaction_token,
                  dpp::snowflake command_id, dpp::snowflake guild_id,
                  dpp::snowflake channel_id);
  void Tick();
  void DisposeThread(std::thread *t);
  virtual bool OnMessage(const dpp::message_create_t &message,
                         const std::string &clean_message, bool mentioned,
                         const std::vector<std::string> &stringmentions);
  bool OnGuildCreate(const dpp::guild_create_t &guild);
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