[![Build](https://github.com/tarolling/rps-bot-cpp/actions/workflows/docker-image.yml/badge.svg)](https://github.com/tarolling/rps-bot-cpp/actions/workflows/docker-image.yml)
[![Code Security](https://github.com/tarolling/rps-bot-cpp/actions/workflows/codeql.yml/badge.svg)](https://github.com/tarolling/rps-bot-cpp/actions/workflows/codeql.yml)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/28b8cd2334a54aecb26c1386f877d169)](https://app.codacy.com/gh/tarolling/rps-bot-cpp/dashboard?utm_source=gh&utm_medium=referral&utm_content=&utm_campaign=Badge_grade)

# rps-bot-cpp

C++ version of Ranked RPS bot. Architecture directly inspired by:

- TriviaBot: <https://github.com/brainboxdotcc/triviabot>

## Invite To Your Server

<https://discord.com/oauth2/authorize?client_id=638351152131604521&scope=bot+applications.commands&permissions=939641936>

## Development

### Dependencies

| Name | Version |
| ---- | ------- |
| [CMake](https://cmake.org/) | 3.16.3+ |
| [D++](https://github.com/brainboxdotcc/DPP) | 10.0.29 |
| [fmtlib](https://github.com/fmtlib/fmt) | 8.1.1 |
| [spdlog](https://github.com/gabime/spdlog) | latest |

### Additional Instructions

Create a `config.json` file, and fill in the fields according to [the example file](example-config.json).
