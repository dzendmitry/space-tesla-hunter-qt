cmake_minimum_required(VERSION 3.19)
project(space_tesla_hunter LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets Multimedia)

qt_standard_project_setup()

qt_add_executable(space_tesla_hunter
    WIN32 MACOSX_BUNDLE
    main.cpp
    spaceteslahunter.cpp
    spaceteslahunter.h
    spaceteslahunter.ui
    gamegraphicscene.h
    gamegraphicscene.cpp
    spaceteslahunter.cpp
    ship.h ship.cpp
    bullet.h bullet.cpp
    resources.h resources.cpp
    menugraphicsscene.h menugraphicsscene.cpp
    gameengine.h gameengine.cpp
)

qt_add_resources(space_tesla_hunter "images"
    PREFIX "/"
    FILES
        resources/images/menu/menu_background_2.png
        resources/images/menu/menu_button_1.png
        resources/images/menu/menu_button_2.png
        resources/images/menu/menu_button_3.png
        resources/images/victory.png
        resources/images/defeat.png
        resources/images/background.png
        resources/images/bullet_player_1.png
        resources/images/bullet_enemy_1.png
        resources/images/player/player.png
        resources/images/player/player_damaged_70.png
        resources/images/player/player_damaged_40.png
        resources/images/player/player_damaged_10.png
        resources/images/player/player_damaged_0.png
        resources/images/enemy_1/enemy_1.png
        resources/images/enemy_1/enemy_1_damaged_70.png
        resources/images/enemy_1/enemy_1_damaged_40.png
        resources/images/enemy_1/enemy_1_damaged_10.png
        resources/images/enemy_1/enemy_1_damaged_0.png
        resources/images/enemy_2/enemy_2.png
        resources/images/enemy_2/enemy_2_damaged_70.png
        resources/images/enemy_2/enemy_2_damaged_40.png
        resources/images/enemy_2/enemy_2_damaged_10.png
        resources/images/enemy_2/enemy_2_damaged_0.png
        resources/images/enemy_3/enemy_3.png
        resources/images/enemy_3/enemy_3_damaged_70.png
        resources/images/enemy_3/enemy_3_damaged_40.png
        resources/images/enemy_3/enemy_3_damaged_10.png
        resources/images/enemy_3/enemy_3_damaged_0.png

)

qt_add_resources(space_tesla_hunter "music"
    PREFIX "/"
    FILES
        resources/music/main.mp3
)

target_link_libraries(space_tesla_hunter
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt::Multimedia
)

install(TARGETS space_tesla_hunter
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET space_tesla_hunter
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
