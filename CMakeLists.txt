cmake_minimum_required (VERSION 2.6)
project (FXBattle)

find_package(Threads)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++17 -pedantic -Wextra")
find_package(Boost 1.52 COMPONENTS system thread REQUIRED)

# includes
include_directories(".")
include_directories("exchange")
include_directories("fxbattle")

# sources
file(GLOB test_SRC
    "test/*.h"
    "test/*.cpp"
)

file(GLOB fxbattle_SRC
    "fxbattle/*.h"
    "fxbattle/*.cpp"
)

# test
add_executable(Test.exe ${test_SRC})

# service
add_executable(FXBattle.exe ${fxbattle_SRC})
target_link_libraries(FXBattle.exe ${Boost_LIBRARIES})
target_link_libraries(FXBattle.exe ${CMAKE_THREAD_LIBS_INIT})

# static files
add_custom_command(OUTPUT index.html
    COMMAND ${CMAKE_COMMAND} -E
    copy ${PROJECT_SOURCE_DIR}/templates/index.html ${CMAKE_CURRENT_BINARY_DIR}/templates/index.html
    DEPENDS ${PROJECT_SOURCE_DIR}/templates/index.html
)
add_custom_target(static ALL DEPENDS index.html)
