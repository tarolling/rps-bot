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
#include <cstdint>
#include <dpp/dpp.h>
#include <dpp/misc-enum.h>
#include <dpp/nlohmann/json.hpp>
#include <fmt/format.h>
#include <sporks/modules.h>
#include <sporks/statusfield.h>
#include <sporks/stringops.h>
#include <string>

using json = nlohmann::json;

/* Make a string safe to send as a JSON literal */
std::string RPSModule::escape_json(const std::string &s) {
  std::ostringstream o;
  for (char c : s) {
    switch (c) {
    case '"':
      o << "\\\"";
      break;
    case '\\':
      o << "\\\\";
      break;
    case '\b':
      o << "\\b";
      break;
    case '\f':
      o << "\\f";
      break;
    case '\n':
      o << "\\n";
      break;
    case '\r':
      o << "\\r";
      break;
    case '\t':
      o << "\\t";
      break;
    default:
      if ('\x00' <= c && c <= '\x1f') {
        o << "\\u" << std::hex << std::setw(4) << std::setfill('0') << (int)c;
      } else {
        o << c;
      }
    }
  }
  return o.str();
}

/* Create an embed from a JSON string and send it to a channel */
void RPSModule::ProcessEmbed(const guild_settings_t &settings,
                             const std::string &embed_json,
                             dpp::snowflake channelID) {
  ProcessEmbed("", 0, settings, embed_json, channelID);
}

/* Create an embed from a JSON string and send it to a channel */
void RPSModule::ProcessEmbed(const std::string &interaction_token,
                             dpp::snowflake command_id,
                             const guild_settings_t &settings,
                             const std::string &embed_json,
                             dpp::snowflake channelID) {
  json embed;
  std::string cleaned_json = embed_json;
  /* Put unicode zero-width spaces in @everyone and @here */
  cleaned_json = ReplaceString(cleaned_json, "@everyone", "@‎everyone");
  cleaned_json = ReplaceString(cleaned_json, "@here", "@‎here");
  try {
    /* Tabs to spaces */
    cleaned_json = ReplaceString(cleaned_json, "\t", " ");
    embed = json::parse(cleaned_json);
  } catch (const std::exception &e) {
    if (!bot->IsTestMode() ||
        from_string<uint64_t>(Bot::GetConfig("test_server"), std::dec) ==
            settings.guild_id) {
      try {
        if (!interaction_token.empty() && command_id != 0) {
          dpp::message msg(
              channelID, fmt::format("EMBED_ERROR_1", cleaned_json, e.what()));
          msg.guild_id = settings.guild_id;
          msg.channel_id = channelID;
          bot->core->interaction_response_edit(interaction_token, msg);
        } else {
          bot->core->message_create(dpp::message(
              channelID, fmt::format("EMBED_ERROR_1", cleaned_json, e.what())));
        }
      } catch (const std::exception &e) {
        bot->core->log(dpp::ll_error,
                       fmt::format("MALFORMED UNICODE: {}", e.what()));
      }
      bot->sent_messages++;
    }
  }
  if (!bot->IsTestMode() ||
      from_string<uint64_t>(Bot::GetConfig("test_server"), std::dec) ==
          settings.guild_id) {

    if (!interaction_token.empty() && command_id != 0) {
      dpp::message msg;
      msg.content = "";
      msg.guild_id = settings.guild_id;
      msg.channel_id = channelID;
      std::string real_interaction_token{interaction_token};
      if (real_interaction_token.substr(0, 9) == "EPHEMERAL") {
        real_interaction_token = real_interaction_token.substr(
            9, real_interaction_token.length() - 9);
        msg.set_flags(dpp::m_ephemeral);
      }
      msg.add_embed(dpp::embed(&embed));
      bot->core->interaction_response_edit(
          real_interaction_token, msg,
          [this](const dpp::confirmation_callback_t &callback) {
            if (callback.is_error()) {
              this->bot->core->log(
                  dpp::ll_error,
                  fmt::format("Can't edit interaction response: {}",
                              callback.http_info.body));
            }
          });
      bot->sent_messages++;
      return;
    }

    bot->sent_messages++;
  }
}

