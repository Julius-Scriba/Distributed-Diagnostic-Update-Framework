cmake_minimum_required(VERSION 3.10)
project(agent_deploy)
# mark this build as deploy variant so shared libraries can include additional modules
set(DEPLOY_BUILD ON)
add_subdirectory(../shared_libs ${CMAKE_CURRENT_BINARY_DIR}/shared)
set(CMAKE_BUILD_TYPE Release)
target_compile_definitions(agent PRIVATE NDEBUG DEPLOY)
