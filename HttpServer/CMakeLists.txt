cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(BUILD_SHARED_LIBS ON)
project(HttpServer)

include_directories(${PROJECT_SOURCE_DIR}/include/http)
include_directories(${PROJECT_SOURCE_DIR}/include/router)
include_directories(${PROJECT_SOURCE_DIR}/include/Database)

add_subdirectory(src)
add_subdirectory(tests)
