cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME hsp)

project(${TARGET_NAME})

# 强制让 vs 将源文件以 utf-8 格式解码
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
    add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")
endif()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

find_package(GDAL 2.3 REQUIRED)
find_package(OpenCV 4 REQUIRED)
find_package(Boost 1.71 COMPONENTS filesystem REQUIRED)
find_package(GTest)
find_package(OpenMP)

include_directories(
    ${GDAL_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
    hsp/
    )


add_subdirectory(samples)

if(GTEST_FOUND)
    enable_testing()
    include(CTest)
    include(GoogleTest)
    add_subdirectory(unittest)
endif(GTEST_FOUND)
