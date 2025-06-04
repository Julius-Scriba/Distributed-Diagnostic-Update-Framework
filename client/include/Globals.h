#pragma once
#include <atomic>
#include <string>

extern std::atomic<bool> g_safe_mode;
extern std::atomic<bool> g_deep_sleep;
extern std::string g_uuid;

#include "HttpHeaderRandomizer.h"
extern HttpHeaderRandomizer g_header_randomizer;

#include "ConfigModule.h"
extern AgentConfig g_agent_config;
