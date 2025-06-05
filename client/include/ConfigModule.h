#pragma once
#include "Module.h"
#include <string>

struct AgentConfig {
    std::string server_url = "http://localhost:5000";
    std::string host_header;
    std::string path_prefix;
};

extern AgentConfig g_agent_config;

class ConfigModule : public Module {
public:
    std::string name() const override { return "Config"; }
    void init() override;
};

extern "C" Module* create_module();
