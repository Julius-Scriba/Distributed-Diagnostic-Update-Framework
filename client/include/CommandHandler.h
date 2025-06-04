#pragma once
#include "Module.h"
class CommandHandler : public Module {
public:
    std::string name() const override { return "CommandHandler"; }
    void init() override {} // placeholder
};
extern "C" Module* create_module();
