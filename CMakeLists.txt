cmake_minimum_required(VERSION 3.16)

set(TARGET_NAME hsip)

project(${TARGET_NAME})

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

find_package(GDAL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Boost COMPONENTS filesystem program_options REQUIRED)
find_package(spdlog REQUIRED)

include_directories(
    ${GDAL_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    )

set(SRC_FILE
    main.cpp
  )

add_executable(${TARGET_NAME} ${SRC_FILE})

target_precompile_headers(${TARGET_NAME} PRIVATE pch.h)

target_link_libraries(${TARGET_NAME} PRIVATE ${GDAL_LIBRARY} ${Boost_LIBRARIES} ${OpenCV_LIBS} spdlog::spdlog)
