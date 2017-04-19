cmake_minimum_required(VERSION 2.8.4)
project(zkclient4cpp)

SET(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_STANDARD 11)

add_definitions(-DTHREADED)

#depend libzookeeper_mt.so
FIND_LIBRARY(ZOOKEEPER_MT zookeeper_mt /usr/local/lib/ NO_DEFAULT_PATH)
SET(ZOOKEEPER_MT_LIB ${ZOOKEEPER_MT_LIB} ${ZOOKEEPER_MT})

#depend spdlog(header only)

INCLUDE_DIRECTORIES(/usr/local/include include)

aux_source_directory(source SOURCE_FILE_LIST)
set(ZOOKEEPER_FILE_LIST ${SOURCE_FILE_LIST})
add_library(zkclient4cpp_static ${ZOOKEEPER_FILE_LIST})
add_library(zkclient4cpp SHARED ${ZOOKEEPER_FILE_LIST})

#make test
set(TEST_LINK_LIBRAIES ${ZOOKEEPER_MT_LIB} pthread)
set(ZOOKEEPER_FILES test/SyncApiTest.cpp ${ZOOKEEPER_FILE_LIST})
add_executable(SyncApiTest  ${ZOOKEEPER_FILES})
target_link_libraries(SyncApiTest ${TEST_LINK_LIBRAIES})

set(TEST_WATCH_FILES test/WatcherTest.cpp ${ZOOKEEPER_FILE_LIST})
add_executable(WatcherTest  ${TEST_WATCH_FILES})
target_link_libraries(WatcherTest ${TEST_LINK_LIBRAIES})

set(TEMP_NODE_FILES test/TempNodeTest.cpp ${ZOOKEEPER_FILE_LIST})
add_executable(TempNodeTest  ${TEMP_NODE_FILES})
target_link_libraries(TempNodeTest  ${TEST_LINK_LIBRAIES})

set(ASYNC_TEST_FILES test/AsyncApiTest.cpp ${ZOOKEEPER_FILE_LIST})
add_executable(AsyncApiTest  ${ASYNC_TEST_FILES})
target_link_libraries(AsyncApiTest  ${TEST_LINK_LIBRAIES})

