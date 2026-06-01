project(frontend LANGUAGES CXX)

find_package(Qt6 REQUIRED COMPONENTS Core Quick Network)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# =============================================================================
# Основной клиент
# =============================================================================
qt_add_executable(miniciv_client
        main.cpp
        GameClient/GameClient.cpp
        MapRender/MapRenderer.cpp
        qml/resources.qrc
)

target_link_libraries(miniciv_client PRIVATE
        Qt6::Core
        Qt6::Quick
        Qt6::Network
)

target_include_directories(miniciv_client PRIVATE
        ${CMAKE_SOURCE_DIR}/backend/primitive_game/include
        ${CMAKE_SOURCE_DIR}/server/include
)

# =============================================================================
# Тесты (исправленные пути)
# =============================================================================
find_package(Qt6 REQUIRED COMPONENTS Test)
enable_testing()

# 🔥 Явно указываем пути через ${CMAKE_CURRENT_SOURCE_DIR}
set(TEST_SOURCES
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_gameclient.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/GameClient/GameClient.cpp
)

# 🔥 Используем add_executable вместо qt_add_executable для тестов
# (надёжнее для консольных утилит, не требует QML/RCC)
add_executable(miniciv_client_tests ${TEST_SOURCES})

target_link_libraries(miniciv_client_tests PRIVATE
        Qt6::Core
        Qt6::Network
        Qt6::Test
)

target_include_directories(miniciv_client_tests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/backend/primitive_game/include
        ${CMAKE_SOURCE_DIR}/server/include
)

# 🔥 Важно для MOC: явно указываем, что это тестовый класс с Q_OBJECT
set_target_properties(miniciv_client_tests PROPERTIES
        AUTOMOC ON
        AUTOMOC_MOC_OPTIONS -DBUILDING_TESTS
)

add_test(NAME ClientTests COMMAND miniciv_client_tests)