cmake_minimum_required(VERSION 3.16)

set (TARGET_NAME hsp)

project(${TARGET_NAME} LANGUAGES CXX)

include(FetchContent)

FetchContent_Declare(cmake_git_version_tracking                   
  GIT_REPOSITORY https://github.com/andrew-hardin/cmake-git-version-tracking.git
  GIT_TAG 904dbda1336ba4b9a1415a68d5f203f576b696bb
)

FetchContent_MakeAvailable(cmake_git_version_tracking)

find_package(spdlog REQUIRED)

set(SRC_FILES main.cpp)

add_executable(${TARGET_NAME} ${SRC_FILES})

include(CMakePrintHelpers)

cmake_print_properties(
TARGETS
opencv_core cmake_git_version_tracking
PROPERTIES
INTERFACE_COMPILE_OPTIONS
INTERFACE_INCLUDE_DIRECTORIES
INTERFACE_LINK_LIBRARIES
)

target_link_libraries(${TARGET_NAME} ${Boost_LIBRARIES} ${GDAL_LIBRARY} ${OpenCV_LIBS} spdlog::spdlog cmake_git_version_tracking)
