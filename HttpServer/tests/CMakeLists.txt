file(GLOB TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/*.cpp)
#找到
find_package(jsoncpp CONFIG REQUIRED)
find_library(MYSQL_CPPCONN_LIB mysqlcppconn)
find_package(CURL REQUIRED)

#把neo4jDriver编译成静态库
file(GLOB_RECURSE NEO4JDriver /usr/include/Neo4j-cpp-driver-master/*.cpp)
add_library(neo4jDriver_static STATIC ${NEO4JDriver})
#链接到curl库
target_link_libraries(neo4jDriver_static PRIVATE CURL::libcurl)

foreach(test_source ${TEST_SOURCES})

    get_filename_component(test_NAME ${test_source} NAME)
    string(REPLACE ".cpp" "" test_NAME ${test_NAME})
    add_executable(${test_NAME} EXCLUDE_FROM_ALL ${test_source})
    #链接源文件动态库
    target_link_libraries(${test_NAME} PRIVATE HttpServer_shared)
    #链接到muduo库
    target_link_libraries(${test_NAME} PRIVATE muduo_net muduo_base)
    #链接json库
    if (jsoncpp_FOUND)
    target_link_libraries(${test_NAME} PRIVATE jsoncpp_lib)
    else ()
    message(FATAL_ERROR "jsoncpp library not found.")
    endif()

    #链接到neo4jDriver
    target_link_libraries(${test_NAME} PRIVATE neo4jDriver_static)
    #链接mysqlcppconn库
    target_link_libraries(${test_NAME} PRIVATE ${MYSQL_CPPCONN_LIB})  

    set_target_properties(${test_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        COMMAND ${test_NAME})

endforeach(test_source ${TEST_SOURCES})
