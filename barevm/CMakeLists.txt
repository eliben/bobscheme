cmake_minimum_required(VERSION 2.6)

project(barevm)

include_directories("${PROJECT_SOURCE_DIR}/src")
include_directories("${PROJECT_SOURCE_DIR}/tests_unit")
add_subdirectory(src)

# build the main barevm executable
add_executable(barevm src/main.cpp)
target_link_libraries(barevm barevm_lib)

# build the unit tests
#add_definitions("-DGTEST_HAS_PTHREAD=0")
add_executable(barevm_unittest
    tests_unit/unittest_basicobjects.cpp
    tests_unit/unittest_environment.cpp
    tests_unit/unittest_example.cpp
    tests_unit/gtest/gtest-all.cc
    tests_unit/gtest/gtest_main.cc
    )
  
if(WIN32)
  set(extra_libs "")
else()
  set(extra_libs pthread)
endif(WIN32)

target_link_libraries(barevm_unittest barevm_lib ${extra_libs})


