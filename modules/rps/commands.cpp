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
#include "commands.h"
#include "rps.h"
#include "settings.h"
#include <cstdint>
#include <dpp/dpp.h>
#include <dpp/misc-enum.h>
#include <dpp/nlohmann/json.hpp>
#include <dpp/snowflake.h>
#include <fmt/format.h>
#include <sporks/modules.h>
#include <sporks/statusfield.h>
#include <sporks/stringops.h>
#include <string>
#include <utility>

using json = nlohmann::json;

in_cmd::in_cmd(const std::string &m, uint64_t author, uint64_t channel,
               uint64_t guild, bool mention, const std::string &_user,
               bool dashboard, dpp::user u, dpp::guild_member gm)
    : msg(m), author_id(author), channel_id(channel), guild_id(guild),
      mentions_bot(mention), from_dashboard(dashboard), username(_user),
      user(u), member(std::move(gm)), command_id(0) {}

command_t::command_t(class RPSModule *_creator,
                     const std::string &_base_command, bool adm,
                     const std::string &descr,
                     std::vector<dpp::command_option> options,
                     bool is_ephemeral,
                     dpp::slashcommand_contextmenu_type command_type)
    : creator(_creator), base_command(_base_command), description(descr),
      opts(std::move(options)), admin(adm), ephemeral(is_ephemeral),
      type(command_type) {}

command_t::~command_t() {}

void command_t::select_click(const dpp::select_click_t &event,
                             const in_cmd &cmd, guild_settings_t &settings) {}

void command_t::button_click(const dpp::button_click_t &event,
                             const in_cmd &cmd, guild_settings_t &settings) {}

