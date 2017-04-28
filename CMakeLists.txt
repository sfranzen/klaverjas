project(klaverjas)

cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)
set(QT_MIN_VERSION "5.7.0")

find_package(ECM 1.0.0 REQUIRED NO_MODULE)
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(FeatureSummary)

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS
    Core
    Gui
    Widgets
    QuickWidgets
    Quick
    Svg
    Concurrent
)

find_package(KF5 REQUIRED COMPONENTS
    CoreAddons
    I18n
    XmlGui
    ConfigWidgets
    DocTools
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

add_subdirectory(doc)
add_subdirectory(src)
add_subdirectory(icons)
