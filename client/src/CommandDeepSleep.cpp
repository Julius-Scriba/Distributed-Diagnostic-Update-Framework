#include "CommandModule.h"
#include "Globals.h"
#include <iostream>

class DeepSleepCmd : public CommandModule {
public:
    std::string name() const override { return "CmdDeepSleep"; }
    void init() override {
        register_command("DEEP_SLEEP", [](const nlohmann::json& cmd){
            g_deep_sleep.store(true);
            std::cout << "Entering deep sleep" << std::endl;
        });
    }
};

extern "C" Module* create_module() { return new DeepSleepCmd(); }
