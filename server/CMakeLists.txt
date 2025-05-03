# Verify cmake version
cmake_minimum_required(VERSION 3.15)

# Enable code coverage
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Create project
project (server VERSION 1.0.0 DESCRIPTION "server" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O2")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -Os")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter")

# Include headers
include_directories(include)

# Add the source files
add_executable(server
    src/main.cpp
    src/server.cpp
    src/authentication.cpp
    src/inventory.cpp
    src/request_router.cpp
    src/orders.cpp
    src/log.cpp
    src/database.cpp
    src/sender.cpp
)

# Find dependencies en Conan
find_package(PostgreSQL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(GTest REQUIRED)

# Link dependencies
target_link_libraries(server PostgreSQL::PostgreSQL nlohmann_json::nlohmann_json)

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
