cmake_minimum_required(VERSION 3.10)
project(a2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
        /usr/local/include
)

link_directories(
        /usr/local/lib
)

file(GLOB_RECURSE mains RELATIVE "${CMAKE_CURRENT_SOURCE_DIR}" bwtdecode.cpp bwtsearch.cpp)

foreach(mainfile IN LISTS mains)
    get_filename_component(mainname ${mainfile} NAME_WE)
    add_executable(${mainname} ${mainfile})
    set_property(TARGET ${mainname} PROPERTY CXX_STANDARD 17)
endforeach()
