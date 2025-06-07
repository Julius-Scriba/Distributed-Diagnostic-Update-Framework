#include "ConfigModule.h"
#include "json.hpp"
#include <fstream>

AgentConfig g_agent_config;

void ConfigModule::init() {
    std::ifstream f("agent_config.json");
    if(!f.is_open()) return;
    try {
        nlohmann::json j; f >> j;
        g_agent_config.server_url = j.value("server_url", g_agent_config.server_url);
        g_agent_config.host_header = j.value("host_header", "");
        g_agent_config.path_prefix = j.value("path_prefix", "");
    } catch(const std::exception&) {
        // ignore malformed config
    }
}

#ifndef STATIC_AGENT
extern "C" Module* create_module() { return new ConfigModule(); }
#endif
