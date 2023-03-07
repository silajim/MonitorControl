cmake_minimum_required(VERSION 3.14)

project(MonitorControl LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core REQUIRED)
#find_package(Dxva2 REQUIRED)


add_library(MonitorControl STATIC primarymonitor.cpp primarymonitor.h monitorcontroller.h monitorcontroller.cpp WindowsErrorException.h WindowsErrorException.cpp)

target_include_directories(MonitorControl PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(MonitorControl PUBLIC Qt6::Core Dxva2 setupapi )
