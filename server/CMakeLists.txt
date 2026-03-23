find_package(Boost REQUIRED)
find_package(PostgreSQL REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_check_modules(HIREDIS REQUIRED IMPORTED_TARGET hiredis)

add_library(server_lib STATIC)

file(GLOB_RECURSE SERVER_SOURCES
        CONFIGURE_DEPENDS
        src/*.cpp
)

target_sources(server_lib PRIVATE
        ${SERVER_SOURCES}
)

target_include_directories(server_lib PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(server_lib PUBLIC
        game_lib
        Boost::boost
        PostgreSQL::PostgreSQL
        PkgConfig::HIREDIS
)

target_compile_features(server_lib PUBLIC cxx_std_20)

add_executable(miniciv_server
        src/main.cpp
)

target_link_libraries(miniciv_server PRIVATE
        server_lib
)