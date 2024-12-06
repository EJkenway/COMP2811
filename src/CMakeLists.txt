cmake_minimum_required(VERSION 3.16)

project(cwk2 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Sql Charts)
qt_standard_project_setup()

qt_add_executable(monitor
    main.cpp
    window.cpp
    window.hpp
    pages/Overview.cpp
    pages/Organic.cpp
    pages/Fluoride.cpp
    pages/Env_litter.cpp
    pages/Compliance.cpp
    utils/database_utils.cpp
)

target_link_libraries(monitor PRIVATE Qt6::Widgets Qt6::Sql Qt6::Charts)

configure_file(../database/pollution.db ${CMAKE_CURRENT_BINARY_DIR}/data.db COPYONLY)

set_target_properties(monitor PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE OFF
)
