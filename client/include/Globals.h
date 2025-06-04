#pragma once
#include <atomic>
#include <string>

extern std::atomic<bool> g_safe_mode;
extern std::atomic<bool> g_deep_sleep;
extern std::string g_uuid;
