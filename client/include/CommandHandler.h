#pragma once
#include "Module.h"
#include <thread>
#include <string>

class CommandHandler : public Module {
public:
    explicit CommandHandler(const std::string& server);
    std::string name() const override { return "CommandHandler"; }
    void init() override;
private:
    std::string server_;
    std::thread poller_;
    static size_t write_cb(void* contents, size_t size, size_t nmemb, void* userp);
    void poll();
};

extern "C" Module* create_module();
