set(MAIN_DRC "${PROJECT_SOURCE_DIR}/FaultDiagnosisServer/main/main.cpp")

#找到库
find_package(jsoncpp CONFIG REQUIRED)
find_library(MYSQL_CPPCONN_LIB mysqlcppconn)
find_package(CURL REQUIRED)

#把neo4jDriver编译成静态库
file(GLOB_RECURSE NEO4JDriver /usr/include/Neo4j-cpp-driver-master/*.cpp)
add_library(neo4jDriver_static STATIC ${NEO4JDriver})
#链接到curl库
target_link_libraries(neo4jDriver_static PRIVATE CURL::libcurl)

#添加可执行文件
add_executable(FDChatServer EXCLUDE_FROM_ALL ${MAIN_DRC})

#链接源文件动态库
#注意链接的顺序：被依赖的库要写在依赖它的库后面，A依赖B，B要在A后面依赖
#FaultDiagnosisServer_shared依赖于HttpServer_shared，所以HttpServer_shared要在后面依赖
target_link_libraries(FDChatServer PRIVATE FaultDiagnosisServer_shared) 
target_link_libraries(FDChatServer PRIVATE HttpServer_shared)
#链接到muduo库
target_link_libraries(FDChatServer PRIVATE muduo_net muduo_base)
target_link_libraries(FDChatServer PRIVATE jsoncpp_lib)
#链接到neo4jDriver
target_link_libraries(FDChatServer PRIVATE neo4jDriver_static)
#链接mysqlcppconn库
target_link_libraries(FDChatServer PRIVATE ${MYSQL_CPPCONN_LIB})

set_target_properties(FDChatServer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    COMMAND FDChatServer)