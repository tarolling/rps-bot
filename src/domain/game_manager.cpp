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

#include <chrono>
#include <dpp/misc-enum.h>
#include <rps/domain/embeds.h>
#include <rps/domain/game.h>
#include <rps/domain/game_manager.h>

using namespace game;

namespace game_manager {
/**
 * @brief Creating rps_bot
 */
dpp::cluster *creator{nullptr};

void init(dpp::cluster &bot) {
  creator = &bot;
  creator->log(dpp::ll_info, "Game manager initialized");
}

void send_ban_message(const unsigned int lobby_id) {
  clear_game_timer(creator, lobby_id);

  unsigned int player_one_ban = get_player_ban(lobby_id, 0);
  unsigned int player_two_ban = get_player_ban(lobby_id, 1);

  if (player_one_ban == 0 && player_two_ban == 0) {
    /* Beginning of ban phase */
    /* Choose higher elo player to send msg (for now, random) */
    long player_index =
        std::chrono::system_clock::now().time_since_epoch().count() % 2;
    creator->direct_message_create(
        get_player_id(lobby_id, player_index),
        embeds::ban(get_player_interaction(lobby_id, player_index), lobby_id),
        [=](const dpp::confirmation_callback_t &callback) {
          if (callback.is_error()) {
            creator->log(
                dpp::ll_error,
                fmt::format("Error: {}", callback.get_error().human_readable));
            return;
          }
          start_game_timer(lobby_id, creator->start_timer(
                                         [=](unsigned long t) {
                                           handle_timeout(lobby_id);
                                           creator->stop_timer(t);
                                         },
                                         GAME_TIMEOUT));
        });
    creator->direct_message_create(
        get_player_id(lobby_id, 1 - player_index),
        dpp::message("Waiting for other player to ban..."));
    return;
  }

  if (player_one_ban != 0 && player_two_ban != 0) {
    unsigned int sum =
        get_player_ban(lobby_id, 0) + get_player_ban(lobby_id, 1);
    /* As unreadable as possible! */
    unsigned int first_to = (sum == 7) ? 5 : ((sum == 8) ? 4 : 3);
    set_first_to(lobby_id, first_to);

    rps_lobby found_lobby = get_lobby(lobby_id);
    if (found_lobby.id == 0) {
      return;
    }

    for (const auto &player_info : found_lobby.players) {
      creator->direct_message_create_sync(
          player_info->player.id,
          dpp::message(fmt::format("The game will now begin! First to {} wins.",
                                   get_first_to(lobby_id))));
    }
    send_game_messages(lobby_id);
    return;
  }

  if (player_one_ban != 0) {
    creator->direct_message_create(
        get_player_id(lobby_id, 1),
        embeds::ban(get_player_interaction(lobby_id, 1), lobby_id,
                    player_one_ban),
        [=](const dpp::confirmation_callback_t &callback) {
          if (callback.is_error()) {
            creator->log(
                dpp::ll_error,
                fmt::format("Error: {}", callback.get_error().human_readable));
            return;
          }
          start_game_timer(lobby_id, creator->start_timer(
                                         [=](unsigned long t) {
                                           handle_timeout(lobby_id);
                                           creator->stop_timer(t);
                                         },
                                         GAME_TIMEOUT));
        });
    return;
  }

  if (player_two_ban != 0) {
    creator->direct_message_create(
        get_player_id(lobby_id, 0),
        embeds::ban(get_player_interaction(lobby_id, 0), lobby_id,
                    player_two_ban),
        [=](const dpp::confirmation_callback_t &callback) {
          if (callback.is_error()) {
            creator->log(
                dpp::ll_error,
                fmt::format("Error: {}", callback.get_error().human_readable));
            return;
          }
          start_game_timer(lobby_id, creator->start_timer(
                                         [=](unsigned long t) {
                                           handle_timeout(lobby_id);
                                           creator->stop_timer(t);
                                         },
                                         GAME_TIMEOUT));
        });
    return;
  }
}

void send_game_messages(const unsigned int lobby_id) {
  rps_lobby found_lobby = get_lobby(lobby_id);
  if (found_lobby.id == 0) {
    return;
  }
  unsigned int game_num = get_game_num(lobby_id);

  if (game_num == 1) {
    for (auto &player_info : found_lobby.players) {
      clear_queue_timer(creator, player_info->player.id);
    }
  }

  std::string player_one_name = get_player_name(lobby_id, 0);
  unsigned int player_one_score = get_player_score(lobby_id, 0);
  std::string player_two_name = get_player_name(lobby_id, 1);
  unsigned int player_two_score = get_player_score(lobby_id, 1);
  unsigned int first_to = get_first_to(lobby_id);

  start_game_timer(lobby_id, creator->start_timer(
                                 [=](unsigned long t) {
                                   handle_timeout(lobby_id);
                                   creator->stop_timer(t);
                                 },
                                 GAME_TIMEOUT));

  for (const auto &player_info : found_lobby.players) {
    creator->direct_message_create(
        player_info->player.id,
        embeds::game(player_info->init_interaction, lobby_id, game_num,
                     player_one_name, player_one_score, player_two_name,
                     player_two_score, first_to));
  }
}

void send_result_messages(const unsigned int lobby_id,
                          const unsigned int winner, const unsigned int loser,
                          bool draw) {
  unsigned int game_num = get_game_num(lobby_id);
  std::string player_one_name = get_player_name(lobby_id, 0);
  std::string player_one_choice = get_player_choice(get_player_id(lobby_id, 0));
  unsigned int player_one_score = get_player_score(lobby_id, 0);
  std::string player_two_name = get_player_name(lobby_id, 1);
  std::string player_two_choice = get_player_choice(get_player_id(lobby_id, 1));
  unsigned int player_two_score = get_player_score(lobby_id, 1);
  dpp::slashcommand_t winner_int = get_player_interaction(lobby_id, winner);
  dpp::slashcommand_t loser_int = get_player_interaction(lobby_id, loser);

  dpp::message msg_win = embeds::game_result(
      winner_int, game_num, player_one_name, player_one_choice, player_two_name,
      player_two_choice, "WIN");
  dpp::message msg_loss = embeds::game_result(
      loser_int, game_num, player_one_name, player_one_choice, player_two_name,
      player_two_choice, "LOSS");
  if (draw) {
    msg_win = embeds::game_result(winner_int, game_num, player_one_name,
                                  player_one_choice, player_two_name,
                                  player_two_choice, "DRAW");
    msg_loss = embeds::game_result(loser_int, game_num, player_one_name,
                                   player_one_choice, player_two_name,
                                   player_two_choice, "DRAW");
  }

  /* These need to be sent before the next game message is sent, so we make them
   * synchronous */
  creator->direct_message_create_sync(get_player_id(lobby_id, winner), msg_win);
  creator->direct_message_create_sync(get_player_id(lobby_id, loser), msg_loss);

  /* Hack to set player emoji */
  std::string player_one_emoji_choice =
      (player_one_choice == "Rock")
          ? ":rock:"
          : ((player_one_choice == "Paper") ? ":page_facing_up:"
                                            : ":scissors:");
  std::string player_two_emoji_choice =
      (player_two_choice == "Rock")
          ? ":rock:"
          : ((player_two_choice == "Paper") ? ":page_facing_up:"
                                            : ":scissors:");

  /* Create normal text message for result (may have a higher rate limit?) */
  dpp::message result_msg;
  if (draw) {
    result_msg = dpp::message(fmt::format(
        "__**Lobby #{} - Game {}**__\n{}  {}  {}  |  {}  {}  {}", lobby_id,
        game_num, player_one_name, player_one_emoji_choice, player_one_score,
        player_two_score, player_two_emoji_choice, player_two_name));
  } else {
    /* Determine which name + score to bold */
    if (winner == 0) {
      result_msg = dpp::message(fmt::format(
          "__**Lobby #{} - Game {}**__\n**{}**  {}  **{}**  |  {}  {}  {}",
          lobby_id, game_num, player_one_name, player_one_emoji_choice,
          player_one_score, player_two_score, player_two_emoji_choice,
          player_two_name));
    } else {
      result_msg = dpp::message(fmt::format(
          "__**Lobby #{} - Game {}**__\n{}  {}  {}  |  **{}**  {}  **{}**",
          lobby_id, game_num, player_one_name, player_one_emoji_choice,
          player_one_score, player_two_score, player_two_emoji_choice,
          player_two_name));
    }
  }

  /* Send results in channels that players queued in */
  dpp::slashcommand_t player_one_interaction =
      get_player_interaction(lobby_id, 0);
  dpp::slashcommand_t player_two_interaction =
      get_player_interaction(lobby_id, 1);

  /* Both of them were in DMs, so no work needed */
  if ((player_one_interaction.command.guild_id.empty() ||
       player_one_interaction.command.guild_id == 0) &&
      (player_two_interaction.command.guild_id.empty() ||
       player_two_interaction.command.guild_id == 0)) {
    return;
  }

  /* Just send one if they are the same */
  if (player_one_interaction.command.channel_id ==
      player_two_interaction.command.channel_id) {
    creator->message_create_sync(
        result_msg.set_guild_id(player_one_interaction.command.guild_id)
            .set_channel_id(player_one_interaction.command.channel_id));
    return;
  }

  if (!player_one_interaction.command.guild_id.empty() &&
      player_one_interaction.command.guild_id != 0) {
    creator->message_create_sync(
        result_msg.set_guild_id(player_one_interaction.command.guild_id)
            .set_channel_id(player_one_interaction.command.channel_id));
  }

  if (!player_two_interaction.command.guild_id.empty() &&
      player_two_interaction.command.guild_id != 0) {
    creator->message_create_sync(
        result_msg.set_guild_id(player_two_interaction.command.guild_id)
            .set_channel_id(player_two_interaction.command.message_id));
  }
}

void send_match_results(const unsigned int lobby_id, const dpp::user &winner,
                        bool double_afk) {
  dpp::slashcommand_t player_one_interaction =
      get_player_interaction(lobby_id, 0);
  dpp::slashcommand_t player_two_interaction =
      get_player_interaction(lobby_id, 1);

  dpp::message player_one_message = embeds::match_result(
      player_one_interaction, lobby_id, get_game_num(lobby_id),
      get_player_name(lobby_id, 0), get_player_score(lobby_id, 0),
      get_player_name(lobby_id, 1), get_player_score(lobby_id, 1), winner,
      double_afk);

  dpp::message player_two_message = embeds::match_result(
      player_two_interaction, lobby_id, get_game_num(lobby_id),
      get_player_name(lobby_id, 0), get_player_score(lobby_id, 0),
      get_player_name(lobby_id, 1), get_player_score(lobby_id, 1), winner,
      double_afk);

  creator->direct_message_create(get_player_id(lobby_id, 0),
                                 player_one_message);
  creator->direct_message_create(get_player_id(lobby_id, 1),
                                 player_two_message);

  /* Send results in channels that players queued in */
  dpp::message msg = embeds::match_result(
      dpp::interaction_create_t(), lobby_id, get_game_num(lobby_id),
      get_player_name(lobby_id, 0), get_player_score(lobby_id, 0),
      get_player_name(lobby_id, 1), get_player_score(lobby_id, 1), winner,
      double_afk);

  /* Both of them were in DMs, so no work needed */
  if ((player_one_interaction.command.guild_id.empty() ||
       player_one_interaction.command.guild_id == 0) &&
      (player_two_interaction.command.guild_id.empty() ||
       player_two_interaction.command.guild_id == 0)) {
    return;
  }

  /* Just send one if they are the same */
  if (player_one_interaction.command.channel_id ==
      player_two_interaction.command.channel_id) {
    creator->message_create(
        msg.set_guild_id(player_one_interaction.command.guild_id)
            .set_channel_id(player_one_interaction.command.channel_id));
    return;
  }

  if (!player_one_interaction.command.guild_id.empty() &&
      player_one_interaction.command.guild_id != 0) {
    creator->message_create(
        msg.set_guild_id(player_one_interaction.command.guild_id)
            .set_channel_id(player_one_interaction.command.channel_id));
    return;
  }

  if (!player_two_interaction.command.guild_id.empty() &&
      player_two_interaction.command.guild_id != 0) {
    creator->message_create(
        msg.set_guild_id(player_two_interaction.command.guild_id)
            .set_channel_id(player_two_interaction.command.message_id));
    return;
  }
}

void handle_choice(const dpp::button_click_t &event) {
  /* Find player lobby */
  unsigned int player_lobby_id =
      find_player_lobby_id(event.command.get_issuing_user().id);

  rps_lobby player_lobby = get_lobby(player_lobby_id);

  /* 1. Go set the choice, then send a confirmation message */
  clear_queue_timer(creator, event.command.get_issuing_user().id);
  set_player_choice(event.command.get_issuing_user().id, event.custom_id);
  creator->direct_message_create(
      event.command.get_issuing_user().id,
      dpp::message(fmt::format("You selected {}! {}", event.custom_id,
                               tr("E_WAITING", event))));

  /* 2. If both choices are selected, determine who won and increment winner
   */
  if (check_both_responses(player_lobby_id)) {
    clear_game_timer(creator, player_lobby_id);
    std::string result = determine_winner(player_lobby_id);
    dpp::user winner;
    if (result == "1") {
      winner = get_player_info(player_lobby_id, 0)->player;
      increment_player_score(player_lobby_id, 0);
      send_result_messages(player_lobby_id, 0, 1);
    } else if (result == "2") {
      winner = get_player_info(player_lobby_id, 1)->player;
      increment_player_score(player_lobby_id, 1);
      send_result_messages(player_lobby_id, 1, 0);
    } else if (result == "D") {
      send_result_messages(player_lobby_id, 0, 1, true);
    }

    if (is_game_complete(player_lobby_id)) {
      /* Finish up */
      send_match_results(player_lobby_id, winner);
      remove_lobby_from_queue(player_lobby_id, true);
    } else {
      /* Send message */
      increment_game_num(player_lobby_id);
      reset_choices(player_lobby_id);
      send_game_messages(player_lobby_id);
    }
  }
}

void handle_timeout(const unsigned int lobby_id) {
  std::string result = determine_winner(lobby_id);
  dpp::user winner;
  if (result == "1") {
    increment_player_score(lobby_id, 0);
    send_match_results(lobby_id, get_player_info(lobby_id, 0)->player);
  } else if (result == "2") {
    increment_player_score(lobby_id, 1);
    send_match_results(lobby_id, get_player_info(lobby_id, 1)->player);
  } else if (result == "FF") {
    send_match_results(lobby_id, winner, true);
  }

  remove_lobby_from_queue(lobby_id, true);
}

void handle_leave(const dpp::slashcommand_t &event,
                  const unsigned int lobby_id) {
  clear_queue_timer(event.from->creator, event.command.usr.id);
  remove_lobby_from_queue(lobby_id, false);
}

void handle_start(const unsigned int lobby_id) {}

} // namespace game_manager