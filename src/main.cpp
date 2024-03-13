#include <dpp/commandhandler.h>
#include <dpp/dpp.h>

#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <unistd.h>
#include <utility>
namespace fs = std::filesystem;

fs::path find_directory(const char *cur_dir, const char *target_dir) {
  fs::path search_dir = cur_dir;

  if (!fs::exists(search_dir)) {
    std::cerr << "Directory " << search_dir << " does not exist."
              << "\n";
  } else {
    for (const auto &entry : fs::directory_iterator(search_dir)) {
      if (fs::is_directory(entry)) {
        if (entry.path().filename() == target_dir) {
          std::cout << "Found directory: " << entry.path() << "\n";
          return entry.path();
        }
      }
    }
  }
  return {};
}

int main(int argc, char **argv) {
  /* Get bot token from .env file */
  char *bot_token = getenv("BOT_TOKEN");
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
    /* Find commands directory */
    fs::path command_dir = find_directory(".", "commands");
    if (command_dir.empty()) {
      return;
    }
    std::cout << command_dir.string() << "\n";
    command_handler.add_command(
        /* Command name*/
        "queue",
        /* Parameters */
        {{"minutes",
          dpp::param_info(dpp::pt_integer, true, "Optional test parameter")}},
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
              std::move(src));
        },
        /* Command description */
        "A test ping command",
        /* Guild id (omit for a global command)*/
        845688390292996156);

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
              std::move(src));
        },
        /* Command description */
        "A test ping command",
        /* Guild id (omit for a global command)*/
        845688390292996156);

    command_handler.register_commands();
  });

  bot.start(dpp::st_wait != 0U);

  return EXIT_SUCCESS;
}

void setup() {}