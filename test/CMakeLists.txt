cmake_minimum_required(VERSION 3.16)

set (TARGET_NAME test)

project(${TARGET_NAME} LANGUAGES CXX)

set(SRC_FILES iter_impl.cpp)

add_executable(${TARGET_NAME}  ${SRC_FILES})

target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES}  ${GDAL_LIBRARY} ${OpenCV_LIBS})
