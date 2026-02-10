add_library(game_lib STATIC)

file(GLOB_RECURSE GAME_SOURCES
        CONFIGURE_DEPENDS
        primitive_game/src/*.cpp
)

target_sources(game_lib PRIVATE
        ${GAME_SOURCES}
)

target_include_directories(game_lib PUBLIC
        primitive_game/include
)

add_executable(game_app
        primitive_game/bin/main.cpp
)

target_link_libraries(game_app
        PRIVATE game_lib
)

add_subdirectory(primitive_game/tests)
