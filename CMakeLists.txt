cmake_minimum_required (VERSION 2.6)
project (FXBattle)

file(GLOB test_SRC
    "test/*.h"
    "test/*.cpp"
)

include_directories("exchange")
include_directories(".")

add_executable(Test.exe ${test_SRC})
