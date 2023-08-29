cmake_minimum_required(VERSION 3.22.1)

project(RPS-BOT
    VERSION 1.0.0
    DESCRIPTION "New Ranked RPS Discord bot"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")

include_directories(BEFORE include/)

find_package(dpp)

if(NOT dpp_FOUND)
    message(STATUS "Fetching D++ from GitHub...")

    include(FetchContent)
    FetchContent_Declare(dpp
        GIT_REPOSITORY https://github.com/brainboxdotcc/DPP
        GIT_TAG v10.0.24
    )
    FetchContent_MakeAvailable(dpp)
endif()

add_executable(${PROJECT_NAME}
    src/main.cpp
)

target_link_libraries(${PROJECT_NAME}
    dpp
)