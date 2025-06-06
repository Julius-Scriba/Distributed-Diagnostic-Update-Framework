cmake_minimum_required(VERSION 3.10)
project(agent_dev)
set(DEPLOY_BUILD OFF)
add_subdirectory(../shared_libs ${CMAKE_CURRENT_BINARY_DIR}/shared)
# enable debug flags
set(CMAKE_BUILD_TYPE Debug)
target_compile_definitions(agent PRIVATE DEBUG)
