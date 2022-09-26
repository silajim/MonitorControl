cmake_minimum_required(VERSION 3.14)

project(MonitorControl LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Quick REQUIRED)
#find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Quick REQUIRED)
#find_package(Dxva2 REQUIRED)


add_library(MonitorControl STATIC "physicalmonitor.h" "physicalmonitor.cpp" monitorcontroller.h monitorcontroller.cpp)
target_link_libraries(MonitorControl PUBLIC  Dxva2 setupapi)
