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

#include "state.h"
#include "rps.h"
#include "time.h"
#include <cstdint>
#include <dpp/dpp.h>
#include <fmt/format.h>
#include <fstream>
#include <sporks/modules.h>
#include <sporks/statusfield.h>
#include <sporks/stringops.h>
#include <streambuf>
#include <string>
#include <unistd.h>

std::unordered_map<uint64_t, bool> banlist;

state_t::state_t()
    : next_tick(time(nullptr)), creator(nullptr), terminating(false){};

state_t::state_t(RPSModule *_creator, uint32_t questions, uint32_t currstreak,
                 uint64_t lastanswered, uint32_t question_index,
                 uint32_t _interval, uint64_t _channel_id, bool _hintless,
                 const std::vector<std::string> &_shuffle_list,
                 uint64_t _guild_id)
    :

      next_tick(time(nullptr)), creator(_creator), terminating(false) {
  creator->GetBot()->core->log(dpp::ll_debug,
                               fmt::format("state_t::state_t()"));
}

/* Games are a finite state machine, where the tick() function is called
 * periodically on each state_t object. The next_tick value indicates when the
 * tick() method should next be called, if the terminating flag is set then the
 * object is removed from the list of current games. Each cluster only stores a
 * game list for itself.
 */
void state_t::tick() {
  // guild_settings_t settings = creator->GetGuildSettings(guild_id);
  // if (!is_valid()) {
  // 	log_game_end(guild_id, channel_id);
  // 	terminating = true;
  // 	gamestate = TRIV_END;
  // 	return;
  // }
  // try {
  // 	if (question_cache.empty()) {
  // 			build_question_cache(settings);
  // 	}
  // 	switch (gamestate) {
  // 		case TRIV_ASK_QUESTION:
  // 			if (!terminating) {
  // 				if (is_insane_round(settings)) {
  // 					do_insane_round(false, settings);
  // 				} else {
  // 					do_normal_round(false, settings);
  // 				}
  // 			}
  // 		break;
  // 		case TRIV_FIRST_HINT:
  // 			if (!terminating) {
  // 				do_first_hint(settings);
  // 			}
  // 		break;
  // 		case TRIV_SECOND_HINT:
  // 			if (!terminating) {
  // 				do_second_hint(settings);
  // 			}
  // 		break;
  // 		case TRIV_TIME_UP:
  // 			if (!terminating) {
  // 				do_time_up(settings);
  // 			}
  // 		break;
  // 		case TRIV_ANSWER_CORRECT:
  // 			if (!terminating) {
  // 				do_answer_correct(settings);
  // 			}
  // 		break;
  // 		case TRIV_END:
  // 			do_end_game(settings);
  // 		break;
  // 		default:
  // 			creator->GetBot()->core->log(dpp::ll_warning,
  // fmt::format("Invalid state '{}', ending round.", gamestate));
  // gamestate = TRIV_END; 			terminating = true;
  // break;
  // 	}

  // 	if (gamestate == TRIV_ANSWER_CORRECT) {
  // 		/* Correct answer shortcuts the timer */
  // 		next_tick = time(NULL);
  // 	} else {
  // 		/* Set time for next tick */
  // 		if (gamestate == TRIV_ASK_QUESTION && interval == TRIV_INTERVAL)
  // { 			next_tick = time(NULL) + settings.question_interval;
  // } else { 			next_tick = time(NULL) + interval;
  // 		}
  // 	}
  // }
  // catch (std::exception &e) {
  // 	creator->GetBot()->core->log(dpp::ll_debug, fmt::format("state_t
  // exception! - {}", e.what()));
  // }
  // catch (...) {
  // 	creator->GetBot()->core->log(dpp::ll_debug, fmt::format("state_t
  // exception! - non-object"));
  // }
}