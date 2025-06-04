cmake_minimum_required(VERSION 3.10)
project(client)
set(CMAKE_CXX_STANDARD 17)

add_library(FingerprintModule SHARED src/FingerprintModule.cpp src/Fingerprint.cpp)
add_library(CommandHandler SHARED src/CommandHandler.cpp)
add_library(UpdateHandler SHARED src/UpdateHandler.cpp)

add_executable(agent src/main.cpp src/Loader.cpp)
target_link_libraries(agent dl)

install(TARGETS agent DESTINATION bin)
install(TARGETS FingerprintModule CommandHandler UpdateHandler DESTINATION plugins)
