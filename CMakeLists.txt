cmake_minimum_required (VERSION 3.7.2)
project(pfsd_test LANGUAGES CXX C)

include_directories(/usr/local/polarstore/pfsd/include)
link_directories(/usr/local/polarstore/pfsd/lib)

add_executable(pfsd_test pfsd_test.cc)
target_link_libraries(pfsd_test pfsd pthread)

add_executable(pfsd_test2 pfsd_test2.cc)
target_link_libraries(pfsd_test2 pfsd readline pthread)
