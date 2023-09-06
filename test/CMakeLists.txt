cmake_minimum_required(VERSION 3.16)

set (TARGET_NAME hsp_test)




add_executable(
  ${TARGET_NAME}
  iterator_test.cpp
)
target_link_libraries(
  ${TARGET_NAME}
  GTest::gtest_main
  ${Boost_LIBRARIES}  ${GDAL_LIBRARY} ${OpenCV_LIBS}
)


gtest_discover_tests(${TARGET_NAME})

