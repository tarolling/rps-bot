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

#include <curl/curl.h>
#include <curl/easy.h>
#include <dpp/discordevents.h>
#include <dpp/misc-enum.h>
#include <dpp/stringops.h>
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
                   dbconf["db_name"], dbconf["port"],
                   dbconf.contains("socket") ? dbconf["socket"] : "")) {
    creator->log(dpp::ll_critical,
                 fmt::format("Database connection error connecting to {}",
                             dbconf["db_name"]));
    exit(2);
  }
  creator->log(dpp::ll_info,
               fmt::format("Connected to database: {}", dbconf["db_name"]));
}

bool connect(const std::string &host, const std::string &user,
             const std::string &pass, const std::string &db, int port,
             const std::string &socket) {
  std::lock_guard<std::mutex> db_lock(db_mutex);

  CURL *curl = nullptr;
  CURLcode res{};

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if (curl) {
    std::string url =
        fmt::format("http://{}:{}/db/{}/tx/commit", host, port, db).c_str();
    const char *fields = R"({"statements": [{"statement": "RETURN 1" }]})";

    // FIXME: Security vulnerability probably
    std::string raw_creds = fmt::format("{}:{}", user, pass);
    std::string encoded_creds = dpp::base64_encode(
        reinterpret_cast<const unsigned char *>(raw_creds.c_str()),
        raw_creds.size());

    /* Set the URL */
    const char *url_cstr = url.c_str();
    curl_easy_setopt(curl, CURLOPT_URL, url_cstr);

    // Specify that we want to perform a POST request
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // Set the POST data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, fields);

    // Define the headers
    struct curl_slist *headers = nullptr;
    headers =
        curl_slist_append(headers, "Accept: application/json;charset=UTF-8");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(
        headers, fmt::format("Authorization: Basic {}", encoded_creds).c_str());

    // Set the headers
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Perform the request and get the response code
    res = curl_easy_perform(curl);

    // Check for errors
    if (res != CURLE_OK) {
      creator->log(dpp::ll_error, fmt::format("curl_easy_perform() failed: {}",
                                              curl_easy_strerror(res)));
      // Clean up
      curl_easy_cleanup(curl);
      curl_slist_free_all(headers);
      curl_global_cleanup();
      return false;
    } else {
      creator->log(dpp::ll_debug, "POST request sent successfully!");
    }

    // Clean up
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
  }

  curl_global_cleanup();
  return true;
}
} // namespace db