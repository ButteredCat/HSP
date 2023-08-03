cmake_minimum_required(VERSION 3.16)

set (TARGET_NAME hycodec)

project(${TARGET_NAME} LANGUAGES CXX)

file(GLOB SRC_FILES *.cpp)

add_library(${TARGET_NAME}  ${SRC_FILES})

target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES} ${GDAL_LIBRARY})
