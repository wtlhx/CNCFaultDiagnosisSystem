file(GLOB_RECURSE SOURCES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(HttpServer_shared SHARED ${SOURCES})