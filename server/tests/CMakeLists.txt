# Verify cmake version
cmake_minimum_required(VERSION 3.15)

# Set flags for coverage
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -fprofile-arcs -ftest-coverage")

#[[
# Orders tests
add_executable(orders_server_test
  orders_test.cpp
  ${CMAKE_SOURCE_DIR}/server/src/orders.cpp
  ${CMAKE_SOURCE_DIR}/server/src/log.cpp
  ${CMAKE_SOURCE_DIR}/server/src/database.cpp
  ${CMAKE_SOURCE_DIR}/server/src/sender.cpp
  ${CMAKE_SOURCE_DIR}/server/src/inventory.cpp
)

target_link_libraries(orders_server_test PostgreSQL::PostgreSQL gtest::gtest nlohmann_json::nlohmann_json)

add_test(NAME orders_server_test COMMAND orders_server_test)
]]

# Request Router tests
add_executable(request_router_test
  request_router_test.cpp
  ${CMAKE_SOURCE_DIR}/server/src/orders.cpp
  ${CMAKE_SOURCE_DIR}/server/src/authentication.cpp
  ${CMAKE_SOURCE_DIR}/server/src/inventory.cpp
  ${CMAKE_SOURCE_DIR}/server/src/log.cpp
  ${CMAKE_SOURCE_DIR}/server/src/request_router.cpp
  ${CMAKE_SOURCE_DIR}/server/src/database.cpp
  ${CMAKE_SOURCE_DIR}/server/src/sender.cpp
)

target_link_libraries(request_router_test PostgreSQL::PostgreSQL gtest::gtest nlohmann_json::nlohmann_json)

add_test(NAME request_router_test COMMAND request_router_test)
