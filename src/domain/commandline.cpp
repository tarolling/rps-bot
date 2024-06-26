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

#include <cstdlib>
#include <getopt.h>
#include <iostream>
#include <rps/domain/commandline.h>
#include <rps/domain/rps.h>
#include <unistd.h>

namespace commandline {
commandline_config parse(int argc, char const *argv[]) {

  struct option long_opts[] = {
      {"dev", no_argument, nullptr, 'd'},
      {"clusterid", required_argument, nullptr, 'c'},
      {"maxclusters", required_argument, nullptr, 'm'},
      {"showcommands", optional_argument, nullptr, 's'},
      {nullptr, 0, nullptr, 0}};

  int index{0};
  int arg;
  bool dev_mode{false}, clusters_defined{false}, show_commands{false};
  uint32_t cluster_id{0};
  uint32_t max_clusters{1};

  /**
   * BIG FAT WARNING: https://nullprogram.com/blog/2014/10/12/
   * getopt_long_only maybe not thread safe. This is only called before the bot
   * is initialised, so it's generally "ok-ish".
   */
  opterr = 0;
  while ((arg = getopt_long_only(argc, (char *const *)argv, "", long_opts,
                                 &index)) != -1) {
    switch (arg) {
    case 0:
      break;
    case 'd':
      dev_mode = true;
      break;
    case 'c':
      /* Cluster id */
      cluster_id = atoi(optarg);
      clusters_defined = true;
      break;
    case 'm':
      /* Number of clusters */
      max_clusters = atoi(optarg);
      break;
    case 's':
      /* Number of clusters */
      show_commands = true;
      break;
    case '?':
    default:
      std::cerr << "Unknown parameter '" << argv[optind - 1] << "'\n";
      std::cerr << "Usage: " << argv[0]
                << "[-dev] [-clusterid <n>] [-maxclusters <n>]\n\n";
      std::cerr << "-dev: Run the bot in development mode\n";
      std::cerr << "-clusterid <n>:    The current cluster id to identify for, "
                   "must be set with -maxclusters\n";
      std::cerr << "-maxclusters <n>:  The maximum number of clusters the bot "
                   "is running, must be set with -clusterid\n";
      std::cerr << "-showcommands:     Output JSON definitions of all "
                   "application commands\n";
      exit(1);
    }
  }

  if (clusters_defined && max_clusters == 0) {
    std::cerr << "ERROR: You have defined a cluster id with -clusterid but no "
                 "cluster count with -maxclusters.\n";
    exit(2);
  }

  return commandline_config{.dev = dev_mode,
                            .cluster_id = cluster_id,
                            .max_clusters = max_clusters,
                            .display_commands = show_commands};
}
} // namespace commandline