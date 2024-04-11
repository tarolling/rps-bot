cmake_minimum_required(VERSION 3.16.3)

project(rps
    VERSION 1.0.0
    DESCRIPTION "New Ranked RPS League"
    LANGUAGES CXX
)

aux_source_directory(src/discord bot_src)
add_executable(discord_bot ${bot_src})

aux_source_directory(src/domain server_src)
add_executable(server ${server_src})

include_directories(include)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

target_link_libraries(discord_bot dl dpp fmt spdlog)
target_link_libraries(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -pthread -g -O2 -fPIC -rdynamic -DFMT_HEADER_ONLY -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O2")

# Modules
set(modules_dir "modules")
file(GLOB subdirlist ${modules_dir}/*)

foreach(fullmodname ${subdirlist})
    get_filename_component(modname ${fullmodname} NAME)
    message(STATUS "Found module '${Esc}[1;34mmodule_${modname}${Esc}[m'")
    set(modsrc "")
    aux_source_directory(${modules_dir}/${modname} modsrc)
    add_library(module_${modname} SHARED ${modsrc})
    set_target_properties(module_${modname} PROPERTIES PREFIX "")
endforeach(fullmodname)