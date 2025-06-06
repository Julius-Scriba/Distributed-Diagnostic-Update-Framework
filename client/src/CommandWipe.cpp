#include "CommandModule.h"
#include "Wipe.h"

class WipeCmd : public CommandModule {
public:
    std::string name() const override { return "CmdWipe"; }
    void init() override {
        register_command("WIPE", [](const nlohmann::json& cmd){
            perform_wipe();
        });
    }
};

extern "C" Module* create_module() { return new WipeCmd(); }
