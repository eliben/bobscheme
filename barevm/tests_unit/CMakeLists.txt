add_definitions("-DGTEST_HAS_PTHRED=0")
set(unittest_lib_src 
    unittest_basicobjects.cpp
    unittest_environment.cpp
    unittest_example.cpp
    gtest/gtest-all.cc
    )

add_library(unittest_lib ${unittest_lib_src})

