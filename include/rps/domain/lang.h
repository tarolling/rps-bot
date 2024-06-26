/************************************************************************************
 *
 * Copyright 1993,2001,2023 Craig Edwards <brain@ssod.org>
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
#pragma once

#include <dpp/dpp.h>
#include <fmt/format.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace i18n {

time_t get_mtime(const char *path);

void load_lang(dpp::cluster &bot);

void check_lang_reload(dpp::cluster &bot);

std::string tr(const std::string &k,
               const dpp::interaction_create_t &interaction);

dpp::slashcommand tr(dpp::slashcommand cmd);

template <typename... T>
std::string tr(const std::string &key,
               const dpp::interaction_create_t &interaction, T &&...args) {
  try {
    return fmt::format(fmt::runtime(tr(key, interaction)),
                       std::forward<T>(args)...);
  } catch (const std::exception &format_exception) {
    if (interaction.from && interaction.from->creator) {
      interaction.from->creator->log(
          dpp::ll_error, "Error in translation string for translation " + key +
                             " lang " + interaction.command.locale + ": " +
                             format_exception.what());
    }
    return key;
  }
}

} // namespace i18n