#include <dpp/dpp.h>

#include <iostream>

int main(int argc, char **argv) {
  /* Get bot token from .env file */
  char *bot_token = std::getenv("BOT_TOKEN");
  if (bot_token == nullptr) {
    std::cerr << "Bot token not found."
              << "\n";
    return EXIT_FAILURE;
  }

  /* Instantiate bot cluster w/ default intents */
  dpp::cluster bot(bot_token, dpp::i_default_intents);

  /* Instantiate logger */
  bot.on_log(dpp::utility::cout_logger());

  /* Command handler */
  dpp::commandhandler command_handler(&bot);
  /* Specifying a prefix of "/" tells the command handler it should also expect
   * slash commands */
  command_handler.add_prefix("/").add_prefix("%");

  bot.on_ready([&command_handler](const dpp::ready_t &event) {
    command_handler.add_command(
        /* Command name*/
        "ping",
        /* Parameters */
        {{"testparam",
          dpp::param_info(dpp::pt_string, true, "Optional test parameter")}},
        /* Command handler */
        [&command_handler](const std::string &command,
                           const dpp::parameter_list_t &parameters,
                           dpp::command_source src) {
          std::string got_param;
          if (!parameters.empty()) {
            got_param = std::get<std::string>(parameters[0].second);
          }
          command_handler.reply(
              dpp::message("PONSGD !!! (now being built in C++) ==> " +
                           got_param),
              src);
        },
        /* Command description */
        "A test ping command",
        /* Guild id (omit for a global command)*/
        845688390292996156);

    command_handler.register_commands();
  });

  bot.start(dpp::st_wait);

  return EXIT_SUCCESS;
}

void setup() {}