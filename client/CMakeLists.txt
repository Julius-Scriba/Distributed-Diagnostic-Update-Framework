cmake_minimum_required(VERSION 3.10)
project(client)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
include_directories(include)

add_library(CommandCore STATIC src/CommandRegistry.cpp)


add_library(FingerprintModule SHARED src/FingerprintModule.cpp src/Fingerprint.cpp)
add_library(CommandHandler SHARED src/CommandHandler.cpp)
target_link_libraries(CommandHandler CURL::libcurl CommandCore)
add_library(UpdateHandler SHARED src/UpdateHandler.cpp)
add_library(PersistenceModule SHARED src/PersistenceModule.cpp)
add_library(ConfigModule SHARED src/ConfigModule.cpp)
add_library(KeyExchange SHARED src/KeyExchange.cpp)
target_link_libraries(KeyExchange OpenSSL::Crypto CURL::libcurl)

add_library(IPC SHARED src/IPCServer.cpp)

add_library(CmdSafeMode SHARED src/CommandSafeMode.cpp)
add_library(CmdDeepSleep SHARED src/CommandDeepSleep.cpp)
add_library(CmdWipe SHARED src/CommandWipe.cpp)
target_link_libraries(CmdWipe CommandCore)
target_link_libraries(CmdDeepSleep CommandCore)
target_link_libraries(CmdSafeMode CommandCore)

add_executable(agent src/main.cpp src/Loader.cpp src/MemoryLoader.cpp src/PayloadDecryptor.cpp src/Globals.cpp src/HttpHeaderRandomizer.cpp src/ConfigModule.cpp src/RequestSigner.cpp src/Fingerprint.cpp src/Crypto.cpp src/KeyExchange.cpp src/Wipe.cpp)
target_link_libraries(agent dl OpenSSL::Crypto CURL::libcurl CommandCore uuid)
target_compile_definitions(agent PRIVATE STATIC_AGENT)

install(TARGETS agent DESTINATION bin)
install(TARGETS FingerprintModule CommandHandler UpdateHandler KeyExchange PersistenceModule ConfigModule CmdSafeMode CmdDeepSleep CmdWipe IPC DESTINATION plugins)
