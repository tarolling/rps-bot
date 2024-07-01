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

#include <cppconn/connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <dpp/discordevents.h>
#include <dpp/misc-enum.h>
#include <dpp/stringops.h>
#include <memory>
#include <mutex>
#include <mysql_connection.h>
#include <mysql_driver.h>
#include <rps/data_source/database.h>
#include <rps/domain/config.h>
#include <rps/domain/rps.h>

namespace db {
/**
 * @brief MySQL database connection
 */
sql::mysql::MySQL_Connection *connection;

/**
 * @brief Database mutex
 * one connection may only be accessed by one thread at a time!
 */
std::mutex db_mutex;

/**
 * @brief Last error string from Neo4j
 */
std::string last_error;

/**
 * @brief Creating D++ cluster, used for logging
 */
dpp::cluster *creator{nullptr};

void init(dpp::cluster &bot) {
  creator = &bot;
  const json &dbconf = config::get("database");
  if (!db::connect(dbconf["host"], dbconf["username"], dbconf["password"],
                   dbconf["database"], dbconf["port"],
                   dbconf.contains("socket") ? dbconf["socket"] : "")) {
    creator->log(dpp::ll_critical,
                 fmt::format("Database connection error connecting to {}",
                             dbconf["database"]));
    exit(2);
  }
  creator->log(dpp::ll_info,
               fmt::format("Connected to database: {}", dbconf["database"]));
}

/**
 * @brief This is an internal connect function which has no locking, there is no
 * public interface for this
 *
 * @param host hostname
 * @param user username
 * @param pass password
 * @param db database
 * @param port port
 * @param socket unix socket
 */
bool unsafe_connect(const std::string &host, const std::string &user,
                    const std::string &pass, const std::string &db, int port,
                    const std::string &socket) {
  try {
    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();

    std::unique_ptr<sql::Connection> connection(
        driver->connect(fmt::format("tcp://{}:{}", host, port), user, pass));

    connection->setSchema(db);

    std::unique_ptr<sql::Statement> stmt(connection->createStatement());

    std::unique_ptr<sql::ResultSet> res(
        stmt->executeQuery("SELECT 'Connected to DB.' AS _message"));

    while (res->next()) {
      creator->log(dpp::ll_debug, res->getString("_message"));
    }
  } catch (sql::SQLException &e) {
    creator->log(dpp::ll_error, fmt::format("SQLException: {}", e.what()));
    creator->log(dpp::ll_error, fmt::format("SQLState: {}", e.getSQLState()));
    return false;
  }

  return true;
}

bool connect(const std::string &host, const std::string &user,
             const std::string &pass, const std::string &db, int port,
             const std::string &socket) {
  std::lock_guard<std::mutex> db_lock(db_mutex);
  return unsafe_connect(host, user, pass, db, port, socket);
}

bool close() {
  std::lock_guard<std::mutex> db_lock(db_mutex);
  curl_global_cleanup();
  return true;
}

size_t write_callback(void *contents, size_t size, size_t nmemb,
                      std::string *s) {
  size_t total_size = size * nmemb;
  s->append(static_cast<char *>(contents), total_size);
  return total_size;
}
} // namespace db