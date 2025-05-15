# Verify cmake version
cmake_minimum_required(VERSION 3.15)

# Set flags for coverage
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -fprofile-arcs -ftest-coverage")
#[[
# Database tests
add_executable(database_tests
  database_test.cpp
  ${CMAKE_SOURCE_DIR}/server/src/database.cpp
  ${CMAKE_SOURCE_DIR}/server/src/log.cpp
)

target_link_libraries(database_tests PostgreSQL::PostgreSQL gtest::gtest)

add_test(NAME database_tests COMMAND database_tests)
#[[
# Orders tests
add_executable(orders_tests
  orders_test.cpp
  ${CMAKE_SOURCE_DIR}/server/src/orders.cpp
  ${CMAKE_SOURCE_DIR}/server/src/log.cpp
  ${CMAKE_SOURCE_DIR}/server/src/sender.cpp
  ${CMAKE_SOURCE_DIR}/server/src/database.cpp
  ${CMAKE_SOURCE_DIR}/server/src/inventory.cpp
)

target_link_libraries(orders_tests PostgreSQL::PostgreSQL nlohmann_json::nlohmann_json gtest::gtest)

add_test(NAME orders_tests COMMAND orders_tests)
]]
# Authentication tests
#[[
add_executable(authentication_tests
  authentication_test.cpp
  ${CMAKE_SOURCE_DIR}/src/authentication.cpp
)
target_include_directories(auth_tests PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${gtest_SOURCE_DIR}/include
)
target_link_libraries(auth_tests
  gtest_main
)
add_test(NAME auth_tests COMMAND auth_tests)

]]

# Add tests
#[[
enable_testing()
add_executable(server_test
    src/authentication.cpp
    src/database.cpp
    src/inventory.cpp
    src/log.cpp
    src/orders.cpp
    src/request_router.cpp
    src/sender.cpp
    src/server.cpp
    tests/authentication_test.cpp
    tests/database_test.cpp
    tests/inventory_test.cpp
    tests/log_test.cpp
    tests/orders_test.cpp
    tests/request_router_test.cpp
    tests/sender_test.cpp
    tests/server_test.cpp
)

# Link test dependencies
target_link_libraries(server_tests PostgreSQL::PostgreSQL nlohmann_json::nlohmann_json gtest::gtest)
]]
