cmake_minimum_required(VERSION 3.21)

project(YukonSolitaire C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(yukon_core
    src/core/card.c
    src/core/game.c
    src/core/list.c
    src/core/deck_io.c
    src/core/shuffle.c
    src/core/game_setup.c
    src/core/move.c
)

target_include_directories(yukon_core PUBLIC include)
target_compile_options(yukon_core PRIVATE -Wall -Wextra -Wpedantic)

add_executable(yukon_cli
    src/cli/main.c
    src/cli/render.c
    src/cli/parser.c
)

target_link_libraries(yukon_cli PRIVATE yukon_core)
