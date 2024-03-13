cmake_minimum_required(VERSION 3.22.1)

project(rps_bot
    VERSION 1.0.0
    DESCRIPTION "New Ranked RPS Discord bot"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Release")

include_directories(include/)

# Set up DPP

# Set up project
add_subdirectory(src)
