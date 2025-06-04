#pragma once
#include "Module.h"
#include "json.hpp"
#include <string>
#ifdef _WIN32
#include <windows.h>
#include <atomic>
#include <thread>
#endif

class IPCServer : public Module {
public:
    IPCServer();
    std::string name() const override { return "IPCServer"; }
    void init() override;
    void stop();
private:
#ifdef _WIN32
    HANDLE pipe_ = INVALID_HANDLE_VALUE;
    std::thread thread_;
    std::atomic<bool> running_{false};
    void run();
#endif
};

class IPCClient {
public:
    static bool send(const nlohmann::json& msg);
};