/* Register slash commands and build the array of standard message commands */
void RPSModule::SetupCommands() {
  /* This is a list of commands which are handled by both the message based
   * oldschool command handler and also via slash commands. Any entry here with
   * an empty description is a message command alias, and won't be registered as
   * a slash command (no point - we have tab completion there)
   */
  commands = {
      {"queue", new command_queue_t(
                    this, "queue", false, "Enter into the RPS queue",
                    {dpp::command_option(dpp::co_integer, "minutes",
                                         "Minutes to stay in queue", false)})},
      {"leave",
       new command_leave_t(this, "leave", false, "Leave the RPS queue", {})}};

  if (bot->GetClusterID() == 0) {
    /* Two lists, one for the main set of global commands, and one for admin
     * commands */
    std::vector<dpp::slashcommand> slashcommands;
    std::vector<dpp::slashcommand> adminslash;

    /* Iterate the list and build dpp::slashcommand object vector */
    for (auto &c : commands) {
      if (!c.second->description.empty()) {
        dpp::slashcommand sc;
        sc.set_type(c.second->type) // Must set type first to prevent
                                    // lowercasing of context menu items
            .set_name(c.first)
            .set_description(c.second->description)
            .set_application_id(from_string<dpp::snowflake>(
                Bot::GetConfig("application_id"), std::dec));
        bot->core->log(dpp::ll_debug,
                       fmt::format("what is this -> {}", sc.application_id));
        for (auto &o : c.second->opts) {
          sc.add_option(o);
        }
        if (c.second->admin) {
          adminslash.push_back(sc);
        } else {
          slashcommands.push_back(sc);
        }
      }
    }

    bot->core->log(
        dpp::ll_info,
        fmt::format("Registering {} global and {} local slash commands",
                    slashcommands.size(), adminslash.size()));

    /* Now register all the commands */
    if (bot->IsDevMode()) {
      /*
       * Development mode - all commands are guild commands, and all are
       * attached to the test server.
       */
      std::copy(std::begin(adminslash), std::end(adminslash),
                std::back_inserter(slashcommands));
      bot->core->guild_bulk_command_create(
          slashcommands,
          from_string<uint64_t>(Bot::GetConfig("test_server"), std::dec),
          [this](const dpp::confirmation_callback_t &callback) {
            if (callback.is_error()) {
              this->bot->core->log(
                  dpp::ll_error,
                  fmt::format("Failed to register guild slash commands (dev "
                              "mode, main set): {}",
                              callback.http_info.body));
            } else {
              dpp::slashcommand_map sm =
                  std::get<dpp::slashcommand_map>(callback.value);
              this->bot->core->log(
                  dpp::ll_info,
                  fmt::format("Registered {} guild commands to test server",
                              sm.size()));
            }
          });
    } else {
      /*
       * Live mode/test mode - main set are global slash commands (with their
       * related cache delay, ew) Admin set are guild commands attached to the
       * support server.
       */
      bot->core->global_bulk_command_create(
          slashcommands, [this](const dpp::confirmation_callback_t &callback) {
            if (callback.is_error()) {
              this->bot->core->log(
                  dpp::ll_error,
                  fmt::format("Failed to register global slash commands (live "
                              "mode, main set): {}",
                              callback.http_info.body));
            } else {
              dpp::slashcommand_map sm =
                  std::get<dpp::slashcommand_map>(callback.value);
              this->bot->core->log(
                  dpp::ll_info,
                  fmt::format("Registered {} global commands", sm.size()));
            }
          });
      bot->core->guild_bulk_command_create(
          adminslash, from_string<uint64_t>(bot->GetConfig("home"), std::dec),
          [this](const dpp::confirmation_callback_t &callback) {
            if (callback.is_error()) {
              this->bot->core->log(
                  dpp::ll_error,
                  fmt::format("Failed to register guild slash commands (live "
                              "mode, admin set): {}",
                              callback.http_info.body));
            } else {
              dpp::slashcommand_map sm =
                  std::get<dpp::slashcommand_map>(callback.value);
              this->bot->core->log(
                  dpp::ll_info,
                  fmt::format("Registered {} guild commands to support server",
                              sm.size()));
            }
          });
    }
  }

  /* Hook interaction events */
  bot->core->on_interaction_create(
      std::bind(&RPSModule::HandleInteraction, this, std::placeholders::_1));
  bot->core->on_select_click(
      std::bind(&RPSModule::HandleSelect, this, std::placeholders::_1));
  bot->core->on_button_click(
      std::bind(&RPSModule::HandleButton, this, std::placeholders::_1));
}

void RPSModule::HandleSelect(const dpp::select_click_t &event) {
  auto command = commands.find(event.custom_id);
  if (command != commands.end()) {
    in_cmd cmd(event.values[0], event.command.usr.id, event.command.channel_id,
               event.command.guild_id, false, event.command.usr.username, false,
               event.command.usr, event.command.member);
    cmd.command_id = event.command.id;
    cmd.interaction_token = event.command.token;
    // guild_settings_t settings = GetGuildSettings(cmd.guild_id);
    // command->second->select_click(event, cmd, settings);
  }
}

void RPSModule::HandleButton(const dpp::button_click_t &event) {
  std::string identifier = event.custom_id.substr(0, event.custom_id.find(','));
  std::string remainder = event.custom_id.substr(event.custom_id.find(',') + 1);
  auto command = commands.find(identifier);
  if (command != commands.end()) {
    in_cmd cmd(remainder, event.command.usr.id, event.command.channel_id,
               event.command.guild_id, false, event.command.usr.username, false,
               event.command.usr, event.command.member);
    cmd.command_id = event.command.id;
    cmd.interaction_token = event.command.token;
    // guild_settings_t settings = GetGuildSettings(cmd.guild_id);
    // command->second->button_click(event, cmd, settings);
  }
}

