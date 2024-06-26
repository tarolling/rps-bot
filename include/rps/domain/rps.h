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
#include <fmt/core.h>
#include <string>

namespace fs = std::filesystem;

constexpr std::string_view RPS_VERSION = "rps@1.0.0";

using json = dpp::json;

/**
 *  trim from end of string (right)
 */
inline std::string rtrim(std::string s) {
  s.erase(s.find_last_not_of(" \t\n\r\f\v") + 1);
  return s;
}

/**
 * trim from beginning of string (left)
 */
inline std::string ltrim(std::string s) {
  s.erase(0, s.find_first_not_of(" \t\n\r\f\v"));
  return s;
}

/**
 * trim from both ends of string (right then left)
 */
inline std::string trim(std::string s) { return ltrim(rtrim(s)); }

std::string sha256(const std::string &buffer);

inline long atol(const std::string &str) {
  if (str.empty())
    return 0;
  return atol(str.c_str());
}

inline long long atoll(const std::string &str) {
  if (str.empty())
    return 0;
  return atoll(str.c_str());
}

inline int atoi(const std::string &str) {
  if (str.empty())
    return 0;
  return atoi(str.c_str());
}

/**
 * @brief Replaces all occurences of substring in string
 * Source: <https://stackoverflow.com/a/29752943>
 *
 * @param source string being operated on
 * @param from original substring
 * @param to target substring
 */
inline void replace_all(std::string &source, const std::string &from,
                        const std::string &to) {
  std::string newString;
  newString.reserve(source.length()); // avoids a few memory allocations

  std::string::size_type lastPos = 0;
  std::string::size_type findPos;

  while (std::string::npos != (findPos = source.find(from, lastPos))) {
    newString.append(source, lastPos, findPos - lastPos);
    newString += to;
    lastPos = findPos + from.length();
  }

  // Care for the rest after last occurrence
  newString += source.substr(lastPos);

  source.swap(newString);
}