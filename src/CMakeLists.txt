set(klaverjas_SRCS
    main.cpp
    klaverjas.cpp
    game.cpp
    rules.cpp
    card.cpp
    cardset.cpp
    trick.cpp
    player.cpp
    team.cpp
    cardimageprovider.cpp
    qml/klaverjas.qrc
)

kconfig_add_kcfg_files(klaverjas_SRCS klaverjasSettings.kcfgc)
ki18n_wrap_ui(klaverjas_SRCS settingsBase.ui)

add_executable(klaverjas ${klaverjas_SRCS})

target_link_libraries(klaverjas
    Qt5::Core
    Qt5::Widgets
    Qt5::QuickWidgets
    Qt5::Svg
    KF5::CoreAddons
    KF5::I18n
    KF5::XmlGui
    KF5::ConfigWidgets
)

install(TARGETS klaverjas ${INSTALL_TARGETS_DEFAULT_ARGS})
install(PROGRAMS org.example.klaverjas.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install(FILES org.example.klaverjas.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR})
install(FILES klaverjasui.rc DESTINATION ${KXMLGUI_INSTALL_DIR}/klaverjas)
