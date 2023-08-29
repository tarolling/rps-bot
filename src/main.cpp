#include <dpp/dpp.h>

#include <iostream>

int main(int argc, char **argv) {
  char *bot_token = std::getenv("BOT_TOKEN");
  if (bot_token == NULL) {
    std::cerr << "Bot token not found."
              << "\n";
    return EXIT_FAILURE;
  }
  dpp::cluster bot(bot_token, dpp::i_default_intents);
  bot.on_log(dpp::utility::cout_logger());

  bot.on_slashcommand([](const dpp::slashcommand_t &event) {
    if (event.command.get_command_name() == "ping") {
      event.reply("PONSGD !!! (now being built in C++)");
    }
  });

  bot.on_ready([&bot](const dpp::ready_t &event) {
    if (dpp::run_once<struct register_bot_commands>()) {
      bot.global_command_create(
          dpp::slashcommand("ping", "Ping pong!!", bot.me.id));
    }
  });

  bot.start(dpp::st_wait);
}

void setup() {}