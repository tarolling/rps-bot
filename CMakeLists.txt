cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(BOT_NAME rps)

project(${BOT_NAME}
    VERSION 1.0.0
    DESCRIPTION "New Ranked RPS League"
    LANGUAGES CXX
)

aux_source_directory(src/discord bot_src)
aux_source_directory(src/discord/commands bot_src)
add_executable(${BOT_NAME} ${bot_src})

string(ASCII 27 Esc)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set_target_properties(${BOT_NAME} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)

set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(DPP REQUIRED CONFIG)
find_package(OpenSSL REQUIRED)

# TODO: Remove manually-set DPP include and library directories
set(DPP_INCLUDE_DIR "/usr/include/dpp")
set(DPP_LIBRARIES "/usr/local/lib/libdpp.so")

set(CMAKE_CXX_FLAGS "-g -O2 -rdynamic -Wall -Wno-psabi -Wempty-body -Wignored-qualifiers -Wimplicit-fallthrough -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Wuninitialized -Wshift-negative-value")
target_compile_definitions(${BOT_NAME} PUBLIC DPP_CORO=ON)

target_include_directories(${BOT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${DPP_INCLUDE_DIR}
)

target_link_libraries(${BOT_NAME} PUBLIC
    fmt
    spdlog
    ${CMAKE_THREAD_LIBS_INIT}
    ${DPP_LIBRARIES}
)

# # Print out some diagnostic information
# message(STATUS "DPP include directory: ${DPP_INCLUDE_DIR}")
# message(STATUS "DPP libraries: ${DPP_LIBRARIES}")
# message(STATUS "CMake thread libs init: ${CMAKE_THREAD_LIBS_INIT}")

# # Optionally, add debugging info
# if(CMAKE_BUILD_TYPE STREQUAL "Debug")
# message(STATUS "Debug build: Adding debug information.")
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
# endif()