PROJECT(HIRAMEKI CXX)
cmake_minimum_required(VERSION 2.8)

set(CMAKI_REPOSITORY "http://artifacts.myftp.biz:8080")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmaki/cmaki.cmake")
cmaki_setup()

cmaki2_executable(kmeans kmeans.cpp)
cmaki_find_package(cryptopp)
cmaki2_gtest(encrypt encrypt.cpp)
