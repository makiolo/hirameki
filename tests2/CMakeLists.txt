cmake_minimum_required(VERSION 2.8)

include_directories(../asyncply)
find_package(verypoco REQUIRED)
find_package(Boost COMPONENTS graph coroutine context filesystem system regex REQUIRED)
IF(Boost_FOUND)
	include_directories(SYSTEM ${Boost_INCLUDE_DIRS} /usr/include)
	CREATE_TEST(coro "multithread")
	target_link_libraries(coro ${Boost_LIBRARIES} /usr/lib/x86_64-linux-gnu/libssh.so)
endif()

