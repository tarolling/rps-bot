cmake_minimum_required(VERSION 3.22.1)

project(RPS-BOT
    VERSION 1.0.0
    DESCRIPTION "New Ranked RPS Discord bot"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(BEFORE include/)

add_executable(${PROJECT_NAME}
    src/init.cpp
)

target_link_libraries(${PROJECT_NAME}
    dpp
)