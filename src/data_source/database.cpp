/************************************************************************************
 *
 * Copyright 2024 tarolling
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

#include <mutex>
#include <rps/data_source/database.h>
#include <rps/domain/config.h>
#include <rps/domain/rps.h>

namespace db {
/**
 * @brief Database mutex
 * one connection may only be accessed by one thread at a time!
 */
std::mutex db_mutex;

/**
 * @brief Creating D++ cluster, used for logging
 */
dpp::cluster *creator{nullptr};

void init(dpp::cluster &bot) {
  creator = &bot;
  //   const json &dbconf = config::get("database");
  //   if (!db::connect(dbconf["host"], dbconf["username"], dbconf["password"],
  //                    dbconf["database"], dbconf["port"],
  //                    dbconf.contains("socket") ? dbconf["socket"] : "")) {
  //     creator->log(dpp::ll_critical,
  //                  fmt::format("Database connection error connecting to {}:
  //                  {}",
  //                              dbconf["database"],
  //                              mysql_error(&connection)));
  //     exit(2);
  //   }
  //   creator->log(dpp::ll_info,
  //                fmt::format("Connected to database: {}",
  //                dbconf["database"]));
}
} // namespace db