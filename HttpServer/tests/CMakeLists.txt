file(GLOB TEST_SOURCES ${PROJECT_SOURCE_DIR}/tests/*.cpp)

foreach(test_source ${TEST_SOURCES})

    get_filename_component(test_NAME ${test_source} NAME)
    string(REPLACE ".cpp" "" test_NAME ${test_NAME})
    add_executable(${test_NAME} EXCLUDE_FROM_ALL ${test_source})
    #链接源文件动态库
    target_link_libraries(${test_NAME} PRIVATE HttpServer_shared)
    #链接到muduo库
    target_link_libraries(${test_NAME} PRIVATE muduo_net muduo_base)
    set_target_properties(${test_NAME} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        COMMAND ${test_NAME})
endforeach(test_source ${TEST_SOURCES})
