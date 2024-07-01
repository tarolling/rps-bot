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

  // if (mysql_init(&connection) != nullptr) {
  //   mysql_options(&connection, MYSQL_INIT_COMMAND, CONNECT_STRING);
  //   int opts = CLIENT_MULTI_RESULTS | CLIENT_MULTI_STATEMENTS |
  //              CLIENT_REMEMBER_OPTIONS | CLIENT_IGNORE_SIGPIPE;
  //   bool result = mysql_real_connect(
  //       &connection, host.c_str(), user.c_str(), pass.c_str(), db.c_str(),
  //       port, socket.empty() ? nullptr : socket.c_str(), opts);
  //   signal(SIGPIPE, SIG_IGN);
  //   return result;
  // } else {
  //   last_error = "mysql_init() failed";
  //   return false;
  // }
}

bool connect(const std::string &host, const std::string &user,
             const std::string &pass, const std::string &db, int port,
             const std::string &socket) {
  std::lock_guard<std::mutex> db_lock(db_mutex);
  return unsafe_connect(host, user, pass, db, port, socket);
}

// bool connect(const std::string &host, const std::string &user,
//              const std::string &pass, const std::string &db, int port,
//              const std::string &socket) {
//   std::lock_guard<std::mutex> db_lock(db_mutex);

//   CURL *curl = nullptr;
//   CURLcode res{};
//   std::string read_buf;

//   curl_global_init(CURL_GLOBAL_DEFAULT);
//   curl = curl_easy_init();
//   if (curl) {
//     std::string url =
//         fmt::format("postgresql://{}:{}@{}:{}/{}?sslmode=verify-full", user,
//                     pass, host, port, db);
//     const char *fields = R"({"statement": "RETURN 1"})";

//     curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
//     curl_easy_setopt(curl, CURLOPT_POST, 1L);
//     curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields);

//     curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
//     curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL);

//     curl_easy_setopt(curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC);
//     curl_easy_setopt(curl, CURLOPT_USERPWD,
//                      fmt::format("{}:{}", user, pass).c_str());

//     struct curl_slist *headers = nullptr;
//     headers = curl_slist_append(headers, "Accept: application/json");
//     headers = curl_slist_append(headers, "Content-Type: application/json");
//     curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

//     /* Set read buffers */
//     curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
//     curl_easy_setopt(curl, CURLOPT_WRITEDATA, &read_buf);

//     /* For debugging purposes */
//     char error_buffer[CURL_ERROR_SIZE];
//     curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_buffer);
//     curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

//     /* Send the authentication to any host (ik this isn't what ur supposed to
//     do
//      * but idc) */
//     curl_easy_setopt(curl, CURLOPT_UNRESTRICTED_AUTH, 1L);
//     curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 10L);

//     /* Perform the request and get the response code */
//     res = curl_easy_perform(curl);

//     /* Check for errors */
//     if (res != CURLE_OK) {
//       creator->log(dpp::ll_error,
//                    fmt::format("curl_easy_perform() failed: {} - {}",
//                                curl_easy_strerror(res), error_buffer));
//       curl_easy_cleanup(curl);
//       curl_slist_free_all(headers);
//       curl_global_cleanup();
//       return false;
//     } else {
//       long response_code;
//       curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &response_code);

//       if (response_code == 202) {
//         creator->log(dpp::ll_debug, "POST request sent successfully!");
//       } else {
//         creator->log(dpp::ll_debug,
//                      fmt::format("response code - {}", response_code));
//         creator->log(dpp::ll_debug, fmt::format("Response data: {}",
//         read_buf)); curl_easy_cleanup(curl); curl_slist_free_all(headers);
//         curl_global_cleanup();
//         return false;
//       }
//     }

//     /* Clean up */
//     curl_easy_cleanup(curl);
//     curl_slist_free_all(headers);
//   }

//   curl_global_cleanup();
//   return true;
// }

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