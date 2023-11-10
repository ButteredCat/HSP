cmake_minimum_required(VERSION 3.16)

set (TARGET_NAME unittest)

file(GLOB SRC_FILES *.cpp *.cc)

add_executable(${TARGET_NAME} ${SRC_FILES})

target_link_libraries(${TARGET_NAME} PRIVATE
  GTest::gtest_main
  ${Boost_LIBRARIES}
  ${GDAL_LIBRARY}
  ${OpenCV_LIBS}
)

gtest_discover_tests(${TARGET_NAME})
