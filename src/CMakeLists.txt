set(klaverjas_SRCS
    main.cpp
    gameengine.cpp
    game.cpp
    card.cpp
    cardset.cpp
    trick.cpp
    team.cpp
    cardimageprovider.cpp
    aitest.cpp
    players/player.cpp
    players/humanplayer.cpp
    players/aiplayer.cpp
    players/randomplayer.cpp
    qml/klaverjas.qrc
    scores.h
)

add_executable(klaverjas ${klaverjas_SRCS})

target_link_libraries(klaverjas
    Qt5::Core
    Qt5::Widgets
    Qt5::QuickWidgets
    Qt5::Quick
    Qt5::Svg
    Qt5::Concurrent
)

install(TARGETS klaverjas ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.example.klaverjas.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES org.example.klaverjas.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
