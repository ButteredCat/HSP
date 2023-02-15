cmake_minimum_required(VERSION 3.16)

project(new_tech)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GDAL REQUIRED)

include_directories(${GDAL_INCLUDE_DIR})

set(new_tech_src 
    main.cpp
)

add_executable(new_tech ${new_tech_src})

target_precompile_headers(new_tech PRIVATE pch.h)

target_link_directories(new_tech PRIVATE ${GDAL_LIBRARY})
