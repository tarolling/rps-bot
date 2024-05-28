#pragma once

#include "settings.h"
#include <dpp/dpp.h>
#include <map>
#include <string>

#define DECLARE_COMMAND_CLASS(__command_name__, __ancestor__)                  \
  class __command_name__ : public __ancestor__ {                               \
  public:                                                                      \
    __command_name__(class RPSModule *_creator,                                \
                     const std::string &_base_command, bool adm,               \
                     const std::string &descr,                                 \
                     std::vector<dpp::command_option> options);                \
    virtual void call(const in_cmd &cmd, std::stringstream &tokens,            \
                      guild_settings_t &settings, const std::string &username, \
                      bool is_moderator, dpp::channel *c, dpp::user *user);    \
    virtual ~__command_name__() = default;                                     \
    __command_name__(const __command_name__ &other);                           \
    __command_name__(__command_name__ &&other) noexcept = default;             \
    __command_name__ &operator=(const __command_name__ &other) = default;      \
    __command_name__ &operator=(__command_name__ &&other) noexcept = default;  \
  };
#define DECLARE_COMMAND_CLASS_SELECT(__command_name__, __ancestor__)           \
  class __command_name__ : public __ancestor__ {                               \
  public:                                                                      \
    __command_name__(class RPSModule *_creator,                                \
                     const std::string &_base_command, bool adm,               \
                     const std::string &descr,                                 \
                     std::vector<dpp::command_option> options);                \
    virtual void call(const in_cmd &cmd, std::stringstream &tokens,            \
                      guild_settings_t &settings, const std::string &username, \
                      bool is_moderator, dpp::channel *c, dpp::user *user);    \
    virtual ~__command_name__() = default;                                     \
    __command_name__(const __command_name__ &other);                           \
    __command_name__(__command_name__ &&other) noexcept;                       \
    __command_name__ &operator=(const __command_name__ &other) = default;      \
    __command_name__ &operator=(__command_name__ &&other) noexcept = default;  \
    virtual void select_click(const dpp::select_click_t &event,                \
                              const in_cmd &cmd, guild_settings_t &settings);  \
    virtual void button_click(const dpp::button_click_t &event,                \
                              const in_cmd &cmd, guild_settings_t &settings);  \
  };

#define BLANK_EMOJI "<:blank:667278047006949386>"

class in_cmd {
public:
  std::string msg;
  std::string username;
  dpp::user user;
  dpp::guild_member member;
  uint64_t author_id;
  uint64_t channel_id;
  uint64_t guild_id;
  bool mentions_bot;
  bool from_dashboard;
  std::string interaction_token;
  dpp::snowflake command_id;
  in_cmd(const std::string &m, uint64_t author, uint64_t channel,
         uint64_t guild, bool mention, const std::string &username,
         bool dashboard, dpp::user u, dpp::guild_member gm);
};

class command_t {
protected:
  class RPSModule *creator;
  std::string base_command;
  std::string _(const std::string &str, const guild_settings_t &settings);

public:
  bool admin;
  bool ephemeral;
  dpp::slashcommand_contextmenu_type type;
  std::string description;
  std::vector<dpp::command_option> opts;
  command_t(
      class RPSModule *_creator, const std::string &_base_command, bool adm,
      const std::string &descr, std::vector<dpp::command_option> options,
      bool is_ephemeral = false,
      dpp::slashcommand_contextmenu_type command_type = dpp::ctxm_chat_input);
  virtual void call(const in_cmd &cmd, std::stringstream &tokens,
                    guild_settings_t &settings, const std::string &username,
                    bool is_moderator, dpp::channel *c, dpp::user *user) = 0;
  virtual void select_click(const dpp::select_click_t &event, const in_cmd &cmd,
                            guild_settings_t &settings);
  virtual void button_click(const dpp::button_click_t &event, const in_cmd &cmd,
                            guild_settings_t &settings);
  virtual ~command_t();
  command_t(const command_t &other);                      // copy constructor
  command_t(command_t &&other) noexcept;                  // move constructor
  command_t &operator=(const command_t &other) = default; // copy assignment
  command_t &operator=(command_t &&other) noexcept;       // move assignment
};

DECLARE_COMMAND_CLASS(command_queue_t, command_t);
DECLARE_COMMAND_CLASS(command_leave_t, command_t);

using command_list_t = std::multimap<std::string, command_t *>;