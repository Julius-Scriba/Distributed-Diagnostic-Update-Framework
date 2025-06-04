#include "Globals.h"
#include <atomic>

std::atomic<bool> g_safe_mode{false};
std::atomic<bool> g_deep_sleep{false};
std::string g_uuid;
