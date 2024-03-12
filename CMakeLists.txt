cmake_minimum_required(VERSION 3.22.1)

project(RPS-BOT
    VERSION 1.0.0
    DESCRIPTION "New Ranked RPS Discord bot"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")

# include_directories(include/)
add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_include_directories(
    ${PROJECT_NAME} PRIVATE
    DPP
)