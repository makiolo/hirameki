cmake_minimum_required(VERSION 2.8)

include_directories(../asyncply)
find_package(verypoco REQUIRED)
CREATE_TEST(test_depends "")
target_link_libraries(test_depends verypoco multithread)

