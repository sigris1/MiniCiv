file(GLOB_RECURSE GAME_SOURCES
        CONFIGURE_DEPENDS
        primitive_game/src/*.cpp
)

add_library(game_lib STATIC ${GAME_SOURCES})

target_include_directories(game_lib
        PUBLIC primitive_game/include
)

add_executable(game_app
        primitive_game/bin/main.cpp
)

target_link_libraries(game_app
        PRIVATE game_lib
)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