void RPSModule::SimpleEmbed(const guild_settings_t &settings,
                            const std::string &emoji, const std::string &text,
                            dpp::snowflake channelID, const std::string &title,
                            const std::string &image,
                            const std::string &thumbnail) {
  SimpleEmbed("", 0, settings, emoji, text, channelID, title, image, thumbnail);
}

void RPSModule::SimpleEmbed(const std::string &interaction_token,
                            dpp::snowflake command_id,
                            const guild_settings_t &settings,
                            const std::string &emoji, const std::string &text,
                            dpp::snowflake channelID, const std::string &title,
                            const std::string &image,
                            const std::string &thumbnail) {
  uint32_t color = settings.embedcolor;
  std::string imageinfo;
  /* Add image if there is one */
  if (!image.empty()) {
    imageinfo += R"(,"image":{"url":")" + escape_json(image) + "\"}";
  }
  if (!thumbnail.empty()) {
    imageinfo += R"(,"thumbnail":{"url":")" + escape_json(thumbnail) + "\"}";
  }
  imageinfo += R"(,"footer":{"text":")" + escape_json(_("POWERED_BY")) +
               R"(","icon_url":"https://i.imgur.com/R19P703.png"})";
  if (!title.empty()) {
    /* With title */
    ProcessEmbed(
        interaction_token, command_id, settings,
        fmt::format(R"({{"title":"{}","color":{},"description":"{} {}"{}}})",
                    escape_json(title), color, emoji, escape_json(text),
                    imageinfo),
        channelID);
  } else {
    /* Without title */
    ProcessEmbed(interaction_token, command_id, settings,
                 fmt::format(R"({{"color":{},"description":"{} {}"{}}})", color,
                             emoji, escape_json(text), imageinfo),
                 channelID);
  }
}

/* Send an embed containing one or more fields */
void RPSModule::EmbedWithFields(
    const guild_settings_t &settings, const std::string &title,
    std::vector<field_t> fields, dpp::snowflake channelID,
    const std::string &url, const std::string &image,
    const std::string &thumbnail, const std::string &description) {
  EmbedWithFields("", 0, settings, title, std::move(fields), channelID, url,
                  image, thumbnail, description);
}

/* Send an embed containing one or more fields */
void RPSModule::EmbedWithFields(
    const std::string &interaction_token, dpp::snowflake command_id,
    const class guild_settings_t &settings, const std::string &title,
    std::vector<field_t> fields, dpp::snowflake channelID,
    const std::string &url, const std::string &image,
    const std::string &thumbnail, const std::string &description) {
  uint32_t color = settings.embedcolor;
  std::string json;

  json = "{" + (!url.empty() ? R"("url":")" + escape_json(url) + "\"," : "");
  if (!description.empty()) {
    json +=
        fmt::format(R"("title":"{}","description":"{}","color":{},"fields":[)",
                    escape_json(title), escape_json(description), color);
  } else {
    json += fmt::format(R"("title":"{}","color":{},"fields":[)",
                        escape_json(title), color);
  }
  for (auto v = fields.begin(); v != fields.end(); ++v) {
    json += fmt::format(R"({{"name":"{}","value":"{}","inline":{}}})",
                        escape_json(v->name), escape_json(v->value),
                        v->_inline ? "true" : "false");
    auto n = v;
    if (++n != fields.end()) {
      json += ",";
    }
  }
  json += "],";
  /* Add image if there is one */
  if (!image.empty()) {
    json += R"("image":{"url":")" + escape_json(image) + "\"},";
  }
  if (!thumbnail.empty()) {
    json += R"("thumbnail":{"url":")" + escape_json(thumbnail) + "\"},";
  }
  /* Footer, 'powered by' detail, icon */
  json += R"("footer":{"text":")" + _("POWERED_BY") +
          R"(","icon_url":"https://i.imgur.com/R19P703.png"}})";
  ProcessEmbed(interaction_token, command_id, settings, json, channelID);
}
