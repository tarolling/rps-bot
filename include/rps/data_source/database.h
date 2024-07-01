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

#pragma once

#include <dpp/cluster.h>

namespace db {
/**
 * @brief Initialise database connection
 *
 * @param bot creating D++ cluster
 */
void init(dpp::cluster &bot);

/**
 * @brief Connect to database and set options
 *
 * @param host Database hostname
 * @param user Database username
 * @param pass Database password
 * @param db Database schema name
 * @param port Database port number
 * @param socket unix socket path
 * @return True if the database connection succeeded
 *
 * @note Unix socket and port number are mutually exclusive. If you set socket
 * to a non-empty string, you should set port to 0 and host to `localhost`. This
 * is a special value in the mysql client and causes a unix socket connection to
 * occur. If you do not want to use unix sockets, keep the value as an empty
 * string.
 */
bool connect(const std::string &host, const std::string &user,
             const std::string &pass, const std::string &db, int port = 7474,
             const std::string &socket = "");

/**
 * @brief Disconnect from database and free query cache
 *
 * @return true on successful disconnection
 */
bool close();

/**
 * @brief Writes response data into buffer
 *
 * @param contents
 * @param size
 * @param nmemb
 * @param s
 * @return size_t
 */
size_t write_callback(void *contents, size_t size, size_t nmemb,
                      std::string *s);
} // namespace db