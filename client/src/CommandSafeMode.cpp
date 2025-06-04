#include "CommandModule.h"
#include "Globals.h"
#include <iostream>

class SafeModeCmd : public CommandModule {
public:
    std::string name() const override { return "CmdSafeMode"; }
    void init() override {
        register_command("SAFE_MODE", [](const nlohmann::json&){
            g_safe_mode.store(true);
            std::cout << "Entering safe mode" << std::endl;
        });
    }
};

extern "C" Module* create_module() { return new SafeModeCmd(); }
