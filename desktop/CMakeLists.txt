cmake_minimum_required(VERSION 3.21)
project(OpenBandDesktop VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 6.4 REQUIRED COMPONENTS Widgets Network)

qt_add_executable(OpenBand
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
)

target_link_libraries(OpenBand PRIVATE Qt6::Widgets Qt6::Network)
target_include_directories(OpenBand PRIVATE src)

if(WIN32)
    target_compile_definitions(OpenBand PRIVATE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
