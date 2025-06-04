#pragma once
#include "Module.h"
#include "CommandRegistry.h"

class CommandModule : public Module {
protected:
    void register_command(const std::string& name, CommandRegistry::Handler h) {
        CommandRegistry::instance().register_handler(name, h);
    }
};