void RPSModule::HandleInteraction(const dpp::interaction_create_t &event) {
  if (event.command.type == dpp::it_application_command) {
    dpp::command_interaction cmd_interaction =
        std::get<dpp::command_interaction>(event.command.data);

    std::stringstream message;

    message << cmd_interaction.name;
    for (auto &p : cmd_interaction.options) {
      if (std::holds_alternative<int64_t>(p.value)) {
        message << " " << std::get<int64_t>(p.value);
      } else if (std::holds_alternative<dpp::snowflake>(p.value)) {
        message << " " << std::get<dpp::snowflake>(p.value);
      } else if (std::holds_alternative<std::string>(p.value)) {
        message << " " << std::get<std::string>(p.value);
      }
    }

    in_cmd cmd(message.str(), event.command.usr.id, event.command.channel_id,
               event.command.guild_id, false, event.command.usr.username, false,
               event.command.usr, event.command.member);
    cmd.command_id = event.command.id;
    cmd.interaction_token = event.command.token;
    if (cmd_interaction.type == dpp::ctxm_chat_input) {
      bot->core->log(dpp::ll_info,
                     fmt::format("SCMD (USER={}, GUILD={}): <{}> /{}",
                                 cmd.author_id, cmd.guild_id, cmd.username,
                                 cmd.msg));
    } else {
      bot->core->log(dpp::ll_info,
                     fmt::format("CONTEXT (USER={}, GUILD={}): {} -> '{}'",
                                 cmd.author_id, cmd.guild_id, cmd.username,
                                 cmd.msg));
    }
    this->handle_command(cmd, event);
  }
}

dpp::user dashboard_dummy;

void RPSModule::thinking(bool ephemeral,
                         const dpp::interaction_create_t &event) {
  if (event.command.guild_id.empty()) {
    return;
  }
  /* Set 'thinking' state */
  dpp::message msg;
  msg.content = "*";
  msg.guild_id = event.command.guild_id;
  msg.channel_id = event.command.channel_id;
  if (ephemeral) {
    msg.set_flags(dpp::m_ephemeral);
  }
  bot->core->interaction_response_create(
      event.command.id, event.command.token,
      dpp::interaction_response(dpp::ir_deferred_channel_message_with_source,
                                msg));
}

bool RPSModule::has_rl_warn(dpp::snowflake channel_id) {
  std::shared_lock cmd_lock(cmdmutex);
  auto check = last_rl_warning.find(channel_id);
  return check != last_rl_warning.end() && time(NULL) < check->second;
}

bool RPSModule::has_limit(dpp::snowflake channel_id) {
  std::shared_lock cmd_lock(cmdmutex);
  auto check = limits.find(channel_id);
  return check != limits.end() && time(NULL) < check->second;
}

bool RPSModule::set_rl_warn(dpp::snowflake channel_id) {
  std::unique_lock cmd_lock(cmdmutex);
  last_rl_warning[channel_id] = time(NULL) + per_channel_rate_limit;
  return true;
}

bool RPSModule::set_limit(dpp::snowflake channel_id) {
  std::unique_lock cmd_lock(cmdmutex);
  limits[channel_id] = time(NULL) + per_channel_rate_limit;
  return true;
}

