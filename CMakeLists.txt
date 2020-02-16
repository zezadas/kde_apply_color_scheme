project(kde_colors_apply)
set(PROJECT_VERSION "5.18.80")

cmake_minimum_required(VERSION 3.10)

add_executable(colorscheme colorscheme.cpp krdb.cpp)

target_sources(colorscheme
PRIVATE
)

find_package(KF5DBusAddons REQUIRED)
find_package(KF5KCMUtils)
find_package(KF5GuiAddons)
find_package(KF5ConfigWidgets)

target_link_libraries(colorscheme
  PRIVATE
    KF5::KCMUtils
    KF5::DBusAddons
    KF5::GuiAddons
    KF5::ConfigWidgets
)
