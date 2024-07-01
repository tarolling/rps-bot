cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
find_package(MySQL REQUIRED)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
set(BOT_NAME rps)

project(${BOT_NAME}
    VERSION 1.0.0
    DESCRIPTION "Ranked RPS Bot"
    LANGUAGES CXX
)

aux_source_directory(src/presentation bot_src)
aux_source_directory(src/domain bot_src)
aux_source_directory(src/domain/commands bot_src)
aux_source_directory(src/data_source bot_src)
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
find_package(CURL REQUIRED)

set(CMAKE_CXX_FLAGS "-g -O2 -rdynamic -Wall -Wno-psabi -Wempty-body -Wignored-qualifiers -Wimplicit-fallthrough -Wmissing-field-initializers -Wsign-compare -Wtype-limits -Wuninitialized -Wshift-negative-value")
target_compile_definitions(${BOT_NAME} PUBLIC _GNU_SOURCE)

target_include_directories(${BOT_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${BOT_NAME} PUBLIC
    dpp
    fmt
    spdlog
    mysqlcppconn
    ${CMAKE_THREAD_LIBS_INIT}
    ${CURL_LIBRARIES}
)