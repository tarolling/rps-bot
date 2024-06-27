# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/f8d7d77c06936315286eb55f8de22cd23c188571.zip
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/domain test_src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/domain/commands test_src)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/../src/data_source test_src)
add_executable(
    unit_tests
    ${test_src}
    test_queue.cpp
)

target_include_directories(unit_tests
    PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/domain
    ${CMAKE_CURRENT_SOURCE_DIR}/../include/domain/commands
)

target_link_libraries(
    unit_tests PRIVATE
    gtest_main
    dpp
    fmt
    spdlog
    ${CMAKE_THREAD_LIBS_INIT}
)

include(GoogleTest)
gtest_discover_tests(unit_tests)