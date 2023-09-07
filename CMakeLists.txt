cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME hsp)

project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

find_package(GDAL 2.3 REQUIRED)
find_package(OpenCV 4 REQUIRED)
find_package(Boost COMPONENTS filesystem program_options REQUIRED)
find_package(GTest)
find_package(spdlog)

include_directories(
    ${GDAL_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    include/
    src/
    )


add_subdirectory(src)
add_subdirectory(app)

if(GTEST_FOUND)
enable_testing()
include(CTest)
include(GoogleTest)
add_subdirectory(test)
endif(GTEST_FOUND)
