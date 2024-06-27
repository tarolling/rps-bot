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

#include <dpp/cluster.h>
#include <dpp/dispatcher.h>
#include <dpp/user.h>
#include <gtest/gtest.h>
#include <rps/domain/commandline.h>
#include <rps/domain/commands/queue.h>
#include <rps/domain/config.h>
#include <rps/domain/game.h>
#include <rps/domain/lang.h>
#include <rps/domain/listeners.h>
#include <rps/domain/logger.h>

class QueueTest : public testing::Test {
protected:
  static void SetUpTestSuite() {
    (void)std::setlocale(LC_ALL, "en_US.UTF-8");

    config::init("../config.json");
    logger::init(config::get("log"));

    bot = new dpp::cluster(config::get("dev_token"), dpp::i_guilds,
                           config::get("shards"), 0, 1, true,
                           dpp::cache_policy::cpol_none);

    i18n::load_lang(*bot);

    bot->set_websocket_protocol(dpp::ws_etf);

    bot->on_log(&logger::log);
    bot->on_slashcommand(&listeners::on_slashcommand);
    bot->on_button_click(&listeners::on_buttonclick);
    bot->on_ready(&listeners::on_ready);

    /* Initialize game state */
    game::init(*bot);

    /* Start bot */
    bot->start(dpp::st_wait);
  }

  static void TearDownTestSuite() {
    delete bot;
    bot = nullptr;
  }

  static dpp::cluster *bot;
};

dpp::cluster *QueueTest::bot = nullptr;

TEST_F(QueueTest, BasicAssertions) {
  EXPECT_STRNE("yo", "waddup");
  EXPECT_EQ(7 * 6, 42);
}

TEST_F(QueueTest, HeavyLoad) {
  dpp::slashcommand_t test_interaction{};
  test_interaction.command.usr = dpp::user{};
  test_interaction.command.usr.id = 0;

  queue_command::route(test_interaction);
  EXPECT_STRNE("yo", "waddup");
  EXPECT_EQ(7 * 6, 42);
}