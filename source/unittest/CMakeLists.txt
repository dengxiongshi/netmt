cmake_minimum_required(VERSION 3.16)
project(unittest)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 11)

# add_definitions(-DHV_SOURCE=1)
include_directories(${PROJECT_SOURCE_DIR}/include/hv)

enable_testing()

add_executable(ping ping_test.cpp)

# target_link_libraries(ping ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/Client/ping.h)
add_library(hv STATIC IMPORTED)
# set_property(GLOBAL ping hv PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/lib/libhv.a)
target_link_libraries(ping ${PROJECT_SOURCE_DIR}/lib/libhv.a)