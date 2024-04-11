#pragma once

#include "commands.h"

#include <cstdint>
#include <dpp/dpp.h>
#include <sporks/modules.h>

// Number of seconds after which a game is considered hung and its thread exits.
// This can happen if a game gets lost in a discord gateway outage (!)
constexpr int32_t game_reap_secs = 20000;

// Number of seconds between allowed API-bound calls, per channel
constexpr int8_t per_channel_rate_limit = 4;

class RPSModule : public Module {
  std::thread *presence_update;
  bool terminating;
  std::thread *command_processor;
  command_list_t commands;

  bool booted;

public:
  time_t startup;
  RPSModule(Bot *instigator, ModuleLoader *ml);
  Bot *GetBot();
  virtual ~RPSModule();
  RPSModule(const RPSModule &other);           // copy constructor
  RPSModule(RPSModule &&other) noexcept;       // move constructor
  RPSModule &operator=(const RPSModule &other) // copy assignment
  {
    return *this = RPSModule(other);
  }
  RPSModule &operator=(RPSModule &&other) noexcept; // move assignment
  void SetupCommands();
  void ProcessCommands();
  bool OnAllShardsReady() override;

  void UpdatePresenceLine();
  bool OnGuildCreate(const dpp::guild_create_t &guild) override;
};