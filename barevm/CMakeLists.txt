cmake_minimum_required(VERSION 2.6)

project(barevm)

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/tests_unit")
add_subdirectory(src)
add_subdirectory(tests_unit)

# build the main barevm executable
add_executable(barevm src/main.cpp)
target_link_libraries(barevm barevm_lib)

# build the unit tests
#add_definitions("-DGTEST_HAS_PTHREAD=0")
add_executable(barevm_unittest tests_unit/gtest/gtest_main.cc)
target_link_libraries(barevm_unittest barevm_lib unittest_lib pthread)