void RPSModule::handle_command(const in_cmd &cmd,
                               const dpp::interaction_create_t &event) {

  try {
    if (!bot->IsTestMode() ||
        from_string<uint64_t>(Bot::GetConfig("test_server"), std::dec) ==
            cmd.guild_id) {

      std::stringstream tokens(cmd.msg);
      std::string base_command;
      tokens >> base_command;

      dpp::channel *c = dpp::find_channel(cmd.channel_id);
      dpp::user *user = (dpp::user *)&cmd.user;
      if (cmd.from_dashboard) {
        dashboard_dummy.username = "Dashboard";
        dashboard_dummy.flags = 0;
        user = &dashboard_dummy;
      }
      if (!c) {
        return;
      }

      guild_settings_t gs(time(NULL), 0, "!", {}, 3238819, false, false, false,
                          false, 0, "", "en", 20, 200, 15, 200, false);

      base_command = lowercase(base_command);

      /* Support for old-style commands e.g. !rps start instead of !start */
      if (base_command == "rps") {
        tokens >> base_command;
        base_command = lowercase(base_command);
      }

      auto command = commands.end();
      if (!cmd.interaction_token.empty()) {
        /* For interactions, if we can't find the command then it's a context
         * menu action */
        command = commands.find(cmd.msg);
      }
      if (command == commands.end()) {
        command = commands.find(base_command);
      }
      if (command != commands.end()) {

        this->thinking(command->second->ephemeral, event);

        bool can_execute = false;
        if (!this->has_limit(cmd.channel_id)) {
          can_execute = true;
          this->set_limit(cmd.channel_id);
        }

        if (can_execute || cmd.from_dashboard || command->second->ephemeral) {
          bot->core->log(
              dpp::ll_debug,
              fmt::format("command_t '{}' routed to handler", command->first));
          command->second->call(cmd, tokens, gs, cmd.username, false, c, user);
        } else {
          /* Display rate limit message, but only one per rate limit period */
          if (!this->has_rl_warn(cmd.channel_id)) {
            this->set_rl_warn(cmd.channel_id);
            // SimpleEmbed(cmd.interaction_token, cmd.command_id, settings,
            //             ":snail:",
            //             fmt::format(_("RATELIMITED", settings),
            //                         per_channel_rate_limit, base_command),
            //             cmd.channel_id, _("WOAHTHERE", settings));
          }
          bot->core->log(dpp::ll_debug,
                         fmt::format("command_t '{}' NOT routed to handler on "
                                     "channel {}, limiting",
                                     base_command, cmd.channel_id));
        }
      } else {
        this->thinking(false, event);

        // /* Custom commands handled completely by the API as a REST call */
        // bool command_exists = false;
        // {
        //   std::shared_lock cmd_list_lock(cmds_mutex);
        //   command_exists =
        //       (std::find(api_commands.begin(), api_commands.end(),
        //                  trim(lowercase(base_command))) !=
        //                  api_commands.end());
        // }
        // if (command_exists) {
        //   bool can_execute = false;
        //   if (!this->has_limit(cmd.channel_id)) {
        //     can_execute = true;
        //     this->set_limit(cmd.channel_id);
        //   }

        //   if (can_execute) {
        //     std::string rest;
        //     std::getline(tokens, rest);
        //     rest = trim(rest);
        //     custom_command(cmd.interaction_token, cmd.command_id, settings,
        //                    this, base_command, trim(rest), cmd.author_id,
        //                    cmd.channel_id, cmd.guild_id);
        //   } else {
        //     /* Display rate limit message, but only one per rate limit period
        //     */ if (!this->has_rl_warn(cmd.channel_id)) {
        //       this->set_rl_warn(cmd.channel_id);
        //       //   SimpleEmbed(cmd.interaction_token, cmd.command_id,
        //       settings,
        //       //               ":snail:",
        //       //               fmt::format("RATELIMITED",
        //       //               per_channel_rate_limit,
        //       //                           base_command),
        //       //               cmd.channel_id, "WOAHTHERE");
        //     }
        //     bot->core->log(
        //         dpp::ll_debug,
        //         fmt::format("Command '{}' not sent to API, rate limited",
        //                     trim(lowercase(base_command))));
        //   }
        // } else {
        //   bot->core->log(dpp::ll_debug,
        //                  fmt::format("Command '{}' not known to API",
        //                              trim(lowercase(base_command))));
        // }
      }
    } else {
      bot->core->log(
          dpp::ll_debug,
          fmt::format("Dropped command {} due to test mode", cmd.msg));
    }
  } catch (std::exception &e) {
    bot->core->log(dpp::ll_debug,
                   fmt::format("command_t exception! - {}", e.what()));
  } catch (...) {
    bot->core->log(dpp::ll_debug, "command_t exception! - non-object");
  }
}